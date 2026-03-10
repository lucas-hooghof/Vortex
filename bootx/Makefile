CC=x86_64-w64-mingw32-gcc


EFI_LDFLAGS ::= \
	-nostdlib \
	-Wl,--subsystem,10 \
	-e efi_main 

CFLAGS ::= \
	-std=c17 \
	-Wall \
	-Wextra \
	-Wpedantic \
	-mno-red-zone \
	-ffreestanding \
	-fno-stack-protector

EFISRC  ::= efi.c
EFIOBJ  ::= $(EFISRC:%.c=$(BUILD_DIR)/bootloader/efi.o)

EFI_APP ::= $(BUILD_DIR)/BOOTX64.efi

.PHONY: always

$(EFI_APP): $(EFIOBJ) 
	$(CC) $(EFI_LDFLAGS) -o $@ $<

$(EFIOBJ): $(EFISRC) always
	$(CC) $(CFLAGS) -c -o $@ $<

always: $(BUILD_DIR)/bootloader

$(BUILD_DIR)/bootloader:
	mkdir $(BUILD_DIR)/bootloader