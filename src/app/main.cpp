#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <random>

struct ConstBuffer { const uint8_t* data; size_t size; };
struct MutBuffer { uint8_t* data; size_t size; };

extern "C" int encrypt(ConstBuffer input, ConstBuffer key, MutBuffer* output);
extern "C" int decrypt(ConstBuffer input, ConstBuffer key, MutBuffer* output);

std::vector<uint8_t> read_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
}

void write_file(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

int main(int argc, char* argv[]) {
    std::string mode, input_file, output_file, key_file;
    
    // Простой парсинг
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            std::cout << "Usage:\n  -m encrypt|decrypt|generate-key\n  -i <input>\n  -o <output>\n  -k <key>\n";
            return 0;
        }
        else if (arg == "-m" && i+1 < argc) mode = argv[++i];
        else if (arg == "-i" && i+1 < argc) input_file = argv[++i];
        else if (arg == "-o" && i+1 < argc) output_file = argv[++i];
        else if (arg == "-k" && i+1 < argc) key_file = argv[++i];
    }
    
    // Генерация ключа
    if (mode == "generate-key") {
        std::vector<uint8_t> key(16);
        std::random_device rd;
        for (size_t i = 0; i < key.size(); i++)
            key[i] = rd() % 256;
        
        if (!output_file.empty())
            write_file(output_file, key);
        else
            std::cout.write((char*)key.data(), key.size());
        return 0;
    }
    
    // Шифрование/расшифрование
    if (mode.empty() || input_file.empty() || key_file.empty()) {
        std::cerr << "Error: missing arguments\n";
        return 1;
    }
    
    std::vector<uint8_t> input = read_file(input_file);
    std::vector<uint8_t> key = read_file(key_file);
    
    MutBuffer out_buf = {nullptr, 0};
    ConstBuffer in_buf{input.data(), input.size()};
    ConstBuffer key_buf{key.data(), key.size()};
    
    int err = (mode == "encrypt") ? encrypt(in_buf, key_buf, &out_buf)
                                  : decrypt(in_buf, key_buf, &out_buf);
    
    if (err) { std::cerr << "Operation failed\n"; return 1; }
    
    write_file(output_file, std::vector<uint8_t>(out_buf.data, out_buf.data + out_buf.size));
    delete[] out_buf.data;
    
    return 0;
}
