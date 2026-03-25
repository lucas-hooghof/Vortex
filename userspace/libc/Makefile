# ------------------------
# Config
# ------------------------
SRC_DIR := src
OBJ_DIR := $(BUILD_DIR)/obj/vlibc
LIB_OUT := $(BUILD_DIR)/lib/vlibc.a

CC      := x86_64-elf-gcc
AS      := nasm
AR      := x86_64-elf-ar

CFLAGS  := -ffreestanding -O2 -Wall -Wextra -Iheaders -mno-red-zone -fno-stack-protector
ASFLAGS := -f elf64

# ------------------------
# Find source files
# ------------------------
C_SOURCES   := $(shell find $(SRC_DIR) -name "*.c")
ASM_SOURCES := $(shell find $(SRC_DIR) -name "*.asm")

C_OBJECTS   := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(C_SOURCES))
ASM_OBJECTS := $(patsubst $(SRC_DIR)/%.asm,$(OBJ_DIR)/%.o,$(ASM_SOURCES))

OBJECTS := $(C_OBJECTS) $(ASM_OBJECTS)

# ------------------------
# Default target
# ------------------------
all: $(LIB_OUT)

# ------------------------
# Build static library
# ------------------------
$(LIB_OUT): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

# ------------------------
# Compile C
# ------------------------
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ------------------------
# Assemble ASM
# ------------------------
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# ------------------------
# Clean
# ------------------------
clean:
	rm -rf $(OBJ_DIR) $(LIB_OUT)

.PHONY: all clean