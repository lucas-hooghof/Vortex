

SRC_DIR := src
OBJ_DIR := $(BUILD_DIR)/obj
KERNEL_OUT := $(BUILD_DIR)/kernel

# Compiler/Tools
CC := x86_64-elf-gcc
CXX := x86_64-elf-g++
LD := x86_64-elf-ld
NASM := nasm
ASFLAGS := -f elf64
CFLAGS := -ffreestanding -Wall -Wextra -Isrc -Werror -Wpedantic -g -mno-red-zone -fno-stack-protector -mcmodel=kernel
CXXFLAGS := $(CFLAGS) -fno-exceptions -fno-rtti -mno-red-zone -mgeneral-regs-only -std=c++20
ifeq ($(log),1)
CFLAGS += -DLOG
CXXFLAGS += -DLOG
endif
LDFLAGS := -T linker.ld -nostdlib -static -Bsymbolic

# Find all source files
C_SRCS := $(shell find $(SRC_DIR) -name '*.c')
CPP_SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
ASM_SRCS := $(shell find $(SRC_DIR) -name '*.s' -o -name '*.asm')

# Object files mirror the source structure in OBJ_DIR
C_OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(C_SRCS))
CPP_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CPP_SRCS))
ASM_OBJS := $(patsubst $(SRC_DIR)/%.s,$(OBJ_DIR)/%.o,$(ASM_SRCS))
ASM_OBJS := $(patsubst $(SRC_DIR)/%.asm,$(OBJ_DIR)/%.o,$(ASM_OBJS))

OBJS := $(C_OBJS) $(CPP_OBJS) $(ASM_OBJS)

# Default target
all: $(KERNEL_OUT)

# Link kernel
$(KERNEL_OUT): $(OBJS) linker.ld
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@mkdir -p $(ROOT_DIR)/system 
	mv $(KERNEL_OUT) $(ROOT_DIR)/system/kernel

# Compile C files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Assemble assembly files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(dir $@)
	$(NASM) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(NASM) $(ASFLAGS) $< -o $@

# Clean build
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean