#!/bin/bash

set -e

IMAGE="$1"
EFI_FILE="$2"
PART_GUID="$3"
KERNEL="$4"
ROOT_DIR="$5"

if [ -z "$IMAGE" ] || [ -z "$EFI_FILE" ] || [ -z "$PART_GUID" ]; then
    echo "Usage: $0 <imagefile> <efi file> <partition guid>"
    exit 1
fi

IMG_SIZE_MB=128
ESP_SIZE_MB=124

echo "Creating disk image..."
dd if=/dev/zero of="$IMAGE" bs=1M count=$IMG_SIZE_MB

echo "Creating GPT..."
sgdisk -o "$IMAGE"

echo "Creating ESP partition..."
sgdisk -n 1:2048:+${ESP_SIZE_MB}M -t 1:EF00 -c 1:"EFI System" "$IMAGE"

echo "Creating data partition..."
sgdisk -n 2:0:0 -t 2:$PART_GUID -c 2:"Root" "$IMAGE"

echo "Setting up loop device..."
LOOP=$(sudo losetup --show -fP "$IMAGE")

sudo partprobe "$LOOP" 2>/dev/null || true
sleep 1

if [ ! -b "${LOOP}p1" ]; then
    echo "Partition device not found via losetup -P, trying kpartx..."
    sudo kpartx -av "$LOOP"
    LOOP_P1="/dev/mapper/$(basename $LOOP)p1"
    LOOP_P2="/dev/mapper/$(basename $LOOP)p2"
    USE_KPARTX=1
else
    LOOP_P1="${LOOP}p1"
    LOOP_P2="${LOOP}p2"
    USE_KPARTX=0
fi

echo "Formatting ESP as FAT32..."
sudo mkfs.fat -F32 "$LOOP_P1"

echo "Mounting ESP..."
mkdir -p /tmp/esp
sudo mount "$LOOP_P1" /tmp/esp

echo "Copying EFI file..."
sudo mkdir -p /tmp/esp/EFI/BOOT
sudo cp "$EFI_FILE" /tmp/esp/EFI/BOOT/BOOTX64.EFI
sudo cp bin/bootfont.psf /tmp/esp/bootfont.psf

sync

echo "Cleaning up..."
sudo umount /tmp/esp
rmdir /tmp/esp

sudo mkVXFS $LOOP_P2 -L Vortex


if [ -n "$ROOT_DIR" ] && [ -d "$ROOT_DIR" ]; then
    echo "Populating VXFS partition from $ROOT_DIR..."

    # First pass: create all directories
    while IFS= read -r -d '' dir; do
        REL="${dir#$ROOT_DIR}"
        REL="${REL#/}"
        if [ -n "$REL" ]; then
            echo "  mkdir: $REL"
            sudo mkdirVXFS "$LOOP_P2" "$REL"
        fi
    done < <(find "$ROOT_DIR" -mindepth 1 -type d -print0 | sort -z)

    # Second pass: copy all files
    while IFS= read -r -d '' file; do
        REL="${file#$ROOT_DIR}"
        REL="${REL#/}"
        echo "  cp: $REL"
        sudo cpVXFS "$LOOP_P2" "$file" "$REL"
    done < <(find "$ROOT_DIR" -type f -print0)
fi

if [ "$USE_KPARTX" -eq 1 ]; then
    sudo kpartx -dv "$LOOP"
fi

sudo losetup -d "$LOOP"

echo "Done. Image created: $IMAGE"