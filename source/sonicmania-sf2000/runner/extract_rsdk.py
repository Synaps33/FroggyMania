import hashlib, struct, sys

RSDK = "/media/Sajnaps/GB300/ROMS/sonicmania/Data.rsdk"
if len(sys.argv) > 2:
    RSDK = sys.argv[2]

def lookup_key(path):
    d = hashlib.md5(path.lower().encode()).digest()
    return b"".join(d[i:i+4][::-1] for i in range(0, 16, 4))

def make_keys(filename, filesize):
    def words(d): return [struct.unpack("<I", d[i*4:i*4+4])[0] for i in range(4)]
    a = hashlib.md5(filename.upper().encode()).digest()
    b = hashlib.md5(str(filesize).encode()).digest()
    keyA, keyB = bytearray(16), bytearray(16)
    for i in range(4):
        for j in range(4):
            keyA[i*4+j] = (words(a)[i] >> (8*(j ^ 3))) & 0xFF
            keyB[i*4+j] = (words(b)[i] >> (8*(j ^ 3))) & 0xFF
    return keyA, keyB

def decrypt(data, filename):
    keyA, keyB = make_keys(filename, len(data))
    out = bytearray(len(data))
    keyNo = (len(data) // 4) & 0x7F
    posA, posB, nyb = 0, 8, False
    for n, ch in enumerate(data):
        v = ch ^ keyNo ^ keyB[posB]
        if nyb: v = ((v << 4) + (v >> 4)) & 0xFF
        v ^= keyA[posA]
        out[n] = v
        posA += 1; posB += 1
        if posA <= 15:
            if posB > 12: posB = 0; nyb ^= 1
        elif posB <= 8: posA = 0; nyb ^= 1
        else:
            keyNo = (keyNo + 2) & 0x7F
            if nyb: nyb = False; posA = keyNo % 7; posB = (keyNo % 12) + 2
            else: nyb = True; posA = (keyNo % 12) + 3; posB = keyNo % 7
    return bytes(out)

def extract(path):
    k = lookup_key(path)
    with open(RSDK, "rb") as f:
        f.read(4); f.read(2)
        (n,) = struct.unpack("<H", f.read(2))
        for _ in range(n):
            h = f.read(16)
            off, size = struct.unpack("<II", f.read(8))
            if h == k:
                enc = bool(size & 0x80000000)
                size &= 0x7FFFFFFF
                f.seek(off)
                raw = f.read(size)
                return decrypt(raw, path) if enc else raw
    return None

if __name__ == "__main__":
    p = sys.argv[1]
    d = extract(p)
    if d: sys.stdout.buffer.write(d)
