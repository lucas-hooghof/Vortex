#SATA/ACHI
qemu-system-x86_64 -drive if=pflash,format=raw,unit=0,file=bin/OVMF_CODE-pure-efi.fd,readonly=on \
                    -cpu qemu64,+x2apic -machine q35 \
                   -m 256M \
                   -device rtl8139,netdev=net0 -netdev user,id=net0 \
                   -drive if=none,id=disk0,format=raw,file=build/Vortex.hdd \
                   -device ahci,id=ahci \
                   -device ide-hd,drive=disk0,bus=ahci.0 \
                   -debugcon stdio -d int -no-reboot -no-shutdown
#IDE
#qemu-system-x86_64 \
#  -drive if=pflash,format=raw,unit=0,file=bin/OVMF_CODE-pure-efi.fd,readonly=on \
#  -cpu qemu64,+x2apic -machine q35 \
#  -m 256M \
#  -device rtl8139,netdev=net0 -netdev user,id=net0 \
#  -drive if=none,id=disk0,format=raw,file=build/Vortex.hdd \
#  -device ide-hd,drive=disk0 \
#  -debugcon stdio -d int -no-reboot -no-shutdown

#NVME
#qemu-system-x86_64 \
# -machine q35 \
# -cpu qemu64 \
# -m 256M \
# -drive if=pflash,format=raw,unit=0,file=bin/OVMF_CODE-pure-efi.fd,readonly=on \
# -drive id=nvme0,file=build/Vortex.hdd,format=raw,if=none \
# -device nvme,drive=nvme0,serial=1234