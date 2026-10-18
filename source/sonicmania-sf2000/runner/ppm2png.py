#!/usr/bin/env python3
import struct, sys, zlib, os, glob

def read_ppm(path):
    data = open(path, "rb").read()
    parts, i = [], 0
    while len(parts) < 4:
        while data[i:i+1].isspace(): i += 1
        if data[i:i+1] == b'#':
            while data[i:i+1] != b'\n': i += 1
            continue
        j = i
        while not data[j:j+1].isspace(): j += 1
        parts.append(data[i:j]); i = j
    i += 1
    w, h = int(parts[1]), int(parts[2])
    return w, h, data[i:i+w*h*3]

def write_png(path, w, h, rgb):
    def chunk(tag, payload):
        return struct.pack(">I", len(payload)) + tag + payload + struct.pack(">I", zlib.crc32(tag + payload) & 0xffffffff)
    raw = b"".join(b"\x00" + rgb[y*w*3:(y+1)*w*3] for y in range(h))
    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)) + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b"")
    open(path, "wb").write(png)

src = sys.argv[1]
for p in sorted(glob.glob(os.path.join(src, "*.ppm"))):
    w, h, rgb = read_ppm(p)
    out = p[:-4] + ".png"
    write_png(out, w, h, rgb)
