import subprocess

'''

0. Patch program
    A value that time64 return is seed of encryption.
    c_contest_2024_out_real.jpg created at 2024-04-08 15:52:46 (GMT+9) 0x6613947a (epoch time)
    patch "mov ecx, eax(time64 returned)" to "mov ecx, 0x6613947a"

1. Prepare c_contest_2024.jpg that filled with null(0x00)
2. Execute patched program => random value written at c_contest_2024_out.jpg
3. xor c_contest_2024_out.jpg and c_contest_2024_out_real.jpg

    ORIGINAL_FILE ^ RANDOM_VALUE = ENCRYPTED_FILE
    1~2. 0x00 ^ RANDOM_VALUE = RANDOM_VALUE
    3. ENCRYPTED_FILE ^ RANDOM_VALUE = ORIGINAL_FILE
'''

FILE_EPOCH_TIME = 0x6613947a
orig_file = list(open('./cryptocontest.exe', 'rb').read())

# mov ecx, eax ====> mov ecx, 0x6613947a
orig_file[0x768:0x768+5] = list(b'\xb9' + FILE_EPOCH_TIME.to_bytes(4, 'little'))

open('./cryptocontest_patched.exe', 'wb').write(bytes(orig_file))
open('./c_contest_2024.jpg', 'wb').write(b'\x00' * 40645)

subprocess.Popen('./cryptocontest_patched.exe').wait()

a = open('./c_contest_2024_out.jpg', 'rb').read()
b = open('./c_contest_2024_out_real.jpg', 'rb').read()

open('./result.jpg', 'wb').write(bytes(x ^ y for x, y in zip(a, b)))