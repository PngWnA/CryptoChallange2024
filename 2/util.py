import struct

def load_wave_file(filepath):
  data = open(filepath, "rb")
  N = load_N(data.read(4)) # Number of waves
  L = load_L(data.read(4)) # Length of each wave (Number of measuring points)
  
  waves = list()
  for i in range(N):
    wave = list()
    for j in range(L):
      point = load_float(data.read(4))
      wave.append(point)
    waves.append(wave)
  return N, L, waves

def load_N(bin):
  return load_unsigned_int(bin)

def load_L(bin):
  return load_unsigned_int(bin)

def load_point(bin):
  return load_fload(bin)

def load_unsigned_int(bin):
  return struct.unpack("<I", bin)[0]

def load_float(bin):
  return struct.unpack("f", bin)[0]
