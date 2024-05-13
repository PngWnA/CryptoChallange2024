plaintexts = open("2024-contest-sca-pt.txt", "r").readlines()
plaintexts = list(map(lambda pt: bytes.fromhex(pt), plaintexts))

ciphertexts = open("2024-contest-sca-ct.txt", "r").readlines()
ciphertexts = list(map(lambda ct: bytes.fromhex(ct), ciphertexts))

pairs = list(zip(plaintexts, ciphertexts))

print(pairs)
