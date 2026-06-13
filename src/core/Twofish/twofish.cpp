#include "twofish.h"
#include <cstring>
#include <cstdio>

#define ROL32(a, b) (((a) << (b)) | ((a) >> (32 - (b))))
#define ROR32(a, b) (((a) >> (b)) | ((a) << (32 - (b))))
#define ROR4(a, b) ((((a) & 0xf) >> (b)) | (((a) & 0xf) << (4 - (b))))

static const uint32_t RO = 0x01010101;

static const uint8_t MDS[] = {
    0x01, 0xEF, 0x5B, 0x5B, 0x5B, 0xEF, 0xEF, 0x01,
    0xEF, 0x5B, 0x01, 0xEF, 0xEF, 0x01, 0xEF, 0x5B
};

static const uint8_t RS[] = {
    0x01, 0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E,
    0xA4, 0x56, 0x82, 0xF3, 0x1E, 0xC6, 0x68, 0xE5,
    0x02, 0xA1, 0xFC, 0xC1, 0x47, 0xAE, 0x3D, 0x19,
    0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E, 0x03
};

static const uint8_t Q0_T0[] = {8,1,7,13,6,15,3,2,0,11,5,9,14,12,10,4};
static const uint8_t Q0_T1[] = {14,12,11,8,1,2,3,5,15,4,10,6,7,0,9,13};
static const uint8_t Q0_T2[] = {11,10,5,14,6,13,9,0,12,8,15,3,2,4,7,1};
static const uint8_t Q0_T3[] = {13,7,15,4,1,2,6,14,9,11,3,0,8,5,12,10};
static const uint8_t Q1_T0[] = {2,8,11,13,15,7,6,14,3,1,9,4,0,10,12,5};
static const uint8_t Q1_T1[] = {1,14,2,11,4,12,3,7,6,13,10,5,15,9,0,8};
static const uint8_t Q1_T2[] = {4,12,7,5,1,6,9,10,0,14,13,8,2,11,3,15};
static const uint8_t Q1_T3[] = {11,9,5,1,12,3,13,14,6,4,7,15,2,0,8,10};

static uint32_t tf_K[40];
static uint32_t tf_S[4];
static int tf_init = 0;

static uint16_t gf28_mul(uint8_t a, uint8_t b) {
    uint16_t res = 0;
    while (a) {
        if (a & 1) res ^= b;
        a >>= 1;
        b <<= 1;
    }
    return res;
}

static uint16_t gf28_mod(uint16_t a, uint16_t poly) {
    while (a & 0xFF00) {
        uint16_t mask = 0x100;
        while (!(a & mask)) mask <<= 1;
        a ^= (poly << (mask >> 4));
    }
    return a;
}

static uint8_t Q0(uint8_t x) {
    uint8_t a0 = x >> 4, b0 = x & 0xf;
    uint8_t a1 = a0 ^ b0;
    uint8_t b1 = a0 ^ ROR4(b0, 1) ^ (a0 << 3);
    uint8_t a2 = Q0_T0[a1];
    uint8_t b2 = Q0_T1[b1];
    uint8_t a3 = a2 ^ b2;
    uint8_t b3 = a2 ^ ROR4(b2, 1) ^ (a2 << 3);
    uint8_t a4 = Q0_T2[a3];
    uint8_t b4 = Q0_T3[b3];
    return (b4 << 4) | a4;
}

static uint8_t Q1(uint8_t x) {
    uint8_t a0 = x >> 4, b0 = x & 0xf;
    uint8_t a1 = a0 ^ b0;
    uint8_t b1 = a0 ^ ROR4(b0, 1) ^ (a0 << 3);
    uint8_t a2 = Q1_T0[a1];
    uint8_t b2 = Q1_T1[b1];
    uint8_t a3 = a2 ^ b2;
    uint8_t b3 = a2 ^ ROR4(b2, 1) ^ (a2 << 3);
    uint8_t a4 = Q1_T2[a3];
    uint8_t b4 = Q1_T3[b3];
    return (b4 << 4) | a4;
}

static uint32_t H(uint32_t X, const uint32_t* L) {
    uint8_t y[4];
    y[0] = X & 0xFF;
    y[1] = (X >> 8) & 0xFF;
    y[2] = (X >> 16) & 0xFF;
    y[3] = (X >> 24) & 0xFF;
    
    y[0] = Q1(y[0]);
    y[1] = Q0(y[1]);
    y[2] = Q0(y[2]);
    y[3] = Q1(y[3]);
    uint32_t t = (y[0]<<24)|(y[1]<<16)|(y[2]<<8)|(y[3] ^ L[3]);
    
    y[0] = Q1(t>>24);
    y[1] = Q1((t>>16)&0xFF);
    y[2] = Q0((t>>8)&0xFF);
    y[3] = Q0(t&0xFF);
    t = (y[0]<<24)|(y[1]<<16)|(y[2]<<8)|(y[3] ^ L[2]);
    
    y[0] = Q0(t>>24);
    y[1] = Q1((t>>16)&0xFF);
    y[2] = Q0((t>>8)&0xFF);
    y[3] = Q1(t&0xFF);
    t = (y[0]<<24)|(y[1]<<16)|(y[2]<<8)|(y[3] ^ L[1]);
    
    y[0] = Q0(t>>24);
    y[1] = Q0((t>>16)&0xFF);
    y[2] = Q1((t>>8)&0xFF);
    y[3] = Q1(t&0xFF);
    t = (y[0]<<24)|(y[1]<<16)|(y[2]<<8)|(y[3] ^ L[0]);
    
    y[0] = Q1(t>>24);
    y[1] = Q0((t>>16)&0xFF);
    y[2] = Q1((t>>8)&0xFF);
    y[3] = Q0(t&0xFF);
    
    uint8_t z[4];
    for (int i = 0; i < 4; i++) {
        z[i] = 0;
        for (int j = 0; j < 4; j++) {
            z[i] ^= gf28_mod(gf28_mul(y[j], MDS[i*4+j]), 0x169);
        }
    }
    return (z[0]<<24)|(z[1]<<16)|(z[2]<<8)|z[3];
}

int twofishInit(const uint32_t* key, int keyWords) {
    if (keyWords != 4 && keyWords != 6 && keyWords != 8) return -1;
    
    uint32_t Me[4] = {0}, Mo[4] = {0};
    for (int i = 0; i < keyWords; i++) {
        if (i < 4) Me[i] = key[i];
        else Mo[i-4] = key[i];
    }
    
    uint8_t m[32] = {0};
    memcpy(m, key, keyWords * 4);
    
    for (int i = 0; i < 4; i++) {
        uint32_t S = 0;
        for (int j = 0; j < 4; j++) {
            uint8_t val = 0;
            for (int k = 0; k < 8; k++) {
                uint8_t byte = (i*4+j < keyWords*4) ? m[i*4+j] : 0;
                val ^= gf28_mod(gf28_mul(RS[j*8+k], byte), 0x14d);
            }
            S |= (val << (24 - j*8));
        }
        tf_S[3-i] = S;
    }
    
    for (int i = 0; i < 20; i++) {
        uint32_t A = H(RO * (i*2), Me);
        uint32_t B = ROL32(H(RO * (i*2+1), Mo), 8);
        tf_K[i*2] = A + B;
        tf_K[i*2+1] = ROL32(A + (B<<1), 9);
    }
    
    tf_init = 1;
    return 0;
}

void twofishEncrypt(const uint32_t* plain, uint32_t* cipher) {
    if (!tf_init) return;
    uint32_t p[4];
    p[0] = plain[0] ^ tf_K[0];
    p[1] = plain[1] ^ tf_K[1];
    p[2] = plain[2] ^ tf_K[2];
    p[3] = plain[3] ^ tf_K[3];
    
    for (int r = 0; r < 16; r++) {
        uint32_t T0 = H(p[0], tf_S);
        uint32_t T1 = H(ROL32(p[1], 8), tf_S);
        uint32_t F0 = T0 + T1 + tf_K[8 + r*2];
        uint32_t F1 = T0 + (T1 << 1) + tf_K[9 + r*2];
        
        p[2] ^= F0;
        p[2] = ROR32(p[2], 1);
        p[3] = ROL32(p[3], 1) ^ F1;
        
        uint32_t tmp = p[0];
        p[0] = p[2];
        p[2] = tmp;
        tmp = p[1];
        p[1] = p[3];
        p[3] = tmp;
    }
    
    cipher[0] = p[0] ^ tf_K[4];
    cipher[1] = p[1] ^ tf_K[5];
    cipher[2] = p[2] ^ tf_K[6];
    cipher[3] = p[3] ^ tf_K[7];
}

void twofishDecrypt(const uint32_t* cipher, uint32_t* plain) {
    if (!tf_init) return;
    uint32_t p[4];
    p[0] = cipher[0] ^ tf_K[4];
    p[1] = cipher[1] ^ tf_K[5];
    p[2] = cipher[2] ^ tf_K[6];
    p[3] = cipher[3] ^ tf_K[7];
    
    for (int r = 15; r >= 0; r--) {
        uint32_t tmp = p[0];
        p[0] = p[2];
        p[2] = tmp;
        tmp = p[1];
        p[1] = p[3];
        p[3] = tmp;
        
        uint32_t T0 = H(p[0], tf_S);
        uint32_t T1 = H(ROL32(p[1], 8), tf_S);
        uint32_t F0 = T0 + T1 + tf_K[8 + r*2];
        uint32_t F1 = T0 + (T1 << 1) + tf_K[9 + r*2];
        
        p[2] = ROL32(p[2], 1) ^ F0;
        p[3] ^= F1;
        p[3] = ROR32(p[3], 1);
    }
    
    plain[0] = p[0] ^ tf_K[0];
    plain[1] = p[1] ^ tf_K[1];
    plain[2] = p[2] ^ tf_K[2];
    plain[3] = p[3] ^ tf_K[3];
}

void twofishDestroy() {
    memset(tf_K, 0, sizeof(tf_K));
    memset(tf_S, 0, sizeof(tf_S));
    tf_init = 0;
}