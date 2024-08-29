import struct

import matplotlib.pyplot as plt

from scipy import stats

from sbox import SBox
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

subbytespoints = [None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None] # 전력측정할 지점을 정의
master_key = [None, None, None, None, None, None, None, None, None, None, None, None, None, None, None]

for i in range(0, 16):
    corrs = []
    for k in range(0, 0x100):
        x = []
        y = []
        for j in range(len(plaintexts)):
            x.append(SBox(plaintexts[j][i] ^ k))
            y.append(partial_wave[j][subbytespoints[i]])
        corrs.append(stats.pearsonr(x, y)[0])
    master_key[i] = corrs.index(max(corrs))

print(master_key)