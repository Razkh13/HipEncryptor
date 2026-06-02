#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <random>

#ifdef _WIN32
    #include <windows.h>
    #define DL_HANDLE HMODULE
    #define DL_LOAD(name) LoadLibraryA(name)
    #define DL_SYM(handle, name) GetProcAddress((HMODULE)handle, name)
    #define DL_CLOSE(handle) FreeLibrary((HMODULE)handle)
    #define DL_ERROR() GetLastError()
#else
    #include <dlfcn.h>
    #define DL_HANDLE void*
    #define DL_LOAD(name) dlopen(name, RTLD_LAZY)
    #define DL_SYM(handle, name) dlsym(handle, name)
    #define DL_CLOSE(handle) dlclose(handle)
    #define DL_ERROR() dlerror()
#endif

struct ConstBuffer { const uint8_t* data; size_t size; };
struct MutBuffer { uint8_t* data; size_t size; };

typedef int (*encrypt_func)(ConstBuffer, ConstBuffer, MutBuffer*);
typedef int (*decrypt_func)(ConstBuffer, ConstBuffer, MutBuffer*);

void error_exit(const std::string& msg, int code = 1) {
    std::cerr << "Error: " << msg << "\n";
    exit(code);
}

void check_file_exists(const std::string& path) {
    std::ifstream f(path);
    if (!f) error_exit("File not found or cannot open: " + path);
}

std::vector<uint8_t> read_file(const std::string& path) {
    check_file_exists(path);
    std::ifstream file(path, std::ios::binary);
    if (!file) error_exit("Cannot read file: " + path);
    
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    if (size == 0) error_exit("File is empty: " + path);
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    if (!file) error_exit("Error reading file: " + path);
    
    return data;
}

void write_file(const std::string& path, const std::vector<uint8_t>& data) {
    if (data.empty()) error_exit("No data to write to: " + path);
    
    std::ofstream file(path, std::ios::binary);
    if (!file) error_exit("Cannot create/write file: " + path);
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    if (!file) error_exit("Error writing to file: " + path);
}

std::vector<uint8_t> generate_key() {
    std::vector<uint8_t> key(16);
    std::random_device rd;
    if (rd.entropy() == 0) {
        std::cerr << "Warning: Random device has low entropy\n";
    }
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (size_t i = 0; i < key.size(); i++) {
        key[i] = static_cast<uint8_t>(dis(gen));
    }
    return key;
}

std::string get_lib_name(const std::string& algorithm) {
    // Поддерживаемые алгоритмы
    if (algorithm != "blowfish" && algorithm != "twofish") {
        error_exit("Unsupported algorithm: " + algorithm + ". Available: blowfish, twofish");
    }
#ifdef _WIN32
    return algorithm + ".dll";
#else
    return "lib" + algorithm + ".so";
#endif
}

void print_help(const char* name) {
    std::cout << "Usage: " << name << " [OPTIONS]\n\n"
              << "Options:\n"
              << "  -a, --algorithm ALGO   Algorithm (blowfish, twofish)\n"
              << "  -m, --mode MODE        Mode: encrypt, decrypt, generate-key\n"
              << "  -i, --input FILE       Input file (reads from stdin if not specified)\n"
              << "  -o, --output FILE      Output file (writes to stdout if not specified)\n"
              << "  -k, --key FILE         Key file (reads from stdin if not specified)\n"
              << "  -h, --help             Show this help\n\n"
              << "Examples:\n"
              << "  " << name << " -a blowfish -m generate-key -o key.bin\n"
              << "  " << name << " -a blowfish -m encrypt -i file.txt -o file.enc -k key.bin\n"
              << "  " << name << " -a blowfish -m decrypt -i file.enc -o file.txt -k key.bin\n"
              << "  " << name << " -a twofish -m encrypt -i file.txt -o file.enc -k key.bin\n"
              << "  " << name << " -a twofish -m decrypt -i file.enc -o file.txt -k key.bin\n";
}

int main(int argc, char* argv[]) {
    std::string algorithm = "blowfish";
    std::string mode;
    std::string input_file;
    std::string output_file;
    std::string key_file;
    
    // Парсинг аргументов
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            print_help(argv[0]);
            return 0;
        }
        else if (arg == "-a" || arg == "--algorithm") {
            if (i+1 >= argc) error_exit("Missing argument for " + arg);
            algorithm = argv[++i];
        }
        else if (arg == "-m" || arg == "--mode") {
            if (i+1 >= argc) error_exit("Missing argument for " + arg);
            mode = argv[++i];
        }
        else if (arg == "-i" || arg == "--input") {
            if (i+1 >= argc) error_exit("Missing argument for " + arg);
            input_file = argv[++i];
        }
        else if (arg == "-o" || arg == "--output") {
            if (i+1 >= argc) error_exit("Missing argument for " + arg);
            output_file = argv[++i];
        }
        else if (arg == "-k" || arg == "--key") {
            if (i+1 >= argc) error_exit("Missing argument for " + arg);
            key_file = argv[++i];
        }
        else {
            error_exit("Unknown argument: " + arg);
        }
    }
    
    // Режим генерации ключа
    if (mode == "generate-key") {
        std::vector<uint8_t> key = generate_key();
        if (!output_file.empty()) {
            write_file(output_file, key);
            std::cerr << "Key saved to: " << output_file << "\n";
        } else {
            std::cout.write(reinterpret_cast<char*>(key.data()), key.size());
            if (!std::cout) error_exit("Failed to write key to stdout");
        }
        return 0;
    }
    
    // Проверка режима
    if (mode.empty()) {
        error_exit("Mode not specified. Use -m encrypt, -m decrypt, or -m generate-key");
    }
    if (mode != "encrypt" && mode != "decrypt") {
        error_exit("Invalid mode: " + mode + ". Use 'encrypt' or 'decrypt'");
    }
    
    // Динамическая загрузка библиотеки
    std::string lib_name = get_lib_name(algorithm);
    
    std::cerr << "Loading library: " << lib_name << "\n";
    DL_HANDLE handle = DL_LOAD(lib_name.c_str());
    if (!handle) {
        error_exit("Cannot load library " + lib_name + ". Make sure it exists in current directory.");
    }
    
    encrypt_func encrypt = (encrypt_func)DL_SYM(handle, "encrypt");
    decrypt_func decrypt = (decrypt_func)DL_SYM(handle, "decrypt");
    
    if (!encrypt) error_exit("Cannot find 'encrypt' function in " + lib_name);
    if (!decrypt) error_exit("Cannot find 'decrypt' function in " + lib_name);
    
    std::cerr << "Library loaded successfully\n";
    
    // Чтение ключа
    std::vector<uint8_t> key;
    if (!key_file.empty()) {
        key = read_file(key_file);
    } else {
        std::cerr << "Reading key from stdin...\n";
        std::cin.unsetf(std::ios::skipws);
        std::istreambuf_iterator<char> it(std::cin), end;
        key.assign(it, end);
    }
    
    if (key.empty()) error_exit("Key is empty");
    std::cerr << "Key size: " << key.size() << " bytes\n";
    
    // Чтение входных данных
    std::vector<uint8_t> input;
    if (!input_file.empty()) {
        input = read_file(input_file);
    } else {
        std::cerr << "Reading input from stdin...\n";
        std::cin.clear();
        std::cin.unsetf(std::ios::skipws);
        std::istreambuf_iterator<char> it(std::cin), end;
        input.assign(it, end);
    }
    
    if (input.empty()) error_exit("Input data is empty");
    std::cerr << "Input size: " << input.size() << " bytes\n";
    
    // Выполнение операции
    ConstBuffer in_buf{input.data(), input.size()};
    ConstBuffer key_buf{key.data(), key.size()};
    MutBuffer out_buf{nullptr, 0};
    
    int result = (mode == "encrypt") ? encrypt(in_buf, key_buf, &out_buf)
                                     : decrypt(in_buf, key_buf, &out_buf);
    
    if (result != 0) error_exit("Cryptographic operation failed with code: " + std::to_string(result));
    
    if (out_buf.data == nullptr) error_exit("Output buffer is null");
    if (out_buf.size == 0) error_exit("Output buffer is empty");
    
    std::cerr << "Output size: " << out_buf.size << " bytes\n";
    
    // Вывод результата
    if (!output_file.empty()) {
        write_file(output_file, std::vector<uint8_t>(out_buf.data, out_buf.data + out_buf.size));
        std::cerr << "Result saved to: " << output_file << "\n";
    } else {
        std::cout.write(reinterpret_cast<char*>(out_buf.data), out_buf.size);
        if (!std::cout) error_exit("Failed to write output to stdout");
    }
    
    delete[] out_buf.data;
    DL_CLOSE(handle);
    
    std::cerr << "Operation completed successfully!\n";
    return 0;
}