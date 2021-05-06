import bsa

misc = bsa.BSA("Data/Oblivion - Misc.bsa")
misc.read()
print(misc)

textures = bsa.BSA("Data/Oblivion - Textures - Compressed.bsa")
textures.read()
print(textures)
