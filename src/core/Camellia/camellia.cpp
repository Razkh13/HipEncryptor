#include "camellia.h"
#include <cstring>
#include <iostream>

using namespace std;

const uint8_t S1[256] = {
    0x70,0x82,0x2C,0xEC,0xB3,0x27,0xC0,0xE5,0xE4,0x85,0x57,0x35,0xEA,0x0C,0xAE,0x41,
    0x23,0xEF,0x6B,0x93,0x45,0x19,0xA5,0x21,0xED,0x0E,0x4F,0x4E,0x1D,0x65,0x92,0xBD,
    0x86,0xB8,0xAF,0x8F,0x7C,0xEB,0x1F,0xCE,0x3E,0x30,0xDC,0x5F,0x5E,0xC5,0x0B,0x1A,
    0xA6,0xE1,0x39,0xCA,0xD5,0x47,0x5D,0x3D,0xD9,0x01,0x5A,0xD6,0x51,0x56,0x6C,0x4D,
    0x8B,0x0D,0x9A,0x66,0xFB,0xCC,0xB0,0x2D,0x74,0x12,0x2B,0x20,0xF0,0xB1,0x84,0x99,
    0xDF,0x4C,0xCB,0xC2,0x34,0x7E,0xFB,0x00,0x94,0x81,0x18,0xCF,0xE2,0x44,0xA1,0x26,
    0x54,0xDE,0xE3,0xB2,0x64,0x1C,0x0F,0xAC,0x3B,0x37,0x9B,0x2A,0x55,0xA2,0x59,0x67,
    0x14,0xC8,0xF3,0xB6,0x28,0x60,0xA4,0x77,0xC7,0x63,0xA8,0x62,0x9D,0x0A,0x7D,0xBC,
    0x52,0xDD,0x9E,0x42,0x22,0x80,0x6F,0x3F,0xC4,0xBA,0x09,0x71,0xBB,0x83,0x0B,0x31,
    0xDF,0x4C,0xCB,0xC2,0x34,0x7E,0xFB,0x00,0x94,0x81,0x18,0xCF,0xE2,0x44,0xA1,0x26,
    0x54,0xDE,0xE3,0xB2,0x64,0x1C,0x0F,0xAC,0x3B,0x37,0x9B,0x2A,0x55,0xA2,0x59,0x67,
    0x14,0xC8,0xF3,0xB6,0x28,0x60,0xA4,0x77,0xC7,0x63,0xA8,0x62,0x9D,0x0A,0x7D,0xBC,
    0x52,0xDD,0x9E,0x42,0x22,0x80,0x6F,0x3F,0xC4,0xBA,0x09,0x71,0xBB,0x83,0x0B,0x31,
    0xDF,0x4C,0xCB,0xC2,0x34,0x7E,0xFB,0x00,0x94,0x81,0x18,0xCF,0xE2,0x44,0xA1,0x26,
    0x54,0xDE,0xE3,0xB2,0x64,0x1C,0x0F,0xAC,0x3B,0x37,0x9B,0x2A,0x55,0xA2,0x59,0x67,
    0x14,0xC8,0xF3,0xB6,0x28,0x60,0xA4,0x77,0xC7,0x63,0xA8,0x62,0x9D,0x0A,0x7D,0xBC
};

uint8_t S2[256], S3[256], S4[256];

void init_sboxes() {
    for (int i = 0; i < 256; i++) {
        S2[i] = ((S1[i] << 1) & 0xFF) | (S1[i] >> 7);
        S3[i] = (S1[i] >> 1) | ((S1[i] << 7) & 0xFF);
        S4[i] = ((S2[i] << 1) & 0xFF) | (S2[i] >> 7);
    }
}

inline uint32_t rotl32(uint32_t x, int n) { return (x << n) | (x >> (32 - n)); }
inline uint64_t rotl64(uint64_t x, int n) { return (x << n) | (x >> (64 - n)); }

uint8_t gf256_mul(uint8_t a, uint8_t b) {
    uint16_t result = 0, aa = a;
    for (int i = 0; i < 8; i++) {
        if (b & 1) result ^= aa;
        aa <<= 1;
        if (aa & 0x100) aa ^= 0x110;
        b >>= 1;
    }
    return (uint8_t)result;
}

uint32_t p_function(uint32_t in) {
    uint8_t x[4], y[4];
    x[0] = (in >> 24) & 0xFF; x[1] = (in >> 16) & 0xFF;
    x[2] = (in >> 8) & 0xFF;  x[3] = in & 0xFF;
    
    y[0] = gf256_mul(0x01, x[0]) ^ gf256_mul(0x02, x[1]) ^ gf256_mul(0x04, x[2]) ^ gf256_mul(0x06, x[3]);
    y[1] = gf256_mul(0x02, x[0]) ^ gf256_mul(0x01, x[1]) ^ gf256_mul(0x06, x[2]) ^ gf256_mul(0x04, x[3]);
    y[2] = gf256_mul(0x04, x[0]) ^ gf256_mul(0x06, x[1]) ^ gf256_mul(0x01, x[2]) ^ gf256_mul(0x02, x[3]);
    y[3] = gf256_mul(0x06, x[0]) ^ gf256_mul(0x04, x[1]) ^ gf256_mul(0x02, x[2]) ^ gf256_mul(0x01, x[3]);
    
    return (y[0] << 24) | (y[1] << 16) | (y[2] << 8) | y[3];
}

void F_function(uint32_t &L, uint32_t R, uint64_t round_key) {
    uint32_t K1 = (round_key >> 32) & 0xFFFFFFFF;
    uint32_t K2 = round_key & 0xFFFFFFFF;
    uint32_t t = R ^ K1;
    
    uint8_t b[4];
    b[0] = (t >> 24) & 0xFF; b[1] = (t >> 16) & 0xFF;
    b[2] = (t >> 8) & 0xFF;  b[3] = t & 0xFF;
    
    b[0] = S1[b[0]]; b[1] = S2[b[1]]; b[2] = S3[b[2]]; b[3] = S4[b[3]];
    t = (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
    t = p_function(t) ^ K2;
    L ^= t;
}

void F_for_key(uint64_t &x, uint64_t &y, uint64_t k) {
    uint32_t xh = (x >> 32) & 0xFFFFFFFF, xl = x & 0xFFFFFFFF;
    uint32_t kh = (k >> 32) & 0xFFFFFFFF, kl = k & 0xFFFFFFFF;
    uint32_t t = xh ^ kh;
    
    uint8_t b[4];
    b[0] = (t >> 24) & 0xFF; b[1] = (t >> 16) & 0xFF;
    b[2] = (t >> 8) & 0xFF;  b[3] = t & 0xFF;
    
    b[0] = S1[b[0]]; b[1] = S2[b[1]]; b[2] = S3[b[2]]; b[3] = S4[b[3]];
    t = (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
    t = p_function(t) ^ kl;
    
    y ^= ((uint64_t)t << 32) ^ xl;
    x = ((uint64_t)t << 32) ^ xl;
}

const uint64_t SIGMA[4] = {0xA09E667F3BCC908BULL, 0xB67AE8584CAA73B2ULL, 0xC6EF372FE94F82BEULL, 0x54FF53A5F1D36F1CULL};

void camellia128_key_schedule(const uint8_t user_key[16], uint64_t round_keys[38]) {
    for (int i = 0; i < 38; i++) round_keys[i] = 0;
    
    uint64_t kl = 0;
    for (int i = 0; i < 8; i++) kl = (kl << 8) | user_key[i];
    uint64_t kr = 0;
    
    uint64_t d1 = kl, d2 = kr;
    F_for_key(d1, d2, SIGMA[0]);
    d1 ^= kl; d2 ^= kr;
    F_for_key(d1, d2, SIGMA[1]);
    uint64_t ka = d1 ^ d2;
    
    uint64_t kll = kl, klr = kr, kal = ka, kar = 0;
    
    round_keys[0] = kll;
    round_keys[1] = klr;
    
    int idx = 2;
    for (int i = 0; i < 4; i++) {
        kll = rotl64(kll, 15); klr = rotl64(klr, 15);
        kal = rotl64(kal, 15); kar = rotl64(kar, 15);
        round_keys[idx++] = kll;
        round_keys[idx++] = klr;
        round_keys[idx++] = kal;
        round_keys[idx++] = kar;
    }
    
    round_keys[18] = kal;
    round_keys[19] = kar;
    
    kll = rotl64(kll, 15); klr = rotl64(klr, 15);
    round_keys[idx++] = kll;
    round_keys[idx++] = klr;
    
    round_keys[20] = kll;
    round_keys[21] = klr;
    
    for (int i = 22; i < 38; i++) {
        round_keys[i] = round_keys[i - 22];
    }
}

void camellia128_encrypt(const uint8_t plaintext[16], uint8_t ciphertext[16], const uint64_t round_keys[38]) {
    uint32_t D1, D2, D3, D4;
    
    D1 = ((uint32_t)plaintext[0] << 24) | ((uint32_t)plaintext[1] << 16) | 
         ((uint32_t)plaintext[2] << 8) | (uint32_t)plaintext[3];
    D2 = ((uint32_t)plaintext[4] << 24) | ((uint32_t)plaintext[5] << 16) | 
         ((uint32_t)plaintext[6] << 8) | (uint32_t)plaintext[7];
    D3 = ((uint32_t)plaintext[8] << 24) | ((uint32_t)plaintext[9] << 16) | 
         ((uint32_t)plaintext[10] << 8) | (uint32_t)plaintext[11];
    D4 = ((uint32_t)plaintext[12] << 24) | ((uint32_t)plaintext[13] << 16) | 
         ((uint32_t)plaintext[14] << 8) | (uint32_t)plaintext[15];
    
    D1 ^= (round_keys[0] >> 32) & 0xFFFFFFFF;
    D2 ^= round_keys[0] & 0xFFFFFFFF;
    D3 ^= (round_keys[1] >> 32) & 0xFFFFFFFF;
    D4 ^= round_keys[1] & 0xFFFFFFFF;
    
    int key_idx = 2;
    for (int r = 0; r < 18; r++) {
        F_function(D1, D2, round_keys[key_idx++]);
        F_function(D3, D4, round_keys[key_idx++]);
        if (r != 17) {
            swap(D1, D3);
            swap(D2, D4);
        }
    }
    
    D1 ^= (round_keys[20] >> 32) & 0xFFFFFFFF;
    D2 ^= round_keys[20] & 0xFFFFFFFF;
    D3 ^= (round_keys[21] >> 32) & 0xFFFFFFFF;
    D4 ^= round_keys[21] & 0xFFFFFFFF;
    
    ciphertext[0] = (D1 >> 24) & 0xFF; ciphertext[1] = (D1 >> 16) & 0xFF;
    ciphertext[2] = (D1 >> 8) & 0xFF;  ciphertext[3] = D1 & 0xFF;
    ciphertext[4] = (D2 >> 24) & 0xFF; ciphertext[5] = (D2 >> 16) & 0xFF;
    ciphertext[6] = (D2 >> 8) & 0xFF;  ciphertext[7] = D2 & 0xFF;
    ciphertext[8] = (D3 >> 24) & 0xFF; ciphertext[9] = (D3 >> 16) & 0xFF;
    ciphertext[10] = (D3 >> 8) & 0xFF; ciphertext[11] = D3 & 0xFF;
    ciphertext[12] = (D4 >> 24) & 0xFF; ciphertext[13] = (D4 >> 16) & 0xFF;
    ciphertext[14] = (D4 >> 8) & 0xFF; ciphertext[15] = D4 & 0xFF;
}

void camellia128_decrypt(const uint8_t ciphertext[16], uint8_t plaintext[16], const uint64_t round_keys[38]) {
    uint32_t D1, D2, D3, D4;
    
    D1 = ((uint32_t)ciphertext[0] << 24) | ((uint32_t)ciphertext[1] << 16) | 
         ((uint32_t)ciphertext[2] << 8) | (uint32_t)ciphertext[3];
    D2 = ((uint32_t)ciphertext[4] << 24) | ((uint32_t)ciphertext[5] << 16) | 
         ((uint32_t)ciphertext[6] << 8) | (uint32_t)ciphertext[7];
    D3 = ((uint32_t)ciphertext[8] << 24) | ((uint32_t)ciphertext[9] << 16) | 
         ((uint32_t)ciphertext[10] << 8) | (uint32_t)ciphertext[11];
    D4 = ((uint32_t)ciphertext[12] << 24) | ((uint32_t)ciphertext[13] << 16) | 
         ((uint32_t)ciphertext[14] << 8) | (uint32_t)ciphertext[15];
    
    D1 ^= (round_keys[20] >> 32) & 0xFFFFFFFF;
    D2 ^= round_keys[20] & 0xFFFFFFFF;
    D3 ^= (round_keys[21] >> 32) & 0xFFFFFFFF;
    D4 ^= round_keys[21] & 0xFFFFFFFF;
    
    int key_idx = 36;
    for (int r = 17; r >= 0; r--) {
        if (r != 17) {
            swap(D1, D3);
            swap(D2, D4);
        }
        F_function(D1, D2, round_keys[key_idx--]);
        F_function(D3, D4, round_keys[key_idx--]);
    }
    
    D1 ^= (round_keys[0] >> 32) & 0xFFFFFFFF;
    D2 ^= round_keys[0] & 0xFFFFFFFF;
    D3 ^= (round_keys[1] >> 32) & 0xFFFFFFFF;
    D4 ^= round_keys[1] & 0xFFFFFFFF;
    
    plaintext[0] = (D1 >> 24) & 0xFF; plaintext[1] = (D1 >> 16) & 0xFF;
    plaintext[2] = (D1 >> 8) & 0xFF;  plaintext[3] = D1 & 0xFF;
    plaintext[4] = (D2 >> 24) & 0xFF; plaintext[5] = (D2 >> 16) & 0xFF;
    plaintext[6] = (D2 >> 8) & 0xFF;  plaintext[7] = D2 & 0xFF;
    plaintext[8] = (D3 >> 24) & 0xFF; plaintext[9] = (D3 >> 16) & 0xFF;
    plaintext[10] = (D3 >> 8) & 0xFF; plaintext[11] = D3 & 0xFF;
    plaintext[12] = (D4 >> 24) & 0xFF; plaintext[13] = (D4 >> 16) & 0xFF;
    plaintext[14] = (D4 >> 8) & 0xFF; plaintext[15] = D4 & 0xFF;
}

void camellia_init() {
    init_sboxes();
}