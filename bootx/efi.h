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


#define TOP_BIT 0x8000000000000000
#define ENCODE_ERROR(x) (TOP_BIT | (x))
#define EFI_ERROR(x) ((INTN)((UINTN)(x)) < 0)


#define EFI_SUCCESS               0
#define EFI_LOAD_ERROR            ENCODE_ERROR(1)
#define EFI_INVALID_PARAMETER     ENCODE_ERROR(2)
#define EFI_UNSUPPORTED           ENCODE_ERROR(3)
#define EFI_BAD_BUFFER_SIZE       ENCODE_ERROR(4)
#define EFI_BUFFER_TOO_SMALL      ENCODE_ERROR(5)
#define EFI_NOT_READY             ENCODE_ERROR(6)
#define EFI_DEVICE_ERROR          ENCODE_ERROR(7)
#define EFI_WRITE_PROTECTED       ENCODE_ERROR(8)
#define EFI_OUT_OF_RESOURCES      ENCODE_ERROR(9)
#define EFI_VOLUME_CORRUPTED      ENCODE_ERROR(10)
#define EFI_VOLUME_FULL           ENCODE_ERROR(11)
#define EFI_NO_MEDIA              ENCODE_ERROR(12)
#define EFI_MEDIA_CHANGED         ENCODE_ERROR(13)
#define EFI_NOT_FOUND             ENCODE_ERROR(14)
#define EFI_ACCESS_DENIED         ENCODE_ERROR(15)
#define EFI_NO_RESPONSE           ENCODE_ERROR(16)
#define EFI_NO_MAPPING            ENCODE_ERROR(17)
#define EFI_TIMEOUT               ENCODE_ERROR(18)
#define EFI_NOT_STARTED           ENCODE_ERROR(19)
#define EFI_ALREADY_STARTED       ENCODE_ERROR(20)
#define EFI_ABORTED               ENCODE_ERROR(21)
#define EFI_ICMP_ERROR            ENCODE_ERROR(22)
#define EFI_TFTP_ERROR            ENCODE_ERROR(23)
#define EFI_PROTOCOL_ERROR        ENCODE_ERROR(24)
#define EFI_INCOMPATIBLE_VERSION  ENCODE_ERROR(25)
#define EFI_SECURITY_VIOLATION    ENCODE_ERROR(26)
#define EFI_CRC_ERROR             ENCODE_ERROR(27)
#define EFI_END_OF_MEDIA          ENCODE_ERROR(28)
#define EFI_END_OF_FILE           ENCODE_ERROR(31)
#define EFI_INVALID_LANGUAGE      ENCODE_ERROR(32)
#define EFI_COMPROMISED_DATA      ENCODE_ERROR(33)
#define EFI_IP_ADDRESS_CONFLICT   ENCODE_ERROR(34)
#define EFI_HTTP_ERROR            ENCODE_ERROR(35)


typedef struct {
  UINT64      Signature;
  UINT32      Revision;
  UINT32      HeaderSize;
  UINT32      CRC32;
  UINT32      Reserved;
 } EFI_TABLE_HEADER;

typedef struct{
  EFI_GUID           VendorGuid;
  VOID               *VendorTable;
}   EFI_CONFIGURATION_TABLE;


#define SAL_SYSTEM_TABLE_GUID \
  {0xeb9d2d32,0x2d88,0x11d3,\
   0x9a,0x16,{0x00,0x90,0x27,0x3f,0xc1,0x4d}}

#define SMBIOS_TABLE_GUID \
  {0xeb9d2d31,0x2d88,0x11d3,\
   0x9a,0x16,{0x00,0x90,0x27,0x3f,0xc1,0x4d}}

#define SMBIOS3_TABLE_GUID \
  {0xf2fd1544, 0x9794, 0x4a2c,\
   0x99,0x2e,{0xe5,0xbb,0xcf,0x20,0xe3,0x94}}

#define MPS_TABLE_GUID \
  {0xeb9d2d2f,0x2d88,0x11d3,\
   0x9a,0x16,{0x00,0x90,0x27,0x3f,0xc1,0x4d}}
//
// ACPI 2.0 or newer tables should use EFI_ACPI_TABLE_GUID
//
#define EFI_ACPI_TABLE_GUID \
{0x8868e871,0xe4f1,0x11d3,\
0xbc,0x22,{0x00,0x80,0xc7,0x3c,0x88,0x81}}

#define EFI_ACPI_20_TABLE_GUID EFI_ACPI_TABLE_GUID

#define ACPI_TABLE_GUID \
{0xeb9d2d30,0x2d88,0x11d3,\
0x9a,0x16,{0x00,0x90,0x27,0x3f,0xc1,0x4d}}

#define ACPI_10_TABLE_GUID ACPI_TABLE_GUID*

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
  {0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}}

typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef struct {
 INT32                              MaxMode;
 // current settings
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
(EFIAPI *EFI_TEXT_TEST_STRING) (
 IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL       *This,
 IN CHAR16                                *String
 );

 typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_QUERY_MODE) (
 IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL          *This,
 IN UINTN                                    ModeNumber,
 OUT UINTN                                   *Columns,
 OUT UINTN                                   *Rows
 );

 typedef
EFI_STATUS
(* EFIAPI EFI_TEXT_SET_MODE) (
 IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL          *This,
 IN UINTN                                    ModeNumber
 );

 typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_SET_ATTRIBUTE) (
 IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL        *This,
 IN UINTN                                  Attribute
 );

 typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_CLEAR_SCREEN) (
 IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL             *This
 );

typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_SET_CURSOR_POSITION) (
 IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL             *This,
 IN UINTN                                       Column,
 IN UINTN                                       Row
 );

typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_ENABLE_CURSOR) (
 IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL             *This,
 IN BOOLEAN                                     Visible
 );

typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
 EFI_TEXT_RESET                           Reset;
 EFI_TEXT_STRING                          OutputString;
 EFI_TEXT_TEST_STRING                     TestString;
 EFI_TEXT_QUERY_MODE                      QueryMode;
 EFI_TEXT_SET_MODE                        SetMode;
 EFI_TEXT_SET_ATTRIBUTE                   SetAttribute;
 EFI_TEXT_CLEAR_SCREEN                    ClearScreen;
 EFI_TEXT_SET_CURSOR_POSITION             SetCursorPosition;
 EFI_TEXT_ENABLE_CURSOR                   EnableCursor;
 SIMPLE_TEXT_OUTPUT_MODE                  *Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

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
(EFIAPI *EFI_FREE_POOL) (
   IN VOID           *Buffer
   );

typedef
EFI_STATUS
(EFIAPI  *EFI_ALLOCATE_POOL) (
   IN EFI_MEMORY_TYPE            PoolType,
   IN UINTN                      Size,
   OUT VOID                      **Buffer
   );

#define EFI_BOOT_SERVICES_SIGNATURE 0x56524553544f4f42
#define EFI_BOOT_SERVICES_REVISION EFI_SPECIFICATION_VERSION

typedef struct {
   UINT32                     Type;
   EFI_PHYSICAL_ADDRESS       PhysicalStart;
   EFI_VIRTUAL_ADDRESS        VirtualStart;
   UINT64                     NumberOfPages;
   UINT64                     Attribute;
  } EFI_MEMORY_DESCRIPTOR;

typedef enum {
   AllocateAnyPages,
   AllocateMaxAddress,
   AllocateAddress,
   MaxAllocateType
} EFI_ALLOCATE_TYPE;

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

typedef
EFI_STATUS
(EFIAPI *EFI_GET_MEMORY_MAP) (
   IN OUT UINTN                  *MemoryMapSize,
   OUT EFI_MEMORY_DESCRIPTOR     *MemoryMap,
   OUT UINTN                     *MapKey,
   OUT UINTN                     *DescriptorSize,
   OUT UINT32                    *DescriptorVersion
  );

#define EFI_DEVICE_PATH_PROTOCOL_GUID \
  {0x09576e91,0x6d3f,0x11d2,\
    {0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}}

typedef struct _EFI_DEVICE_PATH_PROTOCOL {
  UINT8           Type;
  UINT8           SubType;
  UINT8           Length[2];
 } EFI_DEVICE_PATH_PROTOCOL;

#define MEDIA_DEVICE_PATH 0x04
#define MEDIA_FILEPATH_DP 0x04
#define END_DEVICE_PATH_TYPE 0x7F
#define END_ENTIRE_DEVICE_PATH_SUBTYPE 0xFF

typedef struct {
    EFI_DEVICE_PATH_PROTOCOL Header;
    CHAR16 PathName[];
} FILEPATH_DEVICE_PATH;\

typedef
EFI_STATUS
(EFIAPI *EFI_IMAGE_LOAD) (
   IN BOOLEAN                          BootPolicy,
   IN EFI_HANDLE                       ParentImageHandle,
   IN EFI_DEVICE_PATH_PROTOCOL         *DevicePath   OPTIONAL,
   IN VOID                             *SourceBuffer OPTIONAL,
   IN UINTN                            SourceSize,
   OUT EFI_HANDLE                      *ImageHandle
   );
   
typedef
EFI_STATUS
(EFIAPI *EFI_IMAGE_START) (
   IN EFI_HANDLE                             ImageHandle,
   OUT UINTN                                 *ExitDataSize,
   OUT CHAR16                                **ExitData OPTIONAL
   );

 typedef EFI_STATUS
(EFIAPI *EFI_EXIT_BOOT_SERVICES) (
  IN EFI_HANDLE                       ImageHandle,
  IN UINTN                            MapKey
  );
typedef enum {
   AllHandles,
   ByRegisterNotify,
   ByProtocol
  } EFI_LOCATE_SEARCH_TYPE;
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
(EFIAPI *EFI_HANDLE_PROTOCOL) (
   IN EFI_HANDLE                    Handle,
   IN EFI_GUID                      *Protocol,
   OUT VOID                         **Interface
   );
#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL   0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL         0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL        0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER  0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER            0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE            0x00000020
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
    EFI_HANDLE                  LocateHandle;                        // EFI 1.0+
    EFI_HANDLE             LocateDevicePath;                    // EFI 1.0+
    EFI_HANDLE       InstallConfigurationTable;           // EFI 1.0+

    //
    // Image Services
    //
    EFI_IMAGE_LOAD               LoadImage;        // EFI 1.0+
    EFI_IMAGE_START                StartImage;       // EFI 1.0+
    EFI_HANDLE                     Exit;             // EFI 1.0+
    EFI_HANDLE                     UnloadImage;      // EFI 1.0+
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


typedef struct {
  EFI_TABLE_HEADER                 Hdr;
  CHAR16                           *FirmwareVendor;
  UINT32                           FirmwareRevision;
  EFI_HANDLE                       ConsoleInHandle;
  EFI_HANDLE                        *ConIn;
  EFI_HANDLE                       ConsoleOutHandle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *ConOut;
  EFI_HANDLE                       StandardErrorHandle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *StdErr;
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

  EFI_LBA                 LowestAlignedLba;
  UINT32                  LogicalBlocksPerPhysicalBlock;

  UINT32 OptimalTransferLengthGranularity;
} EFI_BLOCK_IO_MEDIA;

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


typedef struct _EFI_BLOCK_IO_PROTOCOL {
  UINT64                         Revision;
  EFI_BLOCK_IO_MEDIA             *Media;
  EFI_BLOCK_RESET                Reset;
  EFI_BLOCK_READ                 ReadBlocks;
  EFI_BLOCK_WRITE                WriteBlocks;
  EFI_BLOCK_FLUSH                FlushBlocks;
} EFI_BLOCK_IO_PROTOCOL;

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

#define EFI_LOADED_IMAGE_PROTOCOL_GUID \
  {0x5B1B31A1,0x9562,0x11d2,\
    0x8E,0x3F,{0x00,0xA0,0xC9,0x69,0x72,0x3B}}

#define EFI_LOADED_IMAGE_PROTOCOL_REVISION 0x1000

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
   EFI_HANDLE              Unload;
} EFI_LOADED_IMAGE_PROTOCOL;

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
 {0x0964e5b22,0x6459,0x11d2,\
  0x8e,0x39,{0x00,0xa0,0xc9,0x69,0x72,0x3b}}


  #define EFI_FILE_PROTOCOL_REVISION           0x00010000
#define EFI_FILE_PROTOCOL_REVISION2          0x00020000
#define EFI_FILE_PROTOCOL_LATEST_REVISION EFI_FILE_PROTOCOL_REVISION2

typedef struct _EFI_FILE EFI_FILE;

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_OPEN) (
  IN EFI_FILE                  *This,
  OUT EFI_FILE                 **NewHandle,
  IN CHAR16                             *FileName,
  IN UINT64                             OpenMode,
  IN UINT64                             Attributes
  );


  typedef
EFI_STATUS
(EFIAPI *EFI_FILE_READ) (
  IN EFI_FILE           *This,
  IN OUT UINTN                   *BufferSize,
  OUT VOID                       *Buffer
  );

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_SET_POSITION) (
   IN EFI_FILE      *This,
   IN UINT64                 Position
   );



typedef struct _EFI_FILE {
  UINT64                          Revision;
  EFI_FILE_OPEN                   Open;
  EFI_HANDLE                  Close;
  EFI_HANDLE                 Delete;
  EFI_FILE_READ                   Read;
  EFI_HANDLE                  Write;
  EFI_HANDLE           GetPosition;
  EFI_FILE_SET_POSITION           SetPosition;
  EFI_HANDLE               GetInfo;
  EFI_HANDLE               SetInfo;
  EFI_HANDLE                  Flush;
  EFI_HANDLE                OpenEx; // Added for revision 2
  EFI_HANDLE                ReadEx; // Added for revision 2
  EFI_HANDLE               WriteEx; // Added for revision 2
  EFI_HANDLE               FlushEx; // Added for revision 2
} EFI_FILE;

  #define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION 0x00010000


typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME) (
  IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL                   *This,
  OUT EFI_FILE                                **Root
  );


typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
 UINT64                                         Revision;
 EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME    OpenVolume;
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

#define EFI_FILE_MODE_READ       0x0000000000000001
#define EFI_FILE_MODE_WRITE      0x0000000000000002
#define EFI_FILE_MODE_CREATE     0x8000000000000000

//******************************************************
// File Attributes
//******************************************************
#define EFI_FILE_READ_ONLY       0x0000000000000001
#define EFI_FILE_HIDDEN          0x0000000000000002
#define EFI_FILE_SYSTEM          0x0000000000000004
#define EFI_FILE_RESERVED        0x0000000000000008
#define EFI_FILE_DIRECTORY       0x0000000000000010
#define EFI_FILE_ARCHIVE         0x0000000000000020
#define EFI_FILE_VALID_ATTR      0x0000000000000037
