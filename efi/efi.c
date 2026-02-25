#include "efi.h"

EFI_STATUS efi_main(EFI_HANDLE ImageHandle,EFI_SYSTEM_TABLE* Systemtable)
{
    Systemtable->ConOut->OutputString(Systemtable->ConOut,L"Hello World in UEFI");
    while(1) {}

    return EFI_SUCCESS;
}