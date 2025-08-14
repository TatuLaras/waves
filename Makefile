CC = gcc
PACKAGES = -lm
INCLUDE = -Iinclude -Iexternal/include
CFLAGS = $(PACKAGES) $(INCLUDE) -Wall -Wextra -Wshadow -pedantic -Wstrict-prototypes -march=native
CFLAGS_DEBUG = $(CFLAGS) -DDEBUG -ggdb
CFLAGS_ASAN = $(CFLAGS_DEBUG) $(SANITIZE)

SRC = $(wildcard src/*.c) $(wildcard external/src/*.c)
BUILD_DIR = build

example: $(BUILD_DIR) $(BUILD_DIR)/example
example_asan: $(BUILD_DIR) $(BUILD_DIR)/example_asan
run: $(BUILD_DIR) $(BUILD_DIR)/example
	@echo "WARNING: no address sanitation enabled, consider running with 'make run_asan' when developing."
	$(BUILD_DIR)/example $(ARGS)
run_asan: $(BUILD_DIR) $(BUILD_DIR)/example_asan
	$(BUILD_DIR)/example_asan $(ARGS)

$(BUILD_DIR)/example: $(SRC)
	$(CC) -o $@ $^ $(CFLAGS_DEBUG)

$(BUILD_DIR)/example_asan: $(SRC)
	$(CC) -o $@ $^ $(CFLAGS_ASAN)

$(BUILD_DIR): 
	@mkdir -p $(BUILD_DIR)
