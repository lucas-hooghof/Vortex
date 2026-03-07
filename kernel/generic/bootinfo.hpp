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
   uint32_t                     Type;
   uintptr_t                    PhysicalStart;
   uintptr_t                    VirtualStart;
   uint64_t                     NumberOfPages;
   uint64_t                     Attribute;
  } EFI_MEMORY_DESCRIPTOR;

typedef struct {
	void* BaseAddress;
	size_t BufferSize;
	unsigned int Width;
	unsigned int Height;
	unsigned int PixelsPerScanLine;
} Framebuffer;

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
    EFI_MEMORY_DESCRIPTOR* mMap;
    uint64_t MapSize;
    uint64_t DescriptorSize;
    Framebuffer framebuffer;
    PSF1_FONT* bootfont;
}bootinfo_t;

size_t GetMemorySize(EFI_MEMORY_DESCRIPTOR* mMap,size_t MapSize,size_t MapDescSize);