import zlib,struct,sys
with open("readback.bin","rb") as f:
    d = f.read()
crc = zlib.crc32(d[:-4]) & 0xFFFFFFFF
tail = struct.unpack("<I", d[-4:])[0]
print(f"CRC calc=0x{crc:08X} tail=0x{tail:08X} match={crc==tail}")
