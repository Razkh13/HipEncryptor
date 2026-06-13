#ifndef CAMELLIA_H
#define CAMELLIA_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

void camellia_init();
void camellia128_key_schedule(const uint8_t user_key[16], uint64_t round_keys[38]);
void camellia128_encrypt(const uint8_t plaintext[16], uint8_t ciphertext[16], const uint64_t round_keys[38]);
void camellia128_decrypt(const uint8_t ciphertext[16], uint8_t plaintext[16], const uint64_t round_keys[38]);

#ifdef __cplusplus
}
#endif

#endif