def bytes_to_int(b):
    return int.from_bytes(b, byteorder='little')


def read_bstring(file):
    b = bytes_to_int(file.read(1))
    bstring = file.read(b)
    return bstring.decode("utf-8")


def read_zstring(file):
    zstring = ""
    b = file.read(1)
    while b and not b == b'\x00':
        zstring += b.decode("utf-8")
        b = file.read(1)
    return zstring


def read_bzstring(file):
    b = bytes_to_int(file.read(1))
    zstring = file.read(b)
    if zstring[-1] != 0:
        raise ValueError
    return zstring[:-1].decode("utf-8")
