BUILD_DIR=$(abspath build/)
ROOT_DIR=$(abspath root/)

.PHONY: always image efi run kernel clean

run: image
	./scripts/run.sh

image: efi kernel
	./scripts/GIEC.sh $(BUILD_DIR)/Vortex.hdd $(BUILD_DIR)/BOOTX64.efi ce2adfbe-1c54-11f1-a2ed-00155d8074c4 $(BUILD_DIR)/kernel $(ROOT_DIR)

efi: always
	$(MAKE) -C bootx/ BUILD_DIR=$(BUILD_DIR)

kernel: always
	$(MAKE) -C kernel/ BUILD_DIR=$(BUILD_DIR) ROOT_DIR=$(ROOT_DIR)

always: $(BUILD_DIR) $(ROOT_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(ROOT_DIR):
	mkdir -p $(ROOT_DIR)
	mkdir -p $(ROOT_DIR)/system/dat
	cp bin/bootfont.psf $(ROOT_DIR)/system/dat/bootfont

clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR)
	rm -rf $(ROOT_DIR)
	$(MAKE) -C bootx/ clean || true
	$(MAKE) -C kernel/ clean || true