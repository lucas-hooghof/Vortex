#!/bin/bash

# --- CONFIGURATION ---
FILENAME="$1"
EFI_FILE="$2"       # File to put in EFI/BOOT
BLOCKS=1000000        # Number of 512-byte blocks
ESP_SIZE_MB=100     # Size of EFI System Partition
ROOT_GUID="a5e8bf06-1238-11f1-b74b-00155d0f3cbb" # Replace with your GUID
OTHER_SIZE_MB=200   # Size of your other partition

# --- VALIDATION ---
if [ -z "$FILENAME" ] || [ -z "$EFI_FILE" ]; then
    echo "Usage: $0 <disk-file> <efi-file>"
    exit 1
fi

if [ ! -f "$EFI_FILE" ]; then
    ls -la
    echo "EFI file '$EFI_FILE' does not exist!"
    exit 1
fi

dd if=/dev/zero of="$FILENAME" bs=512 count="$BLOCKS" status=progress

# --- Setup loop device ---
LOOPDEV=$(sudo losetup --show -f "$FILENAME")

# --- Partition the disk ---
sudo parted -s "$LOOPDEV" mklabel gpt
sudo parted -s "$LOOPDEV" mkpart ESP fat32 1MiB "$((ESP_SIZE_MB + 1))MiB"
sudo parted -s "$LOOPDEV" set 1 boot on

# Re-attach with partition scanning now that the partition table exists
sudo losetup -d "$LOOPDEV"
LOOPDEV=$(sudo losetup --show -fP "$FILENAME")

# Wait for partition devices to appear
sudo partprobe "$LOOPDEV"
sleep 1

# Verify the partition device exists before formatting
if [ ! -b "${LOOPDEV}p1" ]; then
    echo "ERROR: Partition device ${LOOPDEV}p1 did not appear. Aborting."
    sudo losetup -d "$LOOPDEV"
    exit 1
fi

# --- Format the ESP ---
sudo mkfs.fat -F32 "${LOOPDEV}p1"

# --- Mount and copy EFI file ---
MNTDIR=$(mktemp -d)
sudo mount "${LOOPDEV}p1" "$MNTDIR"
sudo mkdir -p "$MNTDIR/EFI/BOOT"
sudo cp "$EFI_FILE" "$MNTDIR/EFI/BOOT/"
echo "Copied EFI file to $MNTDIR/EFI/BOOT/"
sudo umount "$MNTDIR"
rmdir "$MNTDIR"

# --- Create the other partition ---
sudo parted -s "$LOOPDEV" mkpart primary "$((ESP_SIZE_MB + 1))MiB" "$((ESP_SIZE_MB + OTHER_SIZE_MB + 1))MiB"
sudo partprobe "$LOOPDEV"
sleep 1

# --- Set custom GUID on the second partition ---
sudo sgdisk --typecode=2:"$ROOT_GUID" "$LOOPDEV" > /dev/null 

# --- Clean up ---
sudo losetup -d "$LOOPDEV"