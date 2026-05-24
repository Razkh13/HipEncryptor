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
TESTS_DIR = $(SRC_DIR)/tests

MAIN_SRC = $(APP_DIR)/main.cpp
BRIDGE_SRC = $(BRIDGE_DIR)/crypto_api.cpp
BLOWFISH_SRC = $(BLOWFISH_DIR)/blowfish.cpp
TEST_SRC = $(TESTS_DIR)/test_crypto.cpp

BLOWFISH_HPP = $(BLOWFISH_DIR)/blowfish.h

BLOWFISH_OBJ = blowfish.o
BRIDGE_OBJ = crypto_api.o

TARGET = cryptum
LIB_TARGET = libblowfish.so
TEST_TARGET = test_crypto

GREEN = \033[32m
RED = \033[31m
RESET = \033[0m

# Основные цели

.PHONY: all clean help

all: $(LIB_TARGET) $(TARGET) $(TEST_TARGET)
	@echo "$(GREEN)✓ Build complete!$(RESET)"

# Динамическая библиотека
$(LIB_TARGET): $(BRIDGE_OBJ) $(BLOWFISH_OBJ)
	@echo "Building $@..."
	$(CXX) -shared $^ -o $@
	@echo "$(GREEN)✓ $@ created$(RESET)"

$(TARGET): $(MAIN_SRC) $(LIB_TARGET)
	@echo "Building $@..."
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) $< -o $@ $(LDFLAGS)
	@echo "$(GREEN)✓ $@ created$(RESET)"

$(TEST_TARGET): $(TEST_SRC) $(LIB_TARGET)
	@echo "Building $@..."
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) $< -o $@ $(LDFLAGS)
	@echo "$(GREEN)✓ $@ created$(RESET)"

$(BRIDGE_OBJ): $(BRIDGE_SRC) $(BLOWFISH_HPP)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $< -o $@

$(BLOWFISH_OBJ): $(BLOWFISH_SRC) $(BLOWFISH_HPP)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $< -o $@

# Запуск самого Мейкфайла

.PHONY: run
run: $(TARGET) $(LIB_TARGET)
	@echo "Running cryptum..."
	LD_LIBRARY_PATH=. ./$(TARGET)

.PHONY: test
test: $(TEST_TARGET) $(LIB_TARGET)
	@echo "Running tests..."
	LD_LIBRARY_PATH=. ./$(TEST_TARGET) -a blowfish

.PHONY: quick-test
quick-test: $(TARGET) $(LIB_TARGET)
	@echo "=== Quick test ==="
	@echo "Hello Blowfish!" > test.txt
	@LD_LIBRARY_PATH=. ./$(TARGET) -m generate-key -o key.bin
	@LD_LIBRARY_PATH=. ./$(TARGET) -m encrypt -i test.txt -o test.enc -k key.bin
	@LD_LIBRARY_PATH=. ./$(TARGET) -m decrypt -i test.enc -o test.dec -k key.bin
	@diff test.txt test.dec && echo "$(GREEN)✓ Encryption/decryption works!$(RESET)" || echo "$(RED)✗ Test failed!$(RESET)"

clean:
	@echo "Cleaning..."
	rm -f $(BRIDGE_OBJ) $(BLOWFISH_OBJ)
	rm -f $(LIB_TARGET) $(TARGET) $(TEST_TARGET)
	rm -f key.bin test.enc test.dec test.txt
	@echo "$(GREEN)✓ Clean complete$(RESET)"

help:
	@echo "Available targets:"
	@echo "  make          - Build everything"
	@echo "  make clean    - Remove built files"
	@echo "  make test     - Run tests"
	@echo "  make run      - Run cryptum"
	@echo "  make quick-test - Quick encryption test"
