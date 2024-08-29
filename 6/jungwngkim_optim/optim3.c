#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int64_t cpucycles(void)
{
    unsigned int hi, lo;

    __asm__ __volatile__("rdtsc\n\t" : "=a"(lo), "=d"(hi));

    return ((int64_t)lo) | (((int64_t)hi) << 32);
}

#define DEBUG 1
#define DEBUG_KEY 1
#define DEBUG_ENC 1
#define DEBUG_HASH 1

// BENCH ROUND
#define BENCH_ROUND (DEBUG ? 1 : 1000)

// round of block cipher
#define NUM_ROUND 80

// basic operation
#define ROR(x, r) ((x >> r) | (x << (8 - r)))
#define ROL(x, r) ((x << r) | (x >> (8 - r)))

// constant :: cryptogr in ASCII
#define CONST0 0x63
#define CONST1 0x72
#define CONST2 0x79
#define CONST3 0x70
#define CONST4 0x74
#define CONST5 0x6F
#define CONST6 0x67
#define CONST7 0x72

// constant :: shift offset
#define OFF1 1
#define OFF3 3
#define OFF5 5
#define OFF7 7

// constant :: nonce value
#define NONCE1 0x12
#define NONCE2 0x34
#define NONCE3 0x56
#define NONCE4 0x78
#define NONCE5 0x9A
#define NONCE6 0xBC
#define NONCE7 0xDE

void key_scheduling(uint8_t* master_key, uint8_t* rk) {
    uint32_t i = 0;

    // initialization
    for (i = 0; i < 8; i++) {
        rk[i] = master_key[i];
    }

    for (i = 1; i < NUM_ROUND; i++) {
        rk[i * 8 + 0] = ROL(rk[(i - 1) * 8 + 0], (i + OFF1) % 8) + ROL(CONST0, (i + OFF3) % 8);
        rk[i * 8 + 1] = ROL(rk[(i - 1) * 8 + 1], (i + OFF5) % 8) + ROL(CONST1, (i + OFF7) % 8);
        rk[i * 8 + 2] = ROL(rk[(i - 1) * 8 + 2], (i + OFF1) % 8) + ROL(CONST2, (i + OFF3) % 8);
        rk[i * 8 + 3] = ROL(rk[(i - 1) * 8 + 3], (i + OFF5) % 8) + ROL(CONST3, (i + OFF7) % 8);

        rk[i * 8 + 4] = ROL(rk[(i - 1) * 8 + 4], (i + OFF1) % 8) + ROL(CONST4, (i + OFF3) % 8);
        rk[i * 8 + 5] = ROL(rk[(i - 1) * 8 + 5], (i + OFF5) % 8) + ROL(CONST5, (i + OFF7) % 8);
        rk[i * 8 + 6] = ROL(rk[(i - 1) * 8 + 6], (i + OFF1) % 8) + ROL(CONST6, (i + OFF3) % 8);
        rk[i * 8 + 7] = ROL(rk[(i - 1) * 8 + 7], (i + OFF5) % 8) + ROL(CONST7, (i + OFF7) % 8);
    }
}

void ROUND_FUNC(uint8_t* imm, uint8_t* rk, uint8_t index, uint8_t loop_indx, uint8_t offset) {
    imm[index] = rk[loop_indx * 8 + index] ^ imm[index];
    imm[index] = rk[loop_indx * 8 + index] ^ imm[index - 1] + imm[index];
    ROL(imm[index], offset);
}

void block_encryption(uint8_t* PT, uint8_t* rk, uint8_t* CT) {
    uint32_t i = 0;
    uint32_t j = 0;
    uint8_t imm[8] = {
        0,
    };
    uint8_t tmp = 0;

    for (i = 0; i < 8; i++) {
        imm[i] = PT[i];
    }

    for (i = 0; i < NUM_ROUND; i++) {
        for (j = 7; j > 0; j--) {
            ROUND_FUNC(imm, rk, j, i, j);
        }

        // Rotate
        tmp = imm[0];
        for (j = 1; j < 8; j++) {
            imm[j - 1] = imm[j];
        }
        imm[7] = tmp;
    }

    for (i = 0; i < 8; i++) {
        CT[i] = imm[i];
    }
}

void CTR_mode(uint8_t* PT, uint8_t* master_key, uint8_t* CT, uint8_t num_enc) {
    uint32_t i = 0;
    uint32_t j = 0;
    uint8_t imm[8] = {
        0,
    };
    uint8_t imm2[8] = {
        0,
    };
    uint8_t ctr = 0;

    uint8_t rk[8 * NUM_ROUND] = {
        0,
    };

    // key schedule
    key_scheduling(master_key, rk);

    // nonce setting
    imm[1] = NONCE1;
    imm[2] = NONCE2;
    imm[3] = NONCE3;
    imm[4] = NONCE4;
    imm[5] = NONCE5;
    imm[6] = NONCE6;
    imm[7] = NONCE7;

    for (i = 0; i < num_enc; i++) {
        // ctr setting
        imm[0] = ctr++;
        block_encryption(imm, rk, imm2);
        for (j = 0; j < 8; j++) {
            CT[i * 8 + j] = PT[i * 8 + j] ^ imm2[j];
        }
    }
}

void POLY_MUL_RED(uint8_t* IN1, uint8_t* IN2, uint8_t* OUT) {
    uint64_t* in1_64_p = (uint64_t*)IN1;
    uint64_t* in2_64_p = (uint64_t*)IN2;
    uint64_t* out_64_p = (uint64_t*)OUT;

    uint64_t in1_64 = in1_64_p[0];
    uint64_t in2_64 = in2_64_p[0];
    uint64_t one = 1;

    uint64_t result[2] = {
        0,
    };

    int32_t i = 0;

    for (i = 0; i < 64; i++) {
        if (((one << i) & in1_64) > 0) {
            result[0] ^= in2_64 << i;
            if (i != 0) {
                result[1] ^= in2_64 >> (64 - i);
            }
        }
    }

    // reduction
    result[0] ^= result[1];
    result[0] ^= result[1] << 9;
    result[0] ^= result[1] >> 55;
    result[0] ^= (result[1] >> 55) << 9;

    out_64_p[0] = result[0];
}

void AUTH_mode(uint8_t* CT, uint8_t* AUTH, uint8_t num_auth) {
    uint8_t AUTH_nonce[8] = {
        0,
    };
    uint8_t AUTH_inter[8] = {
        0,
    };
    uint32_t i, j;

    // nonce setting
    AUTH_nonce[0] = num_auth;
    AUTH_nonce[1] = num_auth ^ NONCE1;
    AUTH_nonce[2] = num_auth & NONCE2;
    AUTH_nonce[3] = num_auth | NONCE3;
    AUTH_nonce[4] = num_auth ^ NONCE4;
    AUTH_nonce[5] = num_auth & NONCE5;
    AUTH_nonce[6] = num_auth | NONCE6;
    AUTH_nonce[7] = num_auth ^ NONCE7;

    POLY_MUL_RED(AUTH_nonce, AUTH_nonce, AUTH_inter);

    for (i = 0; i < num_auth; i++) {
        for (j = 0; j < 8; j++) {
            AUTH_inter[j] ^= CT[i * 8 + j];
        }
        POLY_MUL_RED(AUTH_nonce, AUTH_inter, AUTH_inter);
        POLY_MUL_RED(AUTH_inter, AUTH_inter, AUTH_inter);
    }

    for (i = 0; i < 8; i++) {
        AUTH[i] = AUTH_inter[i];
    }
}

void ENC_AUTH(uint8_t* PT, uint8_t* master_key, uint8_t* CT, uint8_t* AUTH, uint8_t length_in_byte) {
    uint8_t num_enc_auth = length_in_byte / 8;

    CTR_mode(PT, master_key, CT, num_enc_auth);
    AUTH_mode(CT, AUTH, num_enc_auth);
}

void schedule_key(uint8_t *mk, uint8_t *rk) __attribute__((optimize("-O3")));
void encrypt(uint8_t *PT, uint8_t *rk, uint8_t *CT) __attribute__((optimize("-O3")));
void pmr(uint8_t *IN1, uint8_t *IN2, uint8_t *OUT) __attribute__((optimize("-O3")));
void pmr_sq(uint8_t *IN, uint8_t *OUT) __attribute__((optimize("-O3")));
void encrypt_hash(uint8_t *pt, uint8_t *mk, uint8_t *ct, uint8_t *hash, uint8_t len) __attribute__((optimize("-O3")));

void schedule_key(uint8_t *mk, uint8_t *rk)
{
    uint32_t i = 0;
    uint32_t cache[64] = {
        27, 57, 203, 56, 163, 183, 59, 57, 54, 114, 151, 112, 71, 111,
        118, 114, 108, 228, 47, 224, 142, 222, 236, 228, 216, 201, 94,
        193, 29, 189, 217, 201, 177, 147, 188, 131, 58, 123, 179, 147,
        99, 39, 121, 7, 116, 246, 103, 39, 198, 78, 242, 14, 232,
        237, 206, 78, 141, 156, 229, 28, 209, 219, 157, 156};

    ((uint64_t *)rk)[0] = ((uint64_t *)mk)[0];

    for (i = 1; i < NUM_ROUND; i++)
    {
        rk[i * 8 + 0] = ROL(rk[(i - 1) * 8 + 0], (i + 1) % 8) + cache[(i * 8 + 0) % 64];
        rk[i * 8 + 1] = ROL(rk[(i - 1) * 8 + 1], (i + 5) % 8) + cache[(i * 8 + 1) % 64];
        rk[i * 8 + 2] = ROL(rk[(i - 1) * 8 + 2], (i + 1) % 8) + cache[(i * 8 + 2) % 64];
        rk[i * 8 + 3] = ROL(rk[(i - 1) * 8 + 3], (i + 5) % 8) + cache[(i * 8 + 3) % 64];
        rk[i * 8 + 4] = ROL(rk[(i - 1) * 8 + 4], (i + 1) % 8) + cache[(i * 8 + 4) % 64];
        rk[i * 8 + 5] = ROL(rk[(i - 1) * 8 + 5], (i + 5) % 8) + cache[(i * 8 + 5) % 64];
        rk[i * 8 + 6] = ROL(rk[(i - 1) * 8 + 6], (i + 1) % 8) + cache[(i * 8 + 6) % 64];
        rk[i * 8 + 7] = ROL(rk[(i - 1) * 8 + 7], (i + 5) % 8) + cache[(i * 8 + 7) % 64];
    }
}

void encrypt(uint8_t *PT, uint8_t *rk, uint8_t *CT)
{
    uint32_t i;
    uint8_t tmp[8];

    ((uint64_t *)CT)[0] = ((uint64_t *)PT)[0];

    for (i = 0; i < NUM_ROUND; i++)
    {
        tmp[7] = CT[0];
        tmp[6] = ROL((uint8_t)(rk[i * 8 + 7] ^ (CT[6] + (rk[i * 8 + 7] ^ CT[7]))), 7);
        tmp[5] = ROL((uint8_t)(rk[i * 8 + 6] ^ (CT[5] + (rk[i * 8 + 6] ^ CT[6]))), 6);
        tmp[4] = ROL((uint8_t)(rk[i * 8 + 5] ^ (CT[4] + (rk[i * 8 + 5] ^ CT[5]))), 5);
        tmp[3] = ROL((uint8_t)(rk[i * 8 + 4] ^ (CT[3] + (rk[i * 8 + 4] ^ CT[4]))), 4);
        tmp[2] = ROL((uint8_t)(rk[i * 8 + 3] ^ (CT[2] + (rk[i * 8 + 3] ^ CT[3]))), 3);
        tmp[1] = ROL((uint8_t)(rk[i * 8 + 2] ^ (CT[1] + (rk[i * 8 + 2] ^ CT[2]))), 2);
        tmp[0] = ROL((uint8_t)(rk[i * 8 + 1] ^ (CT[0] + (rk[i * 8 + 1] ^ CT[1]))), 1);

        ((uint64_t *)CT)[0] = ((uint64_t *)tmp)[0];
    }
}

// polynomial_multiplication_and_reduction
void pmr(uint8_t *IN1, uint8_t *IN2, uint8_t *OUT)
{
    uint64_t in1_64 = ((uint64_t *)IN1)[0];
    uint64_t in2_64 = ((uint64_t *)IN2)[0];

    uint64_t r0 = 0, r1 = 0;

    int32_t i;

    if (1ll & in1_64) r0 ^= in2_64;
    for (i = 1; i < 64; i++)
    {
        if ((1ll << i) & in1_64)
        {
            r0 ^= in2_64 << i;
            r1 ^= in2_64 >> (64 - i);
        }
    }

    // reduction
    ((uint64_t *)OUT)[0] = r0 ^ r1 ^ (r1 << 9) ^ (r1 >> 55) ^ ((r1 >> 55) << 9);
}

void pmr_sq(uint8_t *IN, uint8_t *OUT)
{
    uint64_t in_64 = ((uint64_t *)IN)[0];
    uint64_t r0 = 0, r1 = 0;

    int32_t i;

    for (i = 0; i < 32; i++)
        if ((1ll << i) & in_64)
            r0 |= (1ll << (i * 2));

    for (i = 32; i < 64; i++)
        if ((1ll << i) & in_64)
            r1 |= (1ll << (i * 2));
    
    // reduction
    ((uint64_t *)OUT)[0] = r0 ^ r1 ^ (r1 << 9) ^ (r1 >> 55) ^ ((r1 >> 55) << 9);
}

void encrypt_hash(uint8_t *pt, uint8_t *mk, uint8_t *ct, uint8_t *hash, uint8_t len)
{
    uint8_t n = len / 8;

    uint32_t i;
    uint8_t imm_ctr[8], imm_enc[8], rk[8 * NUM_ROUND];
    uint8_t nonce[8];

    uint64_t start;

    // key schedule
    #if DEBUG_KEY
    start = cpucycles();
    #endif
    schedule_key(mk, rk);
    #if DEBUG_KEY
    printf("key schedule: %ld\n", cpucycles() - start);
    #endif

    // encryption input setting
    ((uint64_t *)imm_ctr)[0] = 0xdebc9a7856341200ull;

    // nonce setting
    memset(nonce, n, sizeof(nonce));
    ((uint64_t *)nonce)[0] ^= 0xde00007800001200ull;
    ((uint64_t *)nonce)[0] |= 0x00bc000056000000ull;
    ((uint64_t *)nonce)[0] &= 0xffff9affff34ffffull;
    pmr_sq(nonce, hash);

    for (i = 0; i < n; i++)
    {
        // encryption
        #if DEBUG_ENC
        start = cpucycles();
        #endif
        imm_ctr[0] = i; // counter
        encrypt(imm_ctr, rk, imm_enc);
        ((uint64_t *)ct)[i] = ((uint64_t *)pt)[i] ^ ((uint64_t *)imm_enc)[0];
        #if DEBUG_ENC
        printf("enc: %ld\n", cpucycles() - start);
        #endif

        // hash
        #if DEBUG_HASH
        start = cpucycles();
        #endif
        ((uint64_t *)hash)[0] ^= ((uint64_t *)ct)[i];
        pmr(nonce, hash, hash);
        pmr_sq(hash, hash);
        #if DEBUG_HASH
        printf("hash: %ld\n", cpucycles() - start);
        #endif
    }
}


// PT range (1-255 bytes)
#define LENGTH0 64
#define LENGTH1 128
#define LENGTH2 192

int main(int argc, const char *argv[])
{
    uint8_t plaintext_0[LENGTH0] = {
        0x42, 0xFB, 0x9F, 0xE0, 0x59, 0x81, 0x5A, 0x81, 0x66, 0xA1, 0x0E, 0x5C, 0x4E, 0xB4, 0xDA, 0xEC,
        0x2F, 0xF5, 0x60, 0x7E, 0x8A, 0xED, 0x3B, 0xCA, 0x2B, 0xD5, 0x82, 0x69, 0x1D, 0xC3, 0x84, 0x13,
        0x0E, 0xA6, 0x6A, 0x10, 0xB3, 0x3C, 0xB4, 0x4E, 0x9A, 0x80, 0x4F, 0x61, 0x06, 0x82, 0x17, 0xF4,
        0xCA, 0x76, 0xBA, 0x84, 0xE2, 0xDC, 0xC9, 0x66, 0x4F, 0xA5, 0x07, 0x8C, 0x8E, 0x36, 0xD1, 0x97};
    uint8_t plaintext_1[LENGTH1] = {
        0x4E, 0xE2, 0xB3, 0x54, 0x05, 0x90, 0xB0, 0xFD, 0x87, 0x9B, 0x30, 0xAB, 0x19, 0xC4, 0x66, 0x8F,
        0x2F, 0x22, 0x30, 0xA8, 0x5E, 0x23, 0x5B, 0x0B, 0xB1, 0xEB, 0xD6, 0xAD, 0x10, 0x0F, 0x33, 0x25,
        0x90, 0x66, 0xC5, 0x82, 0xE7, 0x1B, 0x47, 0xCA, 0xBE, 0x61, 0xA3, 0x91, 0xDB, 0xC2, 0x19, 0x97,
        0x04, 0x6A, 0x73, 0x02, 0x08, 0x70, 0x28, 0x44, 0x38, 0x69, 0xB5, 0xCE, 0x55, 0x95, 0xCB, 0x90,
        0xD3, 0x8A, 0xE2, 0x60, 0x89, 0x2A, 0x15, 0xCA, 0x36, 0x9B, 0x73, 0xEC, 0xEF, 0xD0, 0x43, 0x0B,
        0xA7, 0xFC, 0xDA, 0x4B, 0xAB, 0xE7, 0xB3, 0xC9, 0xB7, 0xF5, 0xD8, 0x86, 0xA2, 0xC5, 0x41, 0x5D,
        0x18, 0xC3, 0x0C, 0x30, 0xDB, 0xC2, 0xFE, 0x68, 0x42, 0x3D, 0x33, 0xFA, 0x6D, 0xA0, 0xD3, 0x6F,
        0x03, 0x1F, 0x87, 0x75, 0x3C, 0x1E, 0x81, 0x58, 0x88, 0xAA, 0xF4, 0x90, 0x56, 0xA1, 0x93, 0x64};
    uint8_t plaintext_2[LENGTH2] = {
        0xA7, 0xF1, 0xD9, 0x2A, 0x82, 0xC8, 0xD8, 0xFE, 0x43, 0x4D, 0x98, 0x55, 0x8C, 0xE2, 0xB3, 0x47,
        0x17, 0x11, 0x98, 0x54, 0x2F, 0x11, 0x2D, 0x05, 0x58, 0xF5, 0x6B, 0xD6, 0x88, 0x07, 0x99, 0x92,
        0x48, 0x33, 0x62, 0x41, 0xF3, 0x0D, 0x23, 0xE5, 0x5F, 0x30, 0xD1, 0xC8, 0xED, 0x61, 0x0C, 0x4B,
        0x02, 0x35, 0x39, 0x81, 0x84, 0xB8, 0x14, 0xA2, 0x9C, 0xB4, 0x5A, 0x67, 0x2A, 0xCA, 0xE5, 0x48,
        0xE9, 0xC5, 0xF1, 0xB0, 0xC4, 0x15, 0x8A, 0xE5, 0x9B, 0x4D, 0x39, 0xF6, 0xF7, 0xE8, 0xA1, 0x05,
        0xD3, 0xFE, 0xED, 0xA5, 0xD5, 0xF3, 0xD9, 0xE4, 0x5B, 0xFA, 0x6C, 0xC3, 0x51, 0xE2, 0x20, 0xAE,
        0x0C, 0xE1, 0x06, 0x98, 0x6D, 0x61, 0xFF, 0x34, 0xA1, 0x1E, 0x19, 0xFD, 0x36, 0x50, 0xE9, 0xB7,
        0x81, 0x8F, 0xC3, 0x3A, 0x1E, 0x0F, 0xC0, 0x2C, 0x44, 0x55, 0x7A, 0xC8, 0xAB, 0x50, 0xC9, 0xB2,
        0xDE, 0xB2, 0xF6, 0xB5, 0xE2, 0x4C, 0x4F, 0xDD, 0x9F, 0x88, 0x67, 0xBD, 0xCE, 0x1F, 0xF2, 0x61,
        0x00, 0x8E, 0x78, 0x97, 0x97, 0x0E, 0x34, 0x62, 0x07, 0xD7, 0x5E, 0x47, 0xA1, 0x58, 0x29, 0x8E,
        0x5B, 0xA2, 0xF5, 0x62, 0x46, 0x86, 0x9C, 0xC4, 0x2E, 0x36, 0x2A, 0x02, 0x73, 0x12, 0x64, 0xE6,
        0x06, 0x87, 0xEF, 0x53, 0x09, 0xD1, 0x08, 0x53, 0x4F, 0x51, 0xF8, 0x65, 0x8F, 0xB4, 0xF0, 0x80};

    uint8_t CT_TMP[LENGTH2] = {
        0,
    };

    uint8_t CT0[LENGTH0] = {
        0xEC, 0x83, 0x3A, 0xB7, 0xFB, 0xB0, 0xD3, 0x65, 0xB6, 0xE7, 0x2F, 0x50, 0x57, 0x84, 0xE2, 0x43,
        0x47, 0x47, 0xCE, 0xB2, 0x39, 0x39, 0xB9, 0x7D, 0x83, 0x0B, 0x32, 0x32, 0xCF, 0x06, 0x00, 0x25,
        0xBC, 0x48, 0xD6, 0xD2, 0x21, 0xB2, 0x55, 0xEB, 0x4A, 0x45, 0xA0, 0x68, 0xD0, 0x46, 0x18, 0x38,
        0x10, 0xFF, 0xE5, 0x03, 0x7E, 0xF7, 0xB7, 0x25, 0xAB, 0xC0, 0x26, 0x07, 0x28, 0x1F, 0x6D, 0x85};
    uint8_t CT1[LENGTH1] = {
        0x49, 0x78, 0x8B, 0x7C, 0x18, 0x56, 0x0F, 0x1A, 0xB1, 0xA7, 0x8F, 0x94, 0x88, 0xE0, 0x8F, 0x46,
        0x0E, 0x7F, 0x53, 0x7B, 0xE6, 0x40, 0x02, 0x84, 0x32, 0xAF, 0xEE, 0xD0, 0x29, 0x73, 0x0D, 0x1D,
        0xBF, 0xCE, 0x60, 0x29, 0xDE, 0xB1, 0xA0, 0xC2, 0xCA, 0x77, 0x34, 0xED, 0x70, 0x38, 0x5E, 0x78,
        0x89, 0xB6, 0x8C, 0x80, 0xBC, 0xBE, 0x37, 0xC0, 0xCB, 0x32, 0xB0, 0x2C, 0xEC, 0xA6, 0x06, 0xA4,
        0x50, 0x87, 0xFD, 0x41, 0xD1, 0xA4, 0x32, 0x19, 0x59, 0xBA, 0xDB, 0xE4, 0x82, 0xCE, 0xF5, 0x69,
        0xAE, 0xD4, 0x67, 0xBD, 0xEA, 0x11, 0x8F, 0xDF, 0x53, 0x34, 0x12, 0x6F, 0x73, 0x0C, 0x10, 0x3F,
        0x29, 0xEE, 0x80, 0x82, 0xCF, 0xBC, 0x0C, 0x14, 0x97, 0x6D, 0x7C, 0xDE, 0x41, 0x24, 0x1A, 0x30,
        0x8B, 0xAB, 0x21, 0x97, 0x34, 0xD5, 0x5E, 0x08, 0x25, 0xA7, 0x56, 0xFD, 0x61, 0xE0, 0xB9, 0xA6};
    uint8_t CT2[LENGTH2] = {
        0xC6, 0x1E, 0x1A, 0xC8, 0x88, 0x1A, 0x29, 0x9A, 0xB1, 0xE0, 0xFF, 0xA7, 0x55, 0xC7, 0xD2, 0xEF,
        0x55, 0x21, 0x85, 0x92, 0xE1, 0xF1, 0xC1, 0x3F, 0x7C, 0xEC, 0x87, 0x40, 0x38, 0xF2, 0xB0, 0x1F,
        0xB8, 0xCD, 0x5B, 0x61, 0x78, 0x08, 0xCC, 0x13, 0x46, 0x56, 0x0A, 0xDA, 0xCD, 0x7B, 0x2E, 0x97,
        0xC3, 0xA3, 0x14, 0x18, 0x44, 0x26, 0xB9, 0xAC, 0xAC, 0xE0, 0x5B, 0x0D, 0xA0, 0x55, 0xD0, 0xB1,
        0x0F, 0xD4, 0x49, 0xA1, 0xCB, 0xC1, 0x37, 0x69, 0x63, 0x27, 0xF1, 0x92, 0x40, 0x79, 0x24, 0xCE,
        0xA9, 0x90, 0x68, 0xC8, 0xBE, 0xBC, 0x65, 0x43, 0x13, 0x10, 0x00, 0x5E, 0x21, 0xA3, 0x85, 0x1D,
        0xB6, 0xAB, 0xC3, 0x4D, 0xD3, 0xED, 0x81, 0x48, 0x9F, 0xEA, 0x9F, 0xE2, 0xF1, 0x31, 0x9C, 0xC6,
        0xCF, 0xD8, 0x1D, 0xCC, 0x08, 0x4C, 0x7C, 0x92, 0xA6, 0xDD, 0x39, 0xF6, 0xFB, 0x2E, 0xCB, 0x34,
        0x00, 0x71, 0xB8, 0x9C, 0x72, 0xFC, 0x96, 0x6E, 0x70, 0x72, 0xFD, 0x60, 0x8C, 0x12, 0x9F, 0x2E,
        0xAB, 0x2E, 0x16, 0x86, 0xCD, 0x98, 0x1F, 0xDD, 0xE6, 0xA4, 0x82, 0x9D, 0x47, 0xA3, 0x70, 0xBF,
        0x53, 0xC8, 0xCD, 0x69, 0xCD, 0x47, 0x3C, 0xFC, 0x2E, 0xBE, 0x16, 0x7F, 0x8C, 0x52, 0x42, 0x55,
        0x0B, 0x5B, 0x1D, 0x37, 0xAA, 0xD5, 0x75, 0xC5, 0xBB, 0xE6, 0x42, 0x95, 0x59, 0x88, 0xF5, 0x17};

    uint8_t AUTH_TMP[8] = {
        0,
    };

    uint8_t AUTH0[8] = {0x8B, 0x76, 0x4F, 0x3B, 0x4D, 0xC4, 0x17, 0x73};
    uint8_t AUTH1[8] = {0xC4, 0x47, 0xEC, 0xB3, 0x2D, 0xF0, 0xA7, 0x5F};
    uint8_t AUTH2[8] = {0x51, 0x85, 0x2C, 0x12, 0x91, 0xA9, 0xB0, 0xF2};

    uint8_t master_key_0[8] = {0xF5, 0xD3, 0x8D, 0x7F, 0x87, 0x58, 0x88, 0xFC};
    uint8_t master_key_1[8] = {0x47, 0x33, 0xC9, 0xFC, 0x8E, 0x35, 0x88, 0x11};
    uint8_t master_key_2[8] = {0xD8, 0x99, 0x28, 0xC3, 0xDA, 0x29, 0x6B, 0xB0};

    uint32_t i = 0;

    long long int cycles, cycles1, cycles2;

    printf("--- TEST VECTOR ---\n");

    encrypt_hash(plaintext_0, master_key_0, CT_TMP, AUTH_TMP, LENGTH0);
    for (i = 0; i < LENGTH0; i++)
    {
        if (CT_TMP[i] != CT0[i])
        {
            printf("wrong result.\n");
            return 0;
        }
        CT_TMP[i] = 0;
    }
    for (i = 0; i < 8; i++)
    {
        if (AUTH_TMP[i] != AUTH0[i])
        {
            printf("wrong result.\n");
            return 0;
        }
        AUTH_TMP[i] = 0;
    }

    encrypt_hash(plaintext_1, master_key_1, CT_TMP, AUTH_TMP, LENGTH1);
    for (i = 0; i < LENGTH1; i++)
    {
        if (CT_TMP[i] != CT1[i])
        {
            printf("wrong result.\n");
            return 0;
        }
        CT_TMP[i] = 0;
    }
    for (i = 0; i < 8; i++)
    {
        if (AUTH_TMP[i] != AUTH1[i])
        {
            printf("wrong result.\n");
            return 0;
        }
        AUTH_TMP[i] = 0;
    }

    encrypt_hash(plaintext_2, master_key_2, CT_TMP, AUTH_TMP, LENGTH2);
    for (i = 0; i < LENGTH2; i++)
    {
        if (CT_TMP[i] != CT2[i])
        {
            printf("wrong result.\n");
            return 0;
        }
        CT_TMP[i] = 0;
    }
    for (i = 0; i < 8; i++)
    {
        if (AUTH_TMP[i] != AUTH2[i])
        {
            printf("wrong result.\n");
            return 0;
        }
        AUTH_TMP[i] = 0;
    }
    printf("test pass. \n");

    printf("--- BENCHMARK ---\n");
    cycles = 0;
    cycles1 = cpucycles();
    for (i = 0; i < BENCH_ROUND; i++)
    {
        ENC_AUTH(plaintext_2, master_key_2, CT_TMP, AUTH_TMP, LENGTH2);
    }
    cycles2 = cpucycles();
    cycles = cycles2 - cycles1;
    printf("Original implementation runs in ................. %8lld cycles", cycles / BENCH_ROUND);
    printf("\n");

    cycles = 0;
    cycles1 = cpucycles();
    for (i = 0; i < BENCH_ROUND; i++)
    {
        encrypt_hash(plaintext_2, master_key_2, CT_TMP, AUTH_TMP, LENGTH2);
    }
    cycles2 = cpucycles();
    cycles = cycles2 - cycles1;
    printf("Improved implementation runs in ................. %8lld cycles", cycles / BENCH_ROUND);
    printf("\n");

    return 0;
}
