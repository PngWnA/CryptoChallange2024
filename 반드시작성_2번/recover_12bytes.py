from scipy.stats import pearsonr
from sbox import SBox
from util import load_wave_file
from joblib import Parallel, delayed

plaintexts = open("2024-contest-sca-pt.txt", "r").readlines()
plaintexts = list(map(lambda pt: bytes.fromhex(pt), plaintexts))

WAVE_NUM, LEN, waves = load_wave_file("2024-contest-sca-tr.bin")

HW = [bin(n).count('1') for n in range(256)]

# search for each byte
def process_data(i):
    # get all ith byte plaintext
    plaintexts_i = [x[i] for x in plaintexts]

    ans_point = 0
    ans_p = 0
    ans_k = 0

    for keyGuess in range(256):
        if keyGuess % 16 == 0:
            print(f'Thread {i}: {keyGuess}')
        hammingPower = [HW[SBox[pt ^ keyGuess]] for pt in plaintexts_i]
        for point in range(LEN):
            actualPower = [wave[point] for wave in waves]
            pGuess = abs(pearsonr(hammingPower, actualPower).statistic)
            if pGuess > ans_p:
                ans_point = point
                ans_k = keyGuess
                ans_p = pGuess
    
    print(i, ans_point, ans_k, ans_p)
    return ans_k

results = Parallel(n_jobs=-1, backend='loky')(delayed(process_data)(data) for data in range(16))

print(results)

'''
0 842 16 0.13077799285434402
    1 300 50 0.7285700265786639
    2 201 52 0.7530488270812588
    3 102 75 0.7432896726503209
    4 3 114 0.7542411724892242
5 202 165 0.14279682965204238
    6 332 114 0.737164444435327
    7 234 121 0.7385455267518652
    8 135 112 0.756461075376013
    9 36 116 0.7454948505434374
10 106 235 0.1410921570190006
    11 341 83 0.7208636963078783
    12 267 111 0.7501859888116553
    13 168 108 0.7489794586842916
    14 69 118 0.7760135826791072
15 362 181 0.14422015249316208
'''