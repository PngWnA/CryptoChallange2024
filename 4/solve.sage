load("params.sage")

# Recover Cheolsu's random number
# s = k^-1 * (e + rd) mod n
# => k = s^-1 * (e + rd) mod n
kC = inverse_mod(sC, n) * (hC + rC * dC)
kC = Mod(kC, n)
assert rC == (kC * G).xy()[0]
print(rC, (kC * G).xy()[0])
QC = dC * G
print(rC < n)

print(G*kC)
print(rY)
print(factor(int(kC)))
rC = Zmod(n)(rC)

sYi = inverse_mod(sY, n)
sCi = inverse_mod(sC, n)
dYp = (sCi * hC - sYi * hY) * inverse_mod(sYi * rY - sCi * rC, n)
dYp = Mod(dYp, n)
print(dYp, -dYp)