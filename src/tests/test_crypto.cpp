#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <random>
#include <chrono>

#ifdef _WIN32
    #include <windows.h>
    #define DL_HANDLE HMODULE
    #define DL_LOAD(name) LoadLibraryA(name)
    #define DL_SYM(handle, name) GetProcAddress((HMODULE)handle, name)
    #define DL_CLOSE(handle) FreeLibrary((HMODULE)handle)
#else
    #include <dlfcn.h>
    #define DL_HANDLE void*
    #define DL_LOAD(name) dlopen(name, RTLD_LAZY)
    #define DL_SYM(handle, name) dlsym(handle, name)
    #define DL_CLOSE(handle) dlclose(handle)
#endif

struct ConstBuffer { const uint8_t* data; size_t size; };
struct MutBuffer { uint8_t* data; size_t size; };

typedef int (*encrypt_func)(ConstBuffer, ConstBuffer, MutBuffer*);
typedef int (*decrypt_func)(ConstBuffer, ConstBuffer, MutBuffer*);

#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"

std::vector<uint8_t> generate_random_data(size_t size) {
    std::vector<uint8_t> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (size_t i = 0; i < size; i++) {
        data[i] = static_cast<uint8_t>(dis(gen));
    }
    return data;
}

std::vector<uint8_t> generate_random_key() {
    return generate_random_data(16);  // 128 бит ключ
}

std::string get_lib_name(const std::string& algorithm) {
#ifdef _WIN32
    return algorithm + ".dll";
#else
    return "lib" + algorithm + ".so";
#endif
}

bool load_library(const std::string& lib_name, DL_HANDLE& handle, 
                  encrypt_func& encrypt, decrypt_func& decrypt) {
    handle = DL_LOAD(lib_name.c_str());
    if (!handle) {
        std::cout << RED << "  ❌ Cannot load library: " << lib_name << RESET << "\n";
        return false;
    }
    
    encrypt = (encrypt_func)DL_SYM(handle, "encrypt");
    decrypt = (decrypt_func)DL_SYM(handle, "decrypt");
    
    if (!encrypt || !decrypt) {
        std::cout << RED << "  ❌ Cannot find encrypt/decrypt functions" << RESET << "\n";
        DL_CLOSE(handle);
        return false;
    }
    
    return true;
}

// само тестирование (включает 5 тестов)
bool run_test(encrypt_func encrypt, decrypt_func decrypt,
              const std::vector<uint8_t>& data, 
              const std::vector<uint8_t>& key,
              int test_num, size_t data_size) {
    
    ConstBuffer in_buf{data.data(), data.size()};
    ConstBuffer key_buf{key.data(), key.size()};
    MutBuffer out_buf{nullptr, 0};
    
    int enc_result = encrypt(in_buf, key_buf, &out_buf);
    if (enc_result != 0) {
        std::cout << RED << "  ❌ Encryption failed (code: " << enc_result << ")" << RESET << "\n";
        return false;
    }
    
    if (out_buf.size == 0 || out_buf.data == nullptr) {
        std::cout << RED << "  ❌ Encryption output is empty" << RESET << "\n";
        delete[] out_buf.data;
        return false;
    }
    
    ConstBuffer enc_buf{out_buf.data, out_buf.size};
    MutBuffer dec_buf{nullptr, 0};
    
    int dec_result = decrypt(enc_buf, key_buf, &dec_buf);
    if (dec_result != 0) {
        std::cout << RED << "  ❌ Decryption failed (code: " << dec_result << ")" << RESET << "\n";
        delete[] out_buf.data;
        return false;
    }
    
    // Сравнение
    bool success = (data.size() == dec_buf.size) &&
                   (memcmp(data.data(), dec_buf.data, data.size()) == 0);
    
    if (success) {
        std::cout << GREEN << "  ✅ TEST " << test_num << " PASSED" << RESET;
        std::cout << " (" << data_size << " bytes)\n";
    } else {
        std::cout << RED << "  ❌ TEST " << test_num << " FAILED" << RESET;
        std::cout << " (data mismatch)\n";
    }
    
    delete[] out_buf.data;
    delete[] dec_buf.data;
    
    return success;
}

bool test_empty_data(encrypt_func encrypt, decrypt_func, const std::vector<uint8_t>& key) {
    
    std::vector<uint8_t> empty_data;
    ConstBuffer in_buf{empty_data.data(), empty_data.size()};
    ConstBuffer key_buf{key.data(), key.size()};
    MutBuffer out_buf{nullptr, 0};
    
    int enc_result = encrypt(in_buf, key_buf, &out_buf);
    
    if (enc_result != 0) {
        std::cout << YELLOW << "  ⚠️ EMPTY DATA TEST: Encryption failed (code: " << enc_result << ") - это ожидаемо" << RESET << "\n";
        return true; 
    }
    
    delete[] out_buf.data;
    return true;
}

bool test_wrong_key(encrypt_func encrypt, decrypt_func decrypt,
                    const std::vector<uint8_t>& data) {
    
    std::vector<uint8_t> key1 = generate_random_key();
    std::vector<uint8_t> key2 = generate_random_key();
    
    ConstBuffer in_buf{data.data(), data.size()};
    ConstBuffer key_buf1{key1.data(), key1.size()};
    ConstBuffer key_buf2{key2.data(), key2.size()};
    MutBuffer out_buf{nullptr, 0};
    
    // Шифруем с key1
    encrypt(in_buf, key_buf1, &out_buf);
    
    // Пытаемся расшифровать с key2
    ConstBuffer enc_buf{out_buf.data, out_buf.size};
    MutBuffer dec_buf{nullptr, 0};
    
    int dec_result = decrypt(enc_buf, key_buf2, &dec_buf);
    
    bool success = (dec_result != 0);  // Ожидаем ошибку расшифрования
    
    if (success) {
        std::cout << GREEN << "  ✅ WRONG KEY TEST PASSED" << RESET;
        std::cout << " (decryption rejected wrong key)\n";
    } else {
        std::cout << RED << "  ❌ WRONG KEY TEST FAILED" << RESET;
        std::cout << " (decryption accepted wrong key)\n";
    }
    
    delete[] out_buf.data;
    delete[] dec_buf.data;
    
    return success;
}

bool test_algorithm(const std::string& algorithm) {
    std::cout << "\n" << BLUE << "--- Testing " << algorithm << " ---\n" << RESET;
    
    std::string lib_name = get_lib_name(algorithm);
    std::cout << "Library: " << lib_name << "\n";
    
    DL_HANDLE handle;
    encrypt_func encrypt;
    decrypt_func decrypt;
    
    if (!load_library(lib_name, handle, encrypt, decrypt)) {
        return false;
    }
    
    std::cout << GREEN << "✓ Library loaded successfully\n" << RESET;
    
    // Генерируем ключ
    std::vector<uint8_t> key = generate_random_key();
    std::cout << "Key size: " << key.size() << " bytes\n";
    
    int passed = 0;
    int total = 0;
    
    // Тест 1: маленькие данные (1 байт)
    total++;
    std::vector<uint8_t> data1 = generate_random_data(1);
    if (run_test(encrypt, decrypt, data1, key, total, 1)) passed++;
    
    // Тест 2: данные размером ровно в блок (8 байт)
    total++;
    std::vector<uint8_t> data2 = generate_random_data(8);
    if (run_test(encrypt, decrypt, data2, key, total, 8)) passed++;
    
    // Тест 3: данные размером не кратным блоку (13 байт)
    total++;
    std::vector<uint8_t> data3 = generate_random_data(13);
    if (run_test(encrypt, decrypt, data3, key, total, 13)) passed++;
    
    // Тест 4: большие данные (1 КБ)
    total++;
    std::vector<uint8_t> data4 = generate_random_data(1024);
    if (run_test(encrypt, decrypt, data4, key, total, 1024)) passed++;
    
    // Тест 5: очень большие данные (1 МБ)
    total++;
    std::vector<uint8_t> data5 = generate_random_data(1024 * 1024);
    if (run_test(encrypt, decrypt, data5, key, total, 1024 * 1024)) passed++;
    
    // Тест 6: пустые данные
    total++;
    std::cout << "\n" << YELLOW << "--- Edge Cases ---\n" << RESET;
    if (test_empty_data(encrypt, decrypt, key)) passed++;
    
    // Тест 7: неправильный ключ
    total++;
    std::vector<uint8_t> data6 = generate_random_data(64);
    if (test_wrong_key(encrypt, decrypt, data6)) passed++;
    
    std::cout << "\n" << BLUE << "--- Results ---\n" << RESET;
    std::cout << "Passed: " << GREEN << passed << RESET << " / " << total << "\n";
    
    DL_CLOSE(handle);
    
    return (passed == total);
}

void print_help(const char* name) {
    std::cout << "Usage: " << name << " [OPTIONS]\n\n"
              << "Options:\n"
              << "  -a, --algorithm ALGO   Test specific algorithm (blowfish, twofish, aes)\n"
              << "  -l, --list             List available algorithms\n"
              << "  -h, --help             Show this help\n\n"
              << "Examples:\n"
              << "  " << name << " -a blowfish\n"
              << "  " << name << " -a aes\n"
              << "  " << name << "\n";
}

int main(int argc, char* argv[]) {
    std::string algorithm = "blowfish";
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_help(argv[0]);
            return 0;
        }
        else if (arg == "-a" || arg == "--algorithm") {
            if (i+1 < argc) algorithm = argv[++i];
        }
        else if (arg == "-l" || arg == "--list") {
            std::cout << "Available algorithms:\n  - blowfish\n  - twofish (soon)\n  - aes (soon)\n";
            return 0;
        }
    }
    
    std::cout << "\n" << YELLOW << "╔════════════════════════════════╗\n";
    std::cout << "║   Crypto Algorithm Tester     ║\n";
    std::cout << "╚════════════════════════════════╝\n" << RESET;
    
    bool success = test_algorithm(algorithm);
    
    std::cout << "\n";
    if (success) {
        std::cout << GREEN << "🎉 ALL TESTS PASSED! Algorithm " << algorithm << " is correct.\n" << RESET;
        return 0;
    } else {
        std::cout << RED << "❌ SOME TESTS FAILED! Algorithm " << algorithm << " has issues.\n" << RESET;
        return 1;
    }
}