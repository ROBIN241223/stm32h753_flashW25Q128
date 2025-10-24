# QSPI 16MB Test Pattern

File generated: `build/qspi_full16m.bin` (size 16,777,216 bytes = 16MB) with trailing CRC32.

## Pattern Layout
| Offset      | Length        | Description |
|-------------|---------------|-------------|
| 0x00000000  | 256 B         | Incrementing 0x00..0xFF |
| 0x00000100  | 256 B         | Decrementing 0xFF..0x00 |
| 0x00000200  | 1 KB          | 16-bit LE words 0x0000,0x0001,... |
| 0x00000600  | 2 KB          | 32-bit LE words 0x00000000,0x00000001,... |
| 0x00000E00  | (rest-0xE00)  | Repeating 64-byte LFSR pattern |
| 0x01000000-4| 4 B           | CRC32 (IEEE, little-endian) of first 16MB-4 bytes |

Example bytes (from generation log):
```
@0x000000: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
@0x000100: FF FE FD FC FB FA F9 F8 F7 F6 F5 F4 F3 F2 F1 F0
@0x000200: 00 00 01 00 02 00 03 00 04 00 05 00 06 00 07 00
@0x000600: 00 00 00 00 01 00 00 00 02 00 00 00 03 00 00 00
@0x000E00: E1 A2 C8 D9 25 AD 5C 3C F0 7E E4 4B 2B A7 E1 A2
```

CRC printed during generation (example): `CRC32=0x19A1F3BD` (check your run output).

## Generation
Python script:
```
python tools/make_full_16mb_image.py -o build/qspi_full16m.bin
```

### PowerShell Fallback (no Python)
Generates a simpler repeating pattern (NOT identical to Python version) filling 16MB then appends CRC32 computed by .NET:
```powershell
$size = 0x1000000
$buf = New-Object byte[] $size
for($i=0; $i -lt $size-4; $i++){ $buf[$i] = [byte]($i -band 0xFF) }
# Compute CRC32
Add-Type -AssemblyName System.IO.Compression.FileSystem | Out-Null
# Quick CRC32 implementation
$crcTable = @(0..255 | ForEach-Object {
  $c = $_
  for($j=0;$j -lt 8;$j++){ if(($c -band 1) -ne 0){ $c = (0xEDB88320 -bxor ($c -shr 1)) } else { $c = $c -shr 1 } }
  $c
})
$crc = 0xFFFFFFFF
for($i=0;$i -lt $size-4;$i++){ $crc = $crcTable[($crc -bxor $buf[$i]) -band 0xFF] -bxor ($crc -shr 8) }
$crc = $crc -bxor 0xFFFFFFFF
[BitConverter]::GetBytes($crc).CopyTo($buf, $size-4)
[IO.File]::WriteAllBytes('build/qspi_full16m_simple.bin', $buf)
Write-Host ("Wrote build/qspi_full16m_simple.bin CRC32=0x{0:X8}" -f $crc)
```

## Programming into QSPI (CubeProgrammer)
1. Ensure QSPI memory-mapped mode is working (run internal stub or external loader config).
2. Open STM32CubeProgrammer and connect (SWD).
3. External Loader route:
   - If custom .stldr present, select it so region 0x90000000.. appears for write.
   - Click "Download" and choose `qspi_full16m.bin` with start address `0x90000000`.
4. Stub route (no external loader):
   - Use a RAM helper or stub that provides flash write routine (not covered here) OR
   - Program smaller test sectors via code; full 16MB may be slow.

## Verification
### Memory View Sampling
After programming, open a memory view at 0x90000000 and check:
- 0x90000000 : 00 01 02 03 ...
- 0x90000100 : FF FE FD FC ...
- 0x90000200 : 00 00 01 00 02 00 ...
- 0x90000600 : 00 00 00 00 01 00 00 00 ...
- 0x90000E00 : Repeating LFSR bytes as above.

### CRC32 Full Region
If CubeProgrammer supports checksum for external memory (with loader), compute over length 0x01000000 and compare to printed CRC.
If not, you can read back the binary (Download from memory) and run:
```
python - <<'PY'
import zlib,sys
with open('readback.bin','rb') as f: d=f.read()
print(hex(zlib.crc32(d[:-4]) & 0xFFFFFFFF), d[-4:])
PY
```
Expect CRC32 and last 4 bytes matching.

## Notes
- The LFSR section stresses read bandwidth uniformity.
- You can shrink size by editing SIZE in the script for partial tests.
- For speed, switch QSPI to Fast/Quad Read before reading large spans.
