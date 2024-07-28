# Makefile for compiling replay_libretro.c within RePlay-Core directory

# Compiler
CC := gcc

# Flags for compiling and linking
CFLAGS := -fPIC -Wall -Wextra -O2 `sdl2-config --cflags`
LDFLAGS := -shared `sdl2-config --libs` -lSDL2_gfx

# Source and build directories
SRC_DIR := .
BUILD_DIR := .

# Source file
SRC := $(SRC_DIR)/screentest_libretro.c

# Output file
OUT := $(BUILD_DIR)/screentest_libretro.so

# Default target
all: $(OUT)

# Target for building the output
$(OUT): $(SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

# Target for cleaning the build directory
clean:
	rm -rf $(BUILD_DIR)/*.so

.PHONY: all clean
