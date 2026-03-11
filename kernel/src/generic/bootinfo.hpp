#pragma once

#include <generic/stdint.hpp>


typedef enum {
   EfiReservedMemoryType,
   EfiLoaderCode,
   EfiLoaderData,
   EfiBootServicesCode,
   EfiBootServicesData,
   EfiRuntimeServicesCode,
   EfiRuntimeServicesData,
   EfiConventionalMemory,
   EfiUnusableMemory,
   EfiACPIReclaimMemory,
   EfiACPIMemoryNVS,
   EfiMemoryMappedIO,
   EfiMemoryMappedIOPortSpace,
   EfiPalCode,
   EfiPersistentMemory,
   EfiUnacceptedMemoryType,
   EfiMaxMemoryType
} EFI_MEMORY_TYPE;

typedef struct {
   EFI_MEMORY_TYPE                     Type;
   uintptr_t                    PhysicalStart;
   uintptr_t                    VirtualStart;
   uint64_t                     NumberOfPages;
   uint64_t                     Attribute;
} EFI_MEMORY_DESCRIPTOR;


typedef struct {
    void* BaseAddress;       // Framebuffer base pointer
    uint64_t BufferSize;       // Total size in bytes
    uint32_t Width;            // Width in pixels
    uint32_t Height;           // Height in pixels
    uint32_t PixelsPerScanLine;// Stride (pixels per row)
} FRAMEBUFFER;

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

typedef struct {
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charsize;
} PSF1_HEADER;

typedef struct {
	PSF1_HEADER* psf1_Header;
	void* glyphBuffer;
} PSF1_FONT;

typedef struct 
{
    FRAMEBUFFER* framebuffer;
    PSF1_FONT* font;
    EFI_MEMORY_DESCRIPTOR* mMap;
    uint64_t MapSize;
    uint64_t DescriptorSize;
}bootinfo_t;