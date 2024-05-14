import struct

import matplotlib.pyplot as plt

from util import load_wave_file

#Load plaintexts, ciphertexts
plaintexts = open("2024-contest-sca-pt.txt", "r").readlines()
plaintexts = list(map(lambda pt: bytes.fromhex(pt), plaintexts))

ciphertexts = open("2024-contest-sca-ct.txt", "r").readlines()
ciphertexts = list(map(lambda ct: bytes.fromhex(ct), ciphertexts))

pairs = list(zip(plaintexts, ciphertexts))

fig, (plot1, plot2) = plt.subplots(2, 1)

# Load reference AES wave
N, L, reference_wave = load_wave_file("2024-contest-sca-tr-reference.bin")
plot1.plot(range(L), reference_wave[0])

# Load some AES waves given as challenge
N, L, partial_waves = load_wave_file("2024-contest-sca-tr.bin")
for partial_wave in partial_waves[:10]:
    plot2.plot(range(L), partial_wave)

plt.show()
