# Constructing secp256k1 curve
p = 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f
E = EllipticCurve(GF(p), [0, 7])
G = E(0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798, 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8)
n = 0xfffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141

# half-half Bitcoin ECDSA nonce crack
h = 0x568b4901cc2dac4a13af161bbf6b2087c94d8b8223755fd121ec6aa0519ecee2
r = 0xda7866632109e77f0d3c5bdd49031e6d9a940fcb7d29ea2f858b991d1f17cef8
s = 0xa4a700ac4f18634ac845739e0342cd8335bf6e0606ca9dc9d668abf9ed812e6d
Q = E(0xa51208adff894cdd79d4d7d967aa4d492256ba4d527661b10ae7cfd6e15f28a6, 0x6fbfd9a270cd717afb0949e1c40fd2754b46f4f8472ac5711de0351fe81bbd80)

R = E.lift_x(r)

X128 = 2^128
hmsb = h // X128

# Default attack
A = X128 - s * inverse_mod(r, n)
b = (h - X128 * s * hmsb) * inverse_mod(r, n)

M = Matrix([
    [n, 0, 0], 
    [A, 1, 0], 
    [b, 0, X128]
])

L = M.LLL()

# TODO: Apply Optimizations
# Recentering, Brute forcing MSB or Sieving with predicate
