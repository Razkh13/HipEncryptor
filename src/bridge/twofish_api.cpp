#include <vector>
#include <cstdint>
#include <cstring>
#include "../core/Twofish/twofish.h"

extern "C"
{
    struct ConstBuffer { const uint8_t* data; size_t size; };
    struct MutBuffer { uint8_t* data; size_t size; };
}

static std::vector<uint8_t> add_padding(const std::vector<uint8_t>& data) {
    if (data.empty()) return {};
    std::vector<uint8_t> buffer = data;
    size_t padding = 16 - (buffer.size() % 16);
    if (padding == 0) padding = 16;
    buffer.insert(buffer.end(), padding, static_cast<uint8_t>(padding));
    return buffer;
}

static void remove_padding(std::vector<uint8_t>& buffer) {
    if (buffer.empty()) return;
    uint8_t padding = buffer.back();
    if (padding > 0 && padding <= 16 && padding <= buffer.size()) {
        buffer.resize(buffer.size() - padding);
    }
}

static void bytes_to_words(const uint8_t* bytes, uint32_t* words, int count) {
    for (int i = 0; i < count; i++) {
        words[i] = 0;
        for (int j = 0; j < 4; j++) {
            words[i] |= (static_cast<uint32_t>(bytes[i * 4 + j]) << (j * 8));
        }
    }
}

static void words_to_bytes(const uint32_t* words, uint8_t* bytes, int count) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < 4; j++) {
            bytes[i * 4 + j] = (words[i] >> (j * 8)) & 0xFF;
        }
    }
}

extern "C" int encrypt(ConstBuffer input, ConstBuffer key, MutBuffer* output) {
    try {
        if (!input.data || input.size == 0) return 1;
        if (!key.data || key.size == 0) return 2;
        if (!output) return 3;
        
        std::vector<uint8_t> in(input.data, input.data + input.size);
        std::vector<uint8_t> k(key.data, key.data + key.size);
        
        uint32_t key_words[8];
        int word_count;
        
        if (k.size() == 16) word_count = 4;
        else if (k.size() == 24) word_count = 6;
        else if (k.size() == 32) word_count = 8;
        else return 10;
        
        for (int i = 0; i < word_count; i++) {
            key_words[i] = 0;
            for (int j = 0; j < 4; j++) {
                if ((size_t)(i * 4 + j) < k.size()) {
                    key_words[i] |= (static_cast<uint32_t>(k[i * 4 + j]) << (j * 8));
                }
            }
        }
        
        if (twofishInit(key_words, word_count) != 0) return 11;
        
        std::vector<uint8_t> padded = add_padding(in);
        std::vector<uint8_t> encrypted(padded.size());
        
        for (size_t i = 0; i < padded.size(); i += 16) {
            uint32_t plain_block[4];
            uint32_t cipher_block[4];
            
            bytes_to_words(&padded[i], plain_block, 4);
            twofishEncrypt(plain_block, cipher_block);
            words_to_bytes(cipher_block, &encrypted[i], 4);
        }
        
        output->size = encrypted.size();
        output->data = new uint8_t[encrypted.size()];
        if (!output->data) return 5;
        
        std::memcpy(output->data, encrypted.data(), encrypted.size());
        
        twofishDestroy();
        return 0;
    }
    catch (...) {
        twofishDestroy();
        return 6;
    }
}

extern "C" int decrypt(ConstBuffer input, ConstBuffer key, MutBuffer* output) {
    try {
        if (!input.data || input.size == 0) return 1;
        if (!key.data || key.size == 0) return 2;
        if (!output) return 3;
        if (input.size % 16 != 0) return 7;
        
        std::vector<uint8_t> in(input.data, input.data + input.size);
        std::vector<uint8_t> k(key.data, key.data + key.size);
        
        uint32_t key_words[8];
        int word_count;
        
        if (k.size() == 16) word_count = 4;
        else if (k.size() == 24) word_count = 6;
        else if (k.size() == 32) word_count = 8;
        else return 10;
        
        for (int i = 0; i < word_count; i++) {
            key_words[i] = 0;
            for (int j = 0; j < 4; j++) {
                if ((size_t)(i * 4 + j) < k.size()) {
                    key_words[i] |= (static_cast<uint32_t>(k[i * 4 + j]) << (j * 8));
                }
            }
        }
        
        if (twofishInit(key_words, word_count) != 0) return 11;
        
        std::vector<uint8_t> decrypted(in.size());
        
        for (size_t i = 0; i < in.size(); i += 16) {
            uint32_t cipher_block[4];
            uint32_t plain_block[4];
            
            bytes_to_words(&in[i], cipher_block, 4);
            twofishDecrypt(cipher_block, plain_block);
            words_to_bytes(plain_block, &decrypted[i], 4);
        }
        
        remove_padding(decrypted);
        
        output->size = decrypted.size();
        output->data = new uint8_t[decrypted.size()];
        if (!output->data) return 5;
        
        std::memcpy(output->data, decrypted.data(), decrypted.size());
        
        twofishDestroy();
        return 0;
    }
    catch (...) {
        twofishDestroy();
        return 6;
    }
}

extern "C" const char* get_algorithm_name() {
    return "Twofish";
}

extern "C" size_t get_key_size() {
    return 16;
}