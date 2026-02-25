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
    echo "EFI file '$EFI_FILE' does not exist!"
    exit 1
fi


echo "Creating file $FILENAME with size $((BLOCKS * 512)) bytes..."
dd if=/dev/zero of="$FILENAME" bs=512 count="$BLOCKS" status=progress


LOOPDEV=$(losetup --show -f "$FILENAME")
echo "Attached loop device: $LOOPDEV"


parted -s "$LOOPDEV" mklabel gpt
parted -s "$LOOPDEV" mkpart ESP fat32 1MiB "$((ESP_SIZE_MB + 1))MiB"
parted -s "$LOOPDEV" set 1 boot on

# Re-attach with partition scanning now that the partition table exists
losetup -d "$LOOPDEV"
LOOPDEV=$(losetup --show -fP "$FILENAME")
echo "Re-attached loop device with partition scanning: $LOOPDEV"

# Wait for partition devices to appear
partprobe "$LOOPDEV"
sleep 1

# Verify the partition device exists before formatting
if [ ! -b "${LOOPDEV}p1" ]; then
    echo "ERROR: Partition device ${LOOPDEV}p1 did not appear. Aborting."
    losetup -d "$LOOPDEV"
    exit 1
fi

mkfs.fat -F32 "${LOOPDEV}p1"


MNTDIR=$(mktemp -d)
mount "${LOOPDEV}p1" "$MNTDIR"
mkdir -p "$MNTDIR/EFI/BOOT"
cp "$EFI_FILE" "$MNTDIR/EFI/BOOT/"
echo "Copied EFI file to $MNTDIR/EFI/BOOT/"
umount "$MNTDIR"
rmdir "$MNTDIR"


parted -s "$LOOPDEV" mkpart primary "$((ESP_SIZE_MB + 1))MiB" "$((ESP_SIZE_MB + OTHER_SIZE_MB + 1))MiB"
partprobe "$LOOPDEV"
sleep 1

sgdisk --typecode=2:"$ROOT_GUID" "$LOOPDEV"

echo "Partitioning complete. Summary:"
parted "$LOOPDEV" print


losetup -d "$LOOPDEV"
echo "Done!"