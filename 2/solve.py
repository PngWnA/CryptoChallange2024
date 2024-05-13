import struct

import matplotlib.pyplot as plt

from util import load_wave_file

#Load plaintexts, ciphertexts
plaintexts = open("2024-contest-sca-pt.txt", "r").readlines()
plaintexts = list(map(lambda pt: bytes.fromhex(pt), plaintexts))

ciphertexts = open("2024-contest-sca-ct.txt", "r").readlines()
ciphertexts = list(map(lambda ct: bytes.fromhex(ct), ciphertexts))

pairs = list(zip(plaintexts, ciphertexts))

# Load reference AES wave
# N, L, reference_wave = load_wave_file("2024-contest-sca-tr-reference.bin")
# print(reference_wave)

# Load AES wave given as challenge
N, L, partial_waves = load_wave_file("2024-contest-sca-tr.bin")
plt.plot(range(L), partial_waves[0])
plt.show()
