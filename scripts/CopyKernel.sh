FILENAME="$1"
KERNEL="$2"

echo ${FILENAME}
echo ${KERNEL}

LOOPDEV=$(sudo losetup --show -fP "$FILENAME")

sudo mkVXFS ${LOOPDEV}p2

sudo mkdirVXFS ${LOOPDEV}p2 system/
sudo cpVXFS ${LOOPDEV}p2 ${KERNEL} system/kernel

sudo losetup -d "${LOOPDEV}"