#pragma once

#include <stdint.h>
#include <stddef.h> // NULL

// UEFI Spec 2.10 section 2.4
#define IN
#define OUT
#define OPTIONAL
#define CONST const

// EFIAPI defines the calling convention for EFI defined functions
// Taken from gnu-efi at
// https://github.com/vathpela/gnu-efi/blob/master/inc/x86_64/efibind.h
#define EFIAPI __attribute__((ms_abi))  // x86_64 Microsoft calling convention

// Data types: UEFI Spec 2.10 section 2.3
#define FALSE 0
#define TRUE 1
typedef uint8_t  BOOLEAN;  // 0 = False, 1 = True
typedef int64_t  INTN;
typedef uint64_t UINTN;
typedef int8_t   INT8;
typedef uint8_t  UINT8;
typedef int16_t  INT16;
typedef uint16_t UINT16;
typedef int32_t  INT32;
typedef uint32_t UINT32;
typedef int64_t  INT64;
typedef uint64_t UINT64;
typedef char     CHAR8;

// UTF-16 equivalent-ish type, for UCS-2 characters
//   codepoints <= 0xFFFF
typedef uint_least16_t char16_t;
typedef char16_t CHAR16;

typedef void VOID;

typedef struct {
    UINT32 TimeLow;
    UINT16 TimeMid;
    UINT16 TimeHighAndVersion;
    UINT8  ClockSeqHighAndReserved;
    UINT8  ClockSeqLow;
    UINT8  Node[6];
} __attribute__ ((packed)) EFI_GUID;

typedef UINTN EFI_STATUS;
typedef VOID *EFI_HANDLE;
typedef VOID *EFI_EVENT;
typedef UINT64 EFI_LBA;
typedef UINTN EFI_TPL;

typedef UINT64 EFI_PHYSICAL_ADDRESS;
typedef UINT64 EFI_VIRTUAL_ADDRESS;

#define EFI_SUCCESS 0ULL

#define TOP_BIT 0x8000000000000000
#define ENCODE_ERROR(x) (TOP_BIT | (x))
#define EFI_ERROR(x) ((INTN)((UINTN)(x)) < 0)

#define EFI_UNSUPPORTED      ENCODE_ERROR(3)
#define EFI_BUFFER_TOO_SMALL ENCODE_ERROR(5)
#define EFI_DEVICE_ERROR     ENCODE_ERROR(7)
#define EFI_NOT_FOUND        ENCODE_ERROR(14)
#define EFI_CRC_ERROR        ENCODE_ERROR(27)

#define EFI_SYSTEM_TABLE_SIGNATURE 0x5453595320494249
#define EFI_2_100_SYSTEM_TABLE_REVISION ((2<<16) | (100))
#define EFI_2_90_SYSTEM_TABLE_REVISION  ((2<<16) | (90))
#define EFI_2_80_SYSTEM_TABLE_REVISION  ((2<<16) | (80))
#define EFI_2_70_SYSTEM_TABLE_REVISION  ((2<<16) | (70))
#define EFI_2_60_SYSTEM_TABLE_REVISION  ((2<<16) | (60))
#define EFI_2_50_SYSTEM_TABLE_REVISION  ((2<<16) | (50))
#define EFI_2_40_SYSTEM_TABLE_REVISION  ((2<<16) | (40))
#define EFI_2_31_SYSTEM_TABLE_REVISION  ((2<<16) | (31))
#define EFI_2_30_SYSTEM_TABLE_REVISION  ((2<<16) | (30))
#define EFI_2_20_SYSTEM_TABLE_REVISION  ((2<<16) | (20))
#define EFI_2_10_SYSTEM_TABLE_REVISION  ((2<<16) | (10))
#define EFI_2_00_SYSTEM_TABLE_REVISION  ((2<<16) | (00))
#define EFI_1_10_SYSTEM_TABLE_REVISION  ((1<<16) | (10))
#define EFI_1_02_SYSTEM_TABLE_REVISION  ((1<<16) | (02))
#define EFI_SPECIFICATION_VERSION       EFI_SYSTEM_TABLE_REVISION
#define EFI_SYSTEM_TABLE_REVISION       EFI_2_100_SYSTEM_TABLE_REVISION

#define EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID \
 {0x387477c2,0x69c7,0x11d2,\
  0x8e,0x39,{0x00,0xa0,0xc9,0x69,0x72,0x3b}}

typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef struct {
 INT32                              MaxMode;

 INT32                              Mode;
 INT32                              Attribute;
 INT32                              CursorColumn;
 INT32                              CursorRow;
 BOOLEAN                            CursorVisible;
} SIMPLE_TEXT_OUTPUT_MODE;

typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_RESET) (
 IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL       *This,
 IN BOOLEAN                               ExtendedVerification
 );

typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_STRING) (
 IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
 IN CHAR16                             *String
 );

typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_CLEAR_SCREEN) (
 IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL             *This
 );

typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
 EFI_TEXT_RESET                           Reset;
 EFI_TEXT_STRING                          OutputString;
 EFI_HANDLE                               TestString;
 EFI_HANDLE                               QueryMode;
 EFI_HANDLE                               SetMode;
 EFI_HANDLE                               SetAttribute;
 EFI_TEXT_CLEAR_SCREEN                    ClearScreen;
 EFI_HANDLE                               SetCursorPosition;
 EFI_HANDLE                               EnableCursor;
 SIMPLE_TEXT_OUTPUT_MODE                  *Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef struct {
  UINT64      Signature;
  UINT32      Revision;
  UINT32      HeaderSize;
  UINT32      CRC32;
  UINT32      Reserved;
 } EFI_TABLE_HEADER;

 #define EFI_BOOT_SERVICES_SIGNATURE 0x56524553544f4f42
#define EFI_BOOT_SERVICES_REVISION EFI_SPECIFICATION_VERSION

typedef enum {
   AllocateAnyPages,
   AllocateMaxAddress,
   AllocateAddress,
   MaxAllocateType
} EFI_ALLOCATE_TYPE;

//******************************************************
//EFI_MEMORY_TYPE
//******************************************************
// These type values are discussed in Memory Type Usage before ExitBootServices()  and  Memory Type Usage after ExitBootServices().
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

typedef
EFI_STATUS
(EFIAPI *EFI_ALLOCATE_PAGES) (
   IN EFI_ALLOCATE_TYPE                   Type,
   IN EFI_MEMORY_TYPE                     MemoryType,
   IN UINTN                               Pages,
   IN OUT EFI_PHYSICAL_ADDRESS            *Memory
   );

  typedef
EFI_STATUS
(EFIAPI *EFI_FREE_PAGES) (
IN EFI_PHYSICAL_ADDRESS    Memory,
IN UINTN                   Pages
);

typedef struct {
   UINT32                     Type;
   EFI_PHYSICAL_ADDRESS       PhysicalStart;
   EFI_VIRTUAL_ADDRESS        VirtualStart;
   UINT64                     NumberOfPages;
   UINT64                     Attribute;
  } EFI_MEMORY_DESCRIPTOR;

typedef
EFI_STATUS
(EFIAPI *EFI_GET_MEMORY_MAP) (
   IN OUT UINTN                  *MemoryMapSize,
   OUT EFI_MEMORY_DESCRIPTOR     *MemoryMap,
   OUT UINTN                     *MapKey,
   OUT UINTN                     *DescriptorSize,
   OUT UINT32                    *DescriptorVersion
  );

typedef
EFI_STATUS
(EFIAPI  *EFI_ALLOCATE_POOL) (
   IN EFI_MEMORY_TYPE            PoolType,
   IN UINTN                      Size,
   OUT VOID                      **Buffer
   );

  typedef
EFI_STATUS
(EFIAPI *EFI_FREE_POOL) (
   IN VOID           *Buffer
   );

typedef enum {
   AllHandles,
   ByRegisterNotify,
   ByProtocol
  } EFI_LOCATE_SEARCH_TYPE;

typedef
EFI_STATUS
(EFIAPI *EFI_HANDLE_PROTOCOL) (
   IN EFI_HANDLE                    Handle,
   IN EFI_GUID                      *Protocol,
   OUT VOID                         **Interface
   );

  typedef
EFI_STATUS
(EFIAPI *EFI_OPEN_PROTOCOL) (
   IN EFI_HANDLE                    Handle,
   IN EFI_GUID                      *Protocol,
   OUT VOID                         **Interface OPTIONAL,
   IN EFI_HANDLE                    AgentHandle,
   IN EFI_HANDLE                    ControllerHandle,
   IN UINT32                        Attributes
   );

  typedef
EFI_STATUS
(EFIAPI *EFI_CLOSE_PROTOCOL) (
   IN EFI_HANDLE                 Handle,
   IN EFI_GUID                   *Protocol,
   IN EFI_HANDLE                 AgentHandle,
   IN EFI_HANDLE                 ControllerHandle
   );

  typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_PROTOCOL) (
  IN EFI_GUID                            *Protocol,
  IN VOID                                *Registration OPTIONAL,
  OUT VOID                               **Interface
 );


 typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_HANDLE_BUFFER) (
   IN EFI_LOCATE_SEARCH_TYPE                    SearchType,
   IN EFI_GUID                                  *Protocol OPTIONAL,
   IN VOID                                      *SearchKey OPTIONAL,
   OUT UINTN                                    *NoHandles,
   OUT EFI_HANDLE                               **Buffer
   );

  typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_HANDLE) (
   IN EFI_LOCATE_SEARCH_TYPE                 SearchType,
   IN EFI_GUID                               *Protocol OPTIONAL,
   IN VOID                                   *SearchKey OPTIONAL,
   IN OUT UINTN                              *BufferSize,
   OUT EFI_HANDLE                            *Buffer
   );

 typedef  EFI_STATUS
(EFIAPI *EFI_EXIT_BOOT_SERVICES) (
  IN EFI_HANDLE                       ImageHandle,
  IN UINTN                            MapKey
  );


#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL  0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL        0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL       0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER 0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER           0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE           0x00000020

typedef struct {
  EFI_TABLE_HEADER     Hdr;

  //
  // Task Priority Services
  //
    EFI_HANDLE        RaiseTPL;       // EFI 1.0+
    EFI_HANDLE      RestoreTPL;     // EFI 1.0+

    //
    // Memory Services
    //
    EFI_ALLOCATE_PAGES   AllocatePages;  // EFI 1.0+
    EFI_FREE_PAGES       FreePages;      // EFI 1.0+
    EFI_GET_MEMORY_MAP   GetMemoryMap;   // EFI 1.0+
    EFI_ALLOCATE_POOL    AllocatePool;   // EFI 1.0+
    EFI_FREE_POOL        FreePool;       // EFI 1.0+

    //
    // Event & Timer Services
    //
    EFI_HANDLE     CreateEvent;    // EFI 1.0+
    EFI_HANDLE        SetTimer;       // EFI 1.0+
    EFI_HANDLE   WaitForEvent;   // EFI 1.0+
    EFI_HANDLE     SignalEvent;    // EFI 1.0+
    EFI_HANDLE      CloseEvent;     // EFI 1.0+
    EFI_HANDLE      CheckEvent;     // EFI 1.0+

    //
    // Protocol Handler Services
    //
    EFI_HANDLE     InstallProtocolInterface;            // EFI 1.0+
    EFI_HANDLE   ReinstallProtocolInterface;          // EFI 1.0+
    EFI_HANDLE   UninstallProtocolInterface;          // EFI 1.0+
    EFI_HANDLE_PROTOCOL                HandleProtocol;                      // EFI 1.0+
    VOID*   Reserved;    // EFI 1.0+
    EFI_HANDLE       RegisterProtocolNotify;              // EFI  1.0+
    EFI_LOCATE_HANDLE                  LocateHandle;                        // EFI 1.0+
    EFI_HANDLE             LocateDevicePath;                    // EFI 1.0+
    EFI_HANDLE       InstallConfigurationTable;           // EFI 1.0+

    //
    // Image Services
    //
    EFI_HANDLE               LoadImage;        // EFI 1.0+
    EFI_HANDLE                StartImage;       // EFI 1.0+
    EFI_HANDLE                       Exit;             // EFI 1.0+
    EFI_HANDLE               UnloadImage;      // EFI 1.0+
    EFI_EXIT_BOOT_SERVICES         ExitBootServices; // EFI 1.0+

    //
    // Miscellaneous Services
    //
    EFI_HANDLE   GetNextMonotonicCount; // EFI 1.0+
    EFI_HANDLE                      Stall;                 // EFI 1.0+
    EFI_HANDLE         SetWatchdogTimer;      // EFI 1.0+

    //
    // DriverSupport Services
    //
    EFI_HANDLE         ConnectController;     // EFI 1.1
    EFI_HANDLE      DisconnectController;  // EFI 1.1+

    //
    // Open and Close Protocol Services
    //
    EFI_OPEN_PROTOCOL              OpenProtocol;           // EFI 1.1+
    EFI_CLOSE_PROTOCOL             CloseProtocol;          // EFI 1.1+
 EFI_HANDLE     OpenProtocolInformation;// EFI 1.1+

    //
    // Library Services
    //
    EFI_HANDLE       ProtocolsPerHandle;     // EFI 1.1+
    EFI_LOCATE_HANDLE_BUFFER       LocateHandleBuffer;     // EFI 1.1+
    EFI_LOCATE_PROTOCOL            LocateProtocol;         // EFI 1.1+
  EFI_HANDLE  InstallMultipleProtocolInterfaces;    // EFI 1.1+
  EFI_HANDLE UninstallMultipleProtocolInterfaces;   // EFI 1.1+*

    //
    // 32-bit CRC Services
    //
    EFI_HANDLE    CalculateCrc32;     // EFI 1.1+

    //
    // Miscellaneous Services
    //
    EFI_HANDLE           CopyMem;        // EFI 1.1+
    EFI_HANDLE            SetMem;         // EFI 1.1+
    EFI_HANDLE    CreateEventEx;  // UEFI 2.0+
  } EFI_BOOT_SERVICES;

#define SMBIOS_TABLE_GUID \
  {0xeb9d2d31,0x2d88,0x11d3,\
   0x9a,0x16,{0x00,0x90,0x27,0x3f,0xc1,0x4d}}

  typedef struct{
  EFI_GUID           VendorGuid;
  VOID               *VendorTable;
}   EFI_CONFIGURATION_TABLE;


typedef struct {
  EFI_TABLE_HEADER                 Hdr;
  CHAR16                           *FirmwareVendor;
  UINT32                           FirmwareRevision;
  EFI_HANDLE                       ConsoleInHandle;
  EFI_HANDLE                       *ConIn;
  EFI_HANDLE                       ConsoleOutHandle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *ConOut;
  EFI_HANDLE                       StandardErrorHandle;
  EFI_HANDLE                       *StdErr;
  EFI_HANDLE                       *RuntimeServices;
  EFI_BOOT_SERVICES                *BootServices;
  UINTN                            NumberOfTableEntries;
  EFI_CONFIGURATION_TABLE          *ConfigurationTable;
} EFI_SYSTEM_TABLE;

#define EFI_BLOCK_IO_PROTOCOL_GUID \
 {0x964e5b21,0x6459,0x11d2,\
  0x8e,0x39,{0x00,0xa0,0xc9,0x69,0x72,0x3b}}

  #define EFI_BLOCK_IO_PROTOCOL_REVISION2   0x00020001
#define EFI_BLOCK_IO_PROTOCOL_REVISION3   ((2<<16) | (31))

typedef struct _EFI_BLOCK_IO_PROTOCOL EFI_BLOCK_IO_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *EFI_BLOCK_RESET) (
  IN EFI_BLOCK_IO_PROTOCOL    *This,
  IN BOOLEAN                  ExtendedVerification
  );
typedef
EFI_STATUS
(EFIAPI *EFI_BLOCK_READ) (
  IN EFI_BLOCK_IO_PROTOCOL    *This,
  IN UINT32                   MediaId,
  IN EFI_LBA                  LBA,
  IN UINTN                    BufferSize,
  OUT VOID                    *Buffer
  );

  typedef
EFI_STATUS
(EFIAPI *EFI_BLOCK_WRITE) (
  IN EFI_BLOCK_IO_PROTOCOL       *This,
  IN UINT32                      MediaId,
  IN EFI_LBA                     LBA,
  IN UINTN                       BufferSize,
  IN VOID                        *Buffer
  );

typedef
EFI_STATUS
(EFIAPI *EFI_BLOCK_FLUSH) (
  IN EFI_BLOCK_IO_PROTOCOL    *This
  );

typedef struct {
  UINT32                    MediaId;
  BOOLEAN                   RemovableMedia;
  BOOLEAN                   MediaPresent;
  BOOLEAN                   LogicalPartition;
  BOOLEAN                   ReadOnly;
  BOOLEAN                   WriteCaching;
  UINT32                    BlockSize;
  UINT32                    IoAlign;
  EFI_LBA                   LastBlock;

   EFI_LBA                 LowestAlignedLba; //added in Revision 2
   UINT32                  LogicalBlocksPerPhysicalBlock;
//added in Revision 2
UINT32 OptimalTransferLengthGranularity;
// added in Revision 3
} EFI_BLOCK_IO_MEDIA;



typedef struct _EFI_BLOCK_IO_PROTOCOL {
  UINT64                         Revision;
  EFI_BLOCK_IO_MEDIA             *Media;
  EFI_BLOCK_RESET                Reset;
  EFI_BLOCK_READ                 ReadBlocks;
  EFI_BLOCK_WRITE                WriteBlocks;
  EFI_BLOCK_FLUSH                FlushBlocks;
} EFI_BLOCK_IO_PROTOCOL;

#define EFI_LOADED_IMAGE_PROTOCOL_GUID \
  {0x5B1B31A1,0x9562,0x11d2,\
    0x8E,0x3F,{0x00,0xA0,0xC9,0x69,0x72,0x3B}}

    #define EFI_LOADED_IMAGE_PROTOCOL_REVISION 0x1000

  typedef
EFI_STATUS
(EFIAPI *EFI_IMAGE_UNLOAD) (
  IN EFI_HANDLE               ImageHandle
  );

#define EFI_DEVICE_PATH_PROTOCOL_GUID \
  {0x09576e91,0x6d3f,0x11d2,\
    0x8e,0x39,{0x00,0xa0,0xc9,0x69,0x72,0x3b}}

typedef struct _EFI_DEVICE_PATH_PROTOCOL {
  UINT8           Type;
  UINT8           SubType;
  UINT8           Length[2];
 } EFI_DEVICE_PATH_PROTOCOL;

typedef struct {
   UINT32                        Revision;
   EFI_HANDLE                    ParentHandle;
   EFI_SYSTEM_TABLE              *SystemTable;

   // Source location of the image
   EFI_HANDLE                    DeviceHandle;
   EFI_DEVICE_PATH_PROTOCOL      *FilePath;
   VOID                          *Reserved;

   // Image’s load options
   UINT32                        LoadOptionsSize;
   VOID                          *LoadOptions;

   // Location where image was loaded
   VOID                          *ImageBase;
   UINT64                        ImageSize;
   EFI_MEMORY_TYPE               ImageCodeType;
   EFI_MEMORY_TYPE               ImageDataType;
   EFI_IMAGE_UNLOAD              Unload;
} EFI_LOADED_IMAGE_PROTOCOL;

#define EFI_PARTITION_INFO_PROTOCOL_GUID \
  { \
  0x8cf2f62c, 0xbc9b, 0x4821, 0x80, 0x8d, {0xec, 0x9e, \
                  0xc4, 0x21, 0xa1, 0xa0} \
}

#define EFI_PARTITION_INFO_PROTOCOL_REVISION 0x0001000
#define PARTITION_TYPE_OTHER 0x00
#define PARTITION_TYPE_MBR 0x01
#define PARTITION_TYPE_GPT 0x02

#define PARTITION_TYPE_OTHER                 0x00
#define PARTITION_TYPE_MBR                   0x01
#define PARTITION_TYPE_GPT                   0x02

typedef struct {
    UINT8 BootIndicator;
    UINT8 StartHead;
    UINT8 StartSector;
    UINT8 StartTrack;
    UINT8 OSIndicator;
    UINT8 EndHead;
    UINT8 EndSector;
    UINT8 EndTrack;
    UINT8 StartingLBA[4];
    UINT8 SizeInLBA[4];
} __attribute__ ((packed)) MBR_PARTITION_RECORD;


typedef struct {
    UINT8                BootStrapCode[440];
    UINT8                UniqueMbrSignature[4];
    UINT8                Unknown[2];
    MBR_PARTITION_RECORD Partition[4];
    UINT16               Signature;
} __attribute__ ((packed)) MASTER_BOOT_RECORD;


typedef struct {
    EFI_GUID PartitionTypeGUID;
    EFI_GUID UniquePartitionGUID;
    EFI_LBA  StartingLBA;
    EFI_LBA  EndingLBA;
    UINT64   Attributes;
    CHAR16   PartitionName[36];
} __attribute__ ((packed)) EFI_PARTITION_ENTRY;




typedef struct {

  UINT32         Revision;
  UINT32         Type;
  UINT8          System;
  UINT8          Reserved[7];
  union {
   ///
   /// MBR data
   ///
   MBR_PARTITION_RECORD Mbr;

   ///
   /// GPT data
   ///
   EFI_PARTITION_ENTRY Gpt;
  } Info;
}__attribute__((packed)) EFI_PARTITION_INFO_PROTOCOL;

#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
 {0x9042a9de,0x23dc,0x4a38,\
  0x96,0xfb,{0x7a,0xde,0xd0,0x80,0x51,0x6a}}

typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL EFI_GRAPHICS_OUTPUT_PROTOCOL;

typedef struct {
  UINT32              RedMask;
  UINT32              GreenMask;
  UINT32              BlueMask;
  UINT32              ReservedMask;
 } EFI_PIXEL_BITMASK;

typedef enum {
  PixelRedGreenBlueReserved8BitPerColor,
  PixelBlueGreenRedReserved8BitPerColor,
  PixelBitMask,
  PixelBltOnly,
  PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

typedef struct {
 UINT32                    Version;
 UINT32                    HorizontalResolution;
 UINT32                    VerticalResolution;
 EFI_GRAPHICS_PIXEL_FORMAT PixelFormat;
 EFI_PIXEL_BITMASK         PixelInformation;
 UINT32                    PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
  UINT32                                    MaxMode;
  UINT32                                    Mode;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION      *Info;
  UINTN                                      SizeOfInfo;
  EFI_PHYSICAL_ADDRESS                      FrameBufferBase;
  UINTN                                     FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

typedef
EFI_STATUS
(EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE) (
 IN EFI_GRAPHICS_OUTPUT_PROTOCOL              *This,
 IN UINT32                                    ModeNumber,
 OUT UINTN                                    *SizeOfInfo,
 OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION     **Info
 );

typedef
EFI_STATUS
(EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE) (
 IN EFI_GRAPHICS_OUTPUT_PROTOCOL                *This,
 IN UINT32                                      ModeNumber
 );

typedef struct {
 UINT8                        Blue;
 UINT8                        Green;
 UINT8                        Red;
 UINT8                        Reserved;
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL;

typedef enum {
 EfiBltVideoFill,
 EfiBltVideoToBltBuffer,
 EfiBltBufferToVideo,
 EfiBltVideoToVideo,
 EfiGraphicsOutputBltOperationMax
} EFI_GRAPHICS_OUTPUT_BLT_OPERATION;

typedef
EFI_STATUS
(EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT) (
 IN EFI_GRAPHICS_OUTPUT_PROTOCOL                 *This,
 IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL            *BltBuffer, OPTIONAL
 IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION            BltOperation,
 IN UINTN                                        SourceX,
 IN UINTN                                        SourceY,
 IN UINTN                                        DestinationX,
 IN UINTN                                        DestinationY,
 IN UINTN                                        Width,
 IN UINTN                                        Height,
 IN UINTN                                        Delta OPTIONAL
 );

typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL {
 EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE     QueryMode;
 EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE       SetMode;
 EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT            Blt;
 EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE           *Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;


#define EFI_FILE_PROTOCOL_REVISION           0x00010000
#define EFI_FILE_PROTOCOL_REVISION2          0x00020000
#define EFI_FILE_PROTOCOL_LATEST_REVISION EFI_FILE_PROTOCOL_REVISION2

typedef struct _EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_OPEN) (
  IN EFI_FILE_PROTOCOL                  *This,
  OUT EFI_FILE_PROTOCOL                 **NewHandle,
  IN CHAR16                             *FileName,
  IN UINT64                             OpenMode,
  IN UINT64                             Attributes
  );

  typedef
EFI_STATUS
(EFIAPI *EFI_FILE_CLOSE) (
  IN EFI_FILE_PROTOCOL                     *This
  );

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_DELETE) (
  IN EFI_FILE_PROTOCOL                     *This
  );

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_READ) (
  IN EFI_FILE_PROTOCOL           *This,
  IN OUT UINTN                   *BufferSize,
  OUT VOID                       *Buffer
  );

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_WRITE) (
  IN EFI_FILE_PROTOCOL              *This,
  IN OUT UINTN                      *BufferSize,
  IN VOID                           *Buffer
  );

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_FLUSH) (
  IN EFI_FILE_PROTOCOL             *This
  );

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_GET_INFO) (
  IN EFI_FILE_PROTOCOL             *This,
  IN EFI_GUID                      *InformationType,
  IN OUT UINTN                     *BufferSize,
  OUT VOID                         *Buffer
  );

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_SET_INFO) (
  IN EFI_FILE_PROTOCOL                *This,
  IN EFI_GUID                         *InformationType,
  IN UINTN                            BufferSize,
  IN VOID                             *Buffer
  );

  typedef
EFI_STATUS
(EFIAPI *EFI_FILE_SET_POSITION) (
   IN EFI_FILE_PROTOCOL      *This,
   IN UINT64                 Position
   );
  typedef
EFI_STATUS
(EFIAPI *EFI_FILE_GET_POSITION) (
  IN EFI_FILE_PROTOCOL                *This,
  OUT UINT64                          *Position
  );

typedef struct _EFI_FILE_PROTOCOL {
  UINT64                          Revision;
  EFI_FILE_OPEN                   Open;
  EFI_FILE_CLOSE                  Close;
  EFI_FILE_DELETE                 Delete;
  EFI_FILE_READ                   Read;
  EFI_FILE_WRITE                  Write;
  EFI_FILE_GET_POSITION           GetPosition;
  EFI_FILE_SET_POSITION           SetPosition;
  EFI_FILE_GET_INFO               GetInfo;
  EFI_FILE_SET_INFO               SetInfo;
  EFI_FILE_FLUSH                  Flush;
  EFI_HANDLE                OpenEx; // Added for revision 2
  EFI_HANDLE                ReadEx; // Added for revision 2
  EFI_HANDLE               WriteEx; // Added for revision 2
  EFI_HANDLE               FlushEx; // Added for revision 2
} EFI_FILE;



#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
 {0x0964e5b22,0x6459,0x11d2,\
  0x8e,0x39,{0x00,0xa0,0xc9,0x69,0x72,0x3b}}

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION 0x00010000


typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME) (
  IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL                   *This,
  OUT EFI_FILE_PROTOCOL                                **Root
  );

typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
 UINT64                                         Revision;
 EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME    OpenVolume;
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;


#define EFI_FILE_MODE_READ       0x0000000000000001
#define EFI_FILE_MODE_WRITE      0x0000000000000002
#define EFI_FILE_MODE_CREATE     0x8000000000000000


#define EFI_FILE_READ_ONLY       0x0000000000000001
#define EFI_FILE_HIDDEN          0x0000000000000002
#define EFI_FILE_SYSTEM          0x0000000000000004
#define EFI_FILE_RESERVED        0x0000000000000008
#define EFI_FILE_DIRECTORY       0x0000000000000010
#define EFI_FILE_ARCHIVE         0x0000000000000020
#define EFI_FILE_VALID_ATTR      0x0000000000000037