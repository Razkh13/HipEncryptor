#ifndef TWOFISH_H
#define TWOFISH_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

int twofishInit(const uint32_t* key, int keyWords);
void twofishEncrypt(const uint32_t* plain, uint32_t* cipher);
void twofishDecrypt(const uint32_t* cipher, uint32_t* plain);
void twofishDestroy();

#ifdef __cplusplus
}
#endif

#endif