qemu-system-x86_64 -drive if=pflash,format=raw,unit=0,file=./bin/OVMF_CODE.fd,readonly=on \
                   -drive if=pflash,format=raw,unit=1,file=./bin/OVMF_VARS.fd \
                   -machine q35 \
                   -m 256M \
                   -cpu qemu64 \
                   -device rtl8139,netdev=net0 -netdev user,id=net0 \
                   -monitor stdio \
                   -drive file=build/Vortex.hdd,format=raw


                
