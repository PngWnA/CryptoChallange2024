import os
import re
from tqdm import tqdm

source_list = [
    'o0_base.c',
    'o1_precompute.c',
    'o2_inline_unroll.c',
    'o3_pmr_square.c',
    'o4_merge_enc_auth.c',
    'o5_remove_temp.c',
    'o6_8bit_to_64bit.c',
    'o7_gcc_optim.c',
    'optimized.c',
    '64optim.c'
]

experiment = {k: 0 for k in source_list}

N = 100
for source in source_list:
    os.system(f'gcc {source} -o temp')

    cycles_list = []
    for _ in tqdm(range(N)):
        os.system(f'./temp > results')
        data = open('results').read()
        cycles = re.findall(r'Improved implementation runs in ................. +(\d+) cycles', data)
        cycles = int(cycles[0])
        cycles_list.append(cycles)
    cycles_avg = sum(cycles_list) / N
    experiment[source] = cycles_avg
    
    print(source, cycles_avg)
print(experiment)
