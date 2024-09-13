#include <iostream>
#include "AES.h"

int main() {
    unsigned char plain[] =    { 
        0x5F, 0x57, 0x3B, 0xD5, 
        0x70, 0x84, 0x53, 0x94, 
        0x9A, 0xD5, 0x07, 0x23, 
        0xDA, 0x8D, 0x86, 0x3E };
    unsigned char cipher[] = {
        0xe1,0xd7,0xf,0xc4,
        0x52,0xec,0xc4,0x40,
        0x36,0x91,0x93,0xe8,
        0x34,0xfb,0xbb,0xd6
    };
    unsigned char key[] = { 0, 50, 52, 75, 114, 0, 114, 121, 112, 116, 0, 83, 111, 108, 118, 0 }; //key example
    unsigned int plainLen = 16 * sizeof(unsigned char);  //bytes in plaintext
    unsigned char* c;
    bool same;

    AES aes(AESKeyLength::AES_128);  ////128 - key length, can be 128, 192 or 256

    int start;
    scanf("%d", &start);

    for (int i1 = start; i1 < 256; i1++)
    {
        printf("== %d\n", i1);
        key[0] = i1;
        for (int i2 = 0; i2 < 256; i2++)
        {
            key[5] = i2;
            for (int i3 = 0; i3 < 256; i3++)
            {
                key[10] = i3;
                for (int i4 = 0; i4 < 256; i4++)
                {
                    key[15] = i4;
                    c = aes.EncryptECB(plain, plainLen, key);
                    same = true;
                    for (int i = 0; i < 16; i++) {
                        if (c[i] != cipher[i]) {
                            same = false;
                            break;
                        }
                    }
                    if (same) {
                        printf("%d %d %d %d\n", i1, i2, i3, i4);
                        exit(0);
                    }
                }
            }
        }
    }

}