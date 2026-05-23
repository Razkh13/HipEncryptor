#include <vector>
#include <cstdint>
#include <cstring>
#include "src/core/Blowfish/blowfish.h"

extern "C"
{
    struct ConstBuffer
    {
        const uint8_t* data;
        size_t size;
    };

    struct MutBuffer
    {
        uint8_t* data;
        size_t size;
    };
}

static std::vector<uint8_t> add_padding(const std::vector<uint8_t>& data)
{
    std::vector<uint8_t> buffer = data;
    size_t padding = 8 - (buffer.size() % 8);
    if (padding == 0) padding = 8;
    buffer.insert(buffer.end(), padding, static_cast<uint8_t>(padding));
    return buffer;
}

static void remove_padding(std::vector<uint8_t>& buffer)
{
    if (buffer.empty()) return;
    uint8_t padding = buffer.back();
    if (padding > 0 && padding <= 8)
    {
        buffer.resize(buffer.size() - padding);
    }
}

extern "C" int encrypt(ConstBuffer input, ConstBuffer key, MutBuffer* output)
{
    try
    {
        std::vector<uint8_t> in(input.data, input.data + input.size);
        std::vector<uint8_t> k(key.data, key.data + key.size);
        Blowfish bf(k);
        std::vector<uint8_t> buffer = add_padding(in);
        for (size_t i = 0; i < buffer.size(); i += 8)
        {
            uint32_t L =
                (buffer[i] << 24) |
                (buffer[i + 1] << 16) |
                (buffer[i + 2] << 8) |
                buffer[i + 3];

            uint32_t R =
                (buffer[i + 4] << 24) |
                (buffer[i + 5] << 16) |
                (buffer[i + 6] << 8) |
                buffer[i + 7];

            bf.encryptBlock(L, R);

            buffer[i]     = (L >> 24) & 0xFF;
            buffer[i + 1] = (L >> 16) & 0xFF;
            buffer[i + 2] = (L >> 8)  & 0xFF;
            buffer[i + 3] = L & 0xFF;

            buffer[i + 4] = (R >> 24) & 0xFF;
            buffer[i + 5] = (R >> 16) & 0xFF;
            buffer[i + 6] = (R >> 8)  & 0xFF;
            buffer[i + 7] = R & 0xFF;
        }

        output->size = buffer.size();
        output->data = new uint8_t[buffer.size()];
        std::memcpy(output->data, buffer.data(), buffer.size());

        return 0;
    }
    catch (...)
    {
        return 1;
    }
}


extern "C" int decrypt(ConstBuffer input, ConstBuffer key, MutBuffer* output)
{
    try
    {
        std::vector<uint8_t> in(input.data, input.data + input.size);
        std::vector<uint8_t> k(key.data, key.data + key.size);

        Blowfish bf(k);

        for (size_t i = 0; i < in.size(); i += 8)
        {
            uint32_t L =
                (in[i] << 24) |
                (in[i + 1] << 16) |
                (in[i + 2] << 8) |
                in[i + 3];

            uint32_t R =
                (in[i + 4] << 24) |
                (in[i + 5] << 16) |
                (in[i + 6] << 8) |
                in[i + 7];

            bf.decryptBlock(L, R);

            in[i]     = (L >> 24) & 0xFF;
            in[i + 1] = (L >> 16) & 0xFF;
            in[i + 2] = (L >> 8)  & 0xFF;
            in[i + 3] = L & 0xFF;

            in[i + 4] = (R >> 24) & 0xFF;
            in[i + 5] = (R >> 16) & 0xFF;
            in[i + 6] = (R >> 8)  & 0xFF;
            in[i + 7] = R & 0xFF;
        }

        remove_padding(in);

        output->size = in.size();
        output->data = new uint8_t[in.size()];
        std::memcpy(output->data, in.data(), in.size());

        return 0;
    }
    catch (...)
    {
        return 1;
    }
}