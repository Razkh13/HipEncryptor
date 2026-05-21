#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>

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

extern "C" int encrypt(ConstBuffer input, ConstBuffer key, MutBuffer* output);
extern "C" int decrypt(ConstBuffer input, ConstBuffer key, MutBuffer* output);

std::vector<uint8_t> read_file(const std::string& path);
void write_file(const std::string& path, const std::vector<uint8_t>& data);

int main(int argc, char* argv[])
{
    std::string mode;
    std::string input_file;
    std::string output_file;
    std::string key_file;

    // PARSE ARGS
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "-m")
        {
            if (i + 1 < argc)
                mode = argv[++i];
        }
        else if (arg == "-i")
        {
            if (i + 1 < argc)
                input_file = argv[++i];
        }
        else if (arg == "-o")
        {
            if (i + 1 < argc)
                output_file = argv[++i];
        }
        else if (arg == "-k")
        {
            if (i + 1 < argc)
                key_file = argv[++i];
        }
    }

    if (mode.empty())
    {
        std::cout << "Usage:\n"
                  << "  -m encrypt|decrypt|generate-key\n"
                  << "  -i input file\n"
                  << "  -o output file\n"
                  << "  -k key file\n";
        return 0;
    }

    if (mode == "generate-key")
    {
        std::cout << "Key generation will be implemented in next stage\n";
        return 0;
    }

    if (input_file.empty() || output_file.empty() || key_file.empty())
    {
        std::cerr << "Error: missing arguments\n";
        return 1;
    }

    // LOAD DATA
    std::vector<uint8_t> input = read_file(input_file);
    std::vector<uint8_t> key   = read_file(key_file);

    std::vector<uint8_t> output;

    MutBuffer out_buf;

    ConstBuffer in_buf { input.data(), input.size() };
    ConstBuffer key_buf { key.data(), key.size() };

    if (mode == "encrypt")
    {
        encrypt(in_buf, key_buf, &out_buf);
    }
    else if (mode == "decrypt")
    {
        decrypt(in_buf, key_buf, &out_buf);
    }
    else
    {
        std::cerr << "Unknown mode\n";
        return 1;
    }

    output.assign(out_buf.data, out_buf.data + out_buf.size);

    write_file(output_file, output);

    delete[] out_buf.data;

    return 0;
}