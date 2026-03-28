# ------------------------
# Config
# ------------------------
BUILD_DIR ?= build

OBJ_DIR := $(BUILD_DIR)/obj/de
OUT     := $(BUILD_DIR)/de/DE

LIBC    := $(BUILD_DIR)/lib/vlibc.a

CC      := x86_64-elf-gcc
CXX     := x86_64-elf-g++
AS      := nasm
LD      := x86_64-elf-ld

CFLAGS  := -ffreestanding -O2 -Wall -Wextra -mno-red-zone -fno-stack-protector -nostdlib -I ../libc/headers
CXXFLAGS:= $(CFLAGS) -fno-exceptions -fno-rtti -std=c++20
ASFLAGS := -f elf64 -g

LDFLAGS := -nostdlib -static -T linker.ld -g

# ------------------------
# Source files (flat dir)
# ------------------------
C_SRCS   := $(wildcard *.c)
CPP_SRCS := $(wildcard *.cpp)
ASM_SRCS := $(wildcard *.asm)

C_OBJS   := $(patsubst %.c,$(OBJ_DIR)/%.o,$(C_SRCS))
CPP_OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(CPP_SRCS))
ASM_OBJS := $(patsubst %.asm,$(OBJ_DIR)/%.o,$(ASM_SRCS))

OBJS := $(C_OBJS) $(CPP_OBJS) $(ASM_OBJS)

# ------------------------
# Default
# ------------------------
all: $(OUT)

# ------------------------
# Link program
# ------------------------
$(OUT): $(OBJS) $(LIBC)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBC)


# ------------------------
# Compile C
# ------------------------
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ------------------------
# Compile C++
# ------------------------
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ------------------------
# Assemble
# ------------------------
$(OBJ_DIR)/%.o: %.asm
	@mkdir -p $(OBJ_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# ------------------------
# Clean
# ------------------------
clean:
	rm -rf $(OBJ_DIR) $(OUT)

.PHONY: all clean