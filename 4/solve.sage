load("params.sage")

# Recover Cheolsu's random number
# s = k^-1 * (e + rd) mod n
# => k = s^-1 * (e + rd) mod n
kC = inverse_mod(sC, n) * (hC + rC * dC)
kC = kC % n
assert rC == (kC * G).xy()[0]
QC = dC * G
