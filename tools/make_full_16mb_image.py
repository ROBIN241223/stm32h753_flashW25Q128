#!/usr/bin/env python3
"""
Generate a full 16MB (0x01000000) binary test image for external QSPI flash @0x90000000.
Pattern layout (offset -> length / description):
 0x00000000 : 256 bytes  - Incrementing bytes 0x00..0xFF
 0x00000100 : 256 bytes  - Decrementing bytes 0xFF..0x00
 0x00000200 : 1 KB       - 16-bit little-endian incrementing words (0x0000,0x0001,...)
 0x00000600 : 2 KB       - 32-bit little-endian incrementing words (0x00000000,...)
 0x00000E00 : fill continues with a repeating 64-byte pseudo-random LFSR pattern
 Last 4 bytes (at size-4) : CRC32 (Poly 0x04C11DB7, initial 0xFFFFFFFF, final XOR 0xFFFFFFFF) over preceding (size-4) bytes.

Usage:
  python make_full_16mb_image.py [-o build/qspi_full16m.bin]

The script also prints the CRC so you can verify with an external tool.
"""
from __future__ import annotations
import argparse, struct, zlib, sys, os

SIZE = 0x01000000  # 16MB
CRC_LEN = 4
DATA_LEN = SIZE - CRC_LEN

def build_image() -> bytearray:
    buf = bytearray(DATA_LEN)

    # 0x00000000: 0..255
    for i in range(256):
        buf[i] = i
    # 0x00000100: 255..0
    base = 0x100
    for i in range(256):
        buf[base + i] = 0xFF - i
    # 0x00000200: 1KB 16-bit words
    base = 0x200
    words16 = 1024 // 2
    for w in range(words16):
        struct.pack_into('<H', buf, base + w*2, w & 0xFFFF)
    # 0x00000600: 2KB 32-bit words (0x600..0x65FF + 0x6600.. etc.)
    base = 0x600
    words32 = 2048 // 4
    for w in range(words32):
        struct.pack_into('<I', buf, base + w*4, w)

    # Remainder: repeating 64B LFSR pattern to end
    # Simple 32-bit LFSR (tap polynomial 0x80000057) produce bytes from low byte of state rotated
    lfsr = 0xACE1ACE1
    pattern = []
    for _ in range(64):
        # Extract a byte (e.g., lowest) and append
        pattern.append(lfsr & 0xFF)
        # Advance LFSR
        for _ in range(3):  # advance a few times to decorrelate consecutive bytes
            lsb = lfsr & 1
            lfsr >>= 1
            if lsb:
                lfsr ^= 0x80000057
    pat = bytes(pattern)

    fill_start = 0xE00
    # Safety clamp
    if fill_start < len(buf):
        for off in range(fill_start, len(buf)):
            buf[off] = pat[(off - fill_start) % len(pat)]

    return buf

def place_crc(buf: bytearray) -> int:
    crc = zlib.crc32(buf) & 0xFFFFFFFF
    # Mirror typical STM style final XOR (already applied by & 0xFFFFFFFF, so explicit XOR if desired) - keep raw CRC32 IEEE.
    return crc

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('-o','--output', default='build/qspi_full16m.bin', help='Output binary path')
    args = ap.parse_args()

    os.makedirs(os.path.dirname(args.output), exist_ok=True)

    data = build_image()
    crc = place_crc(data)

    # Append CRC little-endian at end of 16MB region
    out = data + struct.pack('<I', crc)
    if len(out) != SIZE:
        print(f"Internal size mismatch: {len(out):#x}", file=sys.stderr)
        return 2
    with open(args.output,'wb') as f:
        f.write(out)
    print(f"Wrote {args.output} ({len(out)} bytes) CRC32=0x{crc:08X}")
    # Print a few sample offsets for manual verification
    sample_offsets = [0x0, 0x100, 0x200, 0x600, 0xE00, DATA_LEN-16]
    for off in sample_offsets:
        chunk = out[off:off+16]
        print(f"@{off:#08x}: {' '.join(f'{b:02X}' for b in chunk)}")

if __name__ == '__main__':
    sys.exit(main())
