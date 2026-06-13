#ifndef BLOWFISH_H
#define BLOWFISH_H

#include <cstdint>
#include <vector>

class Blowfish {
public:
    explicit Blowfish(const std::vector<uint8_t>& key);
    void encryptBlock(uint32_t& L, uint32_t& R);
    void decryptBlock(uint32_t& L, uint32_t& R);

private:
    uint32_t F(uint32_t x);
    void keyExpansion(const std::vector<uint8_t>& key);
    uint32_t P[18];
    uint32_t S[4][256];
};

#endif 
