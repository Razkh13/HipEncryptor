# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -fPIC
LDFLAGS = -ldl

# Директории 
SRC_DIR = src
APP_DIR = $(SRC_DIR)/app
BRIDGE_DIR = $(SRC_DIR)/bridge
CORE_DIR = $(SRC_DIR)/core
BLOWFISH_DIR = $(CORE_DIR)/Blowfish
TWOFISH_DIR = $(CORE_DIR)/Twofish
CAMELLIA_DIR = $(CORE_DIR)/Camellia
TESTS_DIR = $(SRC_DIR)/tests

# Исходные файлы Blowfish
MAIN_SRC = $(APP_DIR)/main.cpp
BRIDGE_BLOWFISH_SRC = $(BRIDGE_DIR)/blowfish_api.cpp
BLOWFISH_SRC = $(BLOWFISH_DIR)/blowfish.cpp
BLOWFISH_HPP = $(BLOWFISH_DIR)/blowfish.h

# Исходные файлы Twofish
BRIDGE_TWOFISH_SRC = $(BRIDGE_DIR)/twofish_api.cpp
TWOFISH_SRC = $(TWOFISH_DIR)/twofish.cpp
TWOFISH_HPP = $(TWOFISH_DIR)/twofish.h

# Исходные файлы Camellia
BRIDGE_CAMELLIA_SRC = $(BRIDGE_DIR)/camellia_api.cpp
CAMELLIA_SRC = $(CAMELLIA_DIR)/camellia.cpp
CAMELLIA_HPP = $(CAMELLIA_DIR)/camellia.h

# Тесты
TEST_SRC = $(TESTS_DIR)/test_crypto.cpp

BLOWFISH_OBJ = blowfish.o
BRIDGE_BLOWFISH_OBJ = blowfish_api.o
TWOFISH_OBJ = twofish.o
BRIDGE_TWOFISH_OBJ = twofish_api.o
CAMELLIA_OBJ = camellia.o
BRIDGE_CAMELLIA_OBJ = camellia_api.o

# Цели
TARGET = cryptum
LIB_BLOWFISH = libblowfish.so
LIB_TWOFISH = libtwofish.so
LIB_CAMELLIA = libcamellia.so
TEST_TARGET = test_crypto

GREEN = \033[32m
RED = \033[31m
RESET = \033[0m

# Основные цели
.PHONY: all clean help

all: $(LIB_BLOWFISH) $(LIB_TWOFISH) $(LIB_CAMELLIA) $(TARGET) $(TEST_TARGET)
	@echo "$(GREEN)✓ Build complete!$(RESET)"

# Blowfish lib
$(LIB_BLOWFISH): $(BRIDGE_BLOWFISH_OBJ) $(BLOWFISH_OBJ)
	@echo "Building $@..."
	$(CXX) -shared $^ -o $@
	@echo "$(GREEN)✓ $@ created$(RESET)"

$(BRIDGE_BLOWFISH_OBJ): $(BRIDGE_BLOWFISH_SRC) $(BLOWFISH_HPP)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $< -o $@

$(BLOWFISH_OBJ): $(BLOWFISH_SRC) $(BLOWFISH_HPP)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $< -o $@

# Twofish lib
$(LIB_TWOFISH): $(BRIDGE_TWOFISH_OBJ) $(TWOFISH_OBJ)
	@echo "Building $@..."
	$(CXX) -shared $^ -o $@
	@echo "$(GREEN)✓ $@ created$(RESET)"

$(BRIDGE_TWOFISH_OBJ): $(BRIDGE_TWOFISH_SRC) $(TWOFISH_HPP)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $< -o $@

$(TWOFISH_OBJ): $(TWOFISH_SRC) $(TWOFISH_HPP)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $< -o $@

# Camellia lib
$(LIB_CAMELLIA): $(BRIDGE_CAMELLIA_OBJ) $(CAMELLIA_OBJ)
	@echo "Building $@..."
	$(CXX) -shared $^ -o $@
	@echo "$(GREEN)✓ $@ created$(RESET)"

$(BRIDGE_CAMELLIA_OBJ): $(BRIDGE_CAMELLIA_SRC) $(CAMELLIA_HPP)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $< -o $@

$(CAMELLIA_OBJ): $(CAMELLIA_SRC) $(CAMELLIA_HPP)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $< -o $@

$(TARGET): $(MAIN_SRC) $(LIB_BLOWFISH) $(LIB_TWOFISH) $(LIB_CAMELLIA)
	@echo "Building $@..."
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) $< -o $@ $(LDFLAGS)
	@echo "$(GREEN)✓ $@ created$(RESET)"

$(TEST_TARGET): $(TEST_SRC) $(LIB_BLOWFISH) $(LIB_TWOFISH) $(LIB_CAMELLIA)
	@echo "Building $@..."
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) $< -o $@ $(LDFLAGS)
	@echo "$(GREEN)✓ $@ created$(RESET)"

# Запуск команд
.PHONY: run
run: $(TARGET) $(LIB_BLOWFISH) $(LIB_TWOFISH) $(LIB_CAMELLIA)
	@echo "Running cryptum..."
	LD_LIBRARY_PATH=. ./$(TARGET)

.PHONY: test
test: $(TEST_TARGET) $(LIB_BLOWFISH) $(LIB_TWOFISH) $(LIB_CAMELLIA)
	@echo "Running tests for Blowfish..."
	LD_LIBRARY_PATH=. ./$(TEST_TARGET) -a blowfish
	@echo ""
	@echo "Running tests for Twofish..."
	LD_LIBRARY_PATH=. ./$(TEST_TARGET) -a twofish
	@echo ""
	@echo "Running tests for Camellia..."
	LD_LIBRARY_PATH=. ./$(TEST_TARGET) -a camellia

.PHONY: quick-test
quick-test: $(TARGET) $(LIB_BLOWFISH) $(LIB_TWOFISH) $(LIB_CAMELLIA)
	@echo "=== Quick test for Blowfish ==="
	@echo "Blowfish test message" > test.txt
	@LD_LIBRARY_PATH=. ./$(TARGET) -m generate-key -o key.bin
	@LD_LIBRARY_PATH=. ./$(TARGET) -m encrypt -i test.txt -o test.enc -k key.bin
	@LD_LIBRARY_PATH=. ./$(TARGET) -m decrypt -i test.enc -o test.dec -k key.bin
	@diff test.txt test.dec && echo "$(GREEN)✓ Blowfish works!$(RESET)" || echo "$(RED)✗ Blowfish failed!$(RESET)"
	@echo ""
	@echo "=== Quick test for Twofish ==="
	@echo "Twofish test message" > test2.txt
	@LD_LIBRARY_PATH=. ./$(TARGET) -m generate-key -o key2.bin
	@LD_LIBRARY_PATH=. ./$(TARGET) -a twofish -m encrypt -i test2.txt -o test2.enc -k key2.bin
	@LD_LIBRARY_PATH=. ./$(TARGET) -a twofish -m decrypt -i test2.enc -o test2.dec -k key2.bin
	@diff test2.txt test2.dec && echo "$(GREEN)✓ Twofish works!$(RESET)" || echo "$(RED)✗ Twofish failed!$(RESET)"
	@echo ""
	@echo "=== Quick test for Camellia ==="
	@echo "Camellia test message" > test3.txt
	@LD_LIBRARY_PATH=. ./$(TARGET) -m generate-key -o key3.bin
	@LD_LIBRARY_PATH=. ./$(TARGET) -a camellia -m encrypt -i test3.txt -o test3.enc -k key3.bin
	@LD_LIBRARY_PATH=. ./$(TARGET) -a camellia -m decrypt -i test3.enc -o test3.dec -k key3.bin
	@diff test3.txt test3.dec && echo "$(GREEN)✓ Camellia works!$(RESET)" || echo "$(RED)✗ Camellia failed!$(RESET)"
	@rm -f test2.txt test2.enc test2.dec key2.bin test3.txt test3.enc test3.dec key3.bin

clean:
	@echo "Cleaning..."
	rm -f $(BRIDGE_BLOWFISH_OBJ) $(BLOWFISH_OBJ)
	rm -f $(BRIDGE_TWOFISH_OBJ) $(TWOFISH_OBJ)
	rm -f $(BRIDGE_CAMELLIA_OBJ) $(CAMELLIA_OBJ)
	rm -f $(LIB_BLOWFISH) $(LIB_TWOFISH) $(LIB_CAMELLIA)
	rm -f $(TARGET) $(TEST_TARGET)
	rm -f key.bin test.enc test.dec test.txt
	rm -f key2.bin test2.enc test2.dec test2.txt
	rm -f key3.bin test3.enc test3.dec test3.txt
	@echo "$(GREEN)✓ Clean complete$(RESET)"

help:
	@echo "Available targets:"
	@echo "  make          - Build everything (Blowfish + Twofish + Camellia)"
	@echo "  make clean    - Remove built files"
	@echo "  make test     - Run tests for all algorithms"
	@echo "  make run      - Run cryptum"
	@echo "  make quick-test - Quick encryption test for all algorithms"