from Crypto.Cipher import AES
from joblib import Parallel, delayed

k = [-1, 50, 52, 75, 114, -1, 114, 121, 112, 116, -1, 83, 111, 108, 118, -1]
k[0], k[5], k[10], k[15] = 39, 67, 111, 101


cipher = AES.new(bytes(k), AES.MODE_ECB)

plaintexts = open("2024-contest-sca-pt.txt", "r").readlines()
plaintexts = list(map(lambda pt: bytes.fromhex(pt), plaintexts))

ciphertexts = open("2024-contest-sca-ct.txt", "r").readlines()
ciphertexts = list(map(lambda ct: bytes.fromhex(ct), ciphertexts))

for p, c in zip(plaintexts, ciphertexts):
    assert cipher.encrypt(p) == c
