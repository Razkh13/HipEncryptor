#include <vector>
#include <cstdint>
#include <cstring>
#include "../core/Camellia/camellia.h"

extern "C"
{
    struct ConstBuffer { const uint8_t* data; size_t size; };
    struct MutBuffer { uint8_t* data; size_t size; };
}

static bool camellia_initialized = false;

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

extern "C" int encrypt(ConstBuffer input, ConstBuffer key, MutBuffer* output) {
    try {
        if (!input.data || input.size == 0) return 1;
        if (!key.data || key.size != 16) return 2;
        if (!output) return 3;
        
        if (!camellia_initialized) {
            camellia_init();
            camellia_initialized = true;
        }
        
        std::vector<uint8_t> in(input.data, input.data + input.size);
        std::vector<uint8_t> k(key.data, key.data + 16);
        
        uint64_t round_keys[38] = {0};
        camellia128_key_schedule(k.data(), round_keys);
        
        std::vector<uint8_t> padded = add_padding(in);
        std::vector<uint8_t> encrypted(padded.size());
        
        for (size_t i = 0; i < padded.size(); i += 16) {
            camellia128_encrypt(&padded[i], &encrypted[i], round_keys);
        }
        
        output->size = encrypted.size();
        output->data = new uint8_t[encrypted.size()];
        if (!output->data) return 5;
        
        std::memcpy(output->data, encrypted.data(), encrypted.size());
        return 0;
    }
    catch (...) {
        return 6;
    }
}

extern "C" int decrypt(ConstBuffer input, ConstBuffer key, MutBuffer* output) {
    try {
        if (!input.data || input.size == 0) return 1;
        if (!key.data || key.size != 16) return 2;
        if (!output) return 3;
        if (input.size % 16 != 0) return 7;
        
        if (!camellia_initialized) {
            camellia_init();
            camellia_initialized = true;
        }
        
        std::vector<uint8_t> in(input.data, input.data + input.size);
        std::vector<uint8_t> k(key.data, key.data + 16);
        
        uint64_t round_keys[38] = {0};
        camellia128_key_schedule(k.data(), round_keys);
        
        std::vector<uint8_t> decrypted(in.size());
        
        for (size_t i = 0; i < in.size(); i += 16) {
            camellia128_decrypt(&in[i], &decrypted[i], round_keys);
        }
        
        remove_padding(decrypted);
        
        output->size = decrypted.size();
        output->data = new uint8_t[decrypted.size()];
        if (!output->data) return 5;
        
        std::memcpy(output->data, decrypted.data(), decrypted.size());
        return 0;
    }
    catch (...) {
        return 6;
    }
}

extern "C" const char* get_algorithm_name() {
    return "Camellia";
}

extern "C" size_t get_key_size() {
    return 16;
}