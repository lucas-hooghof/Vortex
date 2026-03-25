#pragma once

#include <generic/stdint.h>
#include <generic/stdio.h>

#include <fs/VFS.h>
#include <fs/devices/BlockDevices/SataDevice.h>

namespace fs
{
    #define VXFS_HEADER "VXFS"
    #define BLOCK_SIZE 512

    #define GROUP_SIZE_SECTORS   4096   // 2 MiB
    #define INODE_RATIO_SECTORS  32     // 16 KiB per inode
    #define EXTENTS_PER_INODE    4

    #define BIT(x) (1 << x)

    typedef struct 
    {
        char Header[4];
        uint64_t BytesPerSector;
        uint64_t TotalSectors;

        uint32_t InodeSize;
        uint32_t InodeTableSize;
        uint8_t InodeTables;


        uint32_t ExtentSize;
        uint32_t ExtentTableSize;
        uint32_t ExtentTables;

        uint16_t NextExtentID;
        uint16_t NextInodeID;
        uint16_t NextExtentTableID;
        uint16_t NextInodeTableID;


        uint64_t RootInodeID;

        uint64_t InodeTablesStart;
        uint64_t ExtentTableStart;
        uint64_t DataBitmapStart;
        uint64_t DataRegionStart;

        uint16_t InodesPerTable;
        uint16_t ExtentsPerTable;
        uint64_t DataBitmapSize;

        char Label[16];

        uint16_t unused;

        uint8_t padding[BLOCK_SIZE-119];

    }__attribute__((packed))VXFS_SUPERBLOCK;

    typedef struct
    {
        uint64_t StartSector;
        uint64_t SizeInSectors;

        uint16_t NextExtentID;
        uint16_t NextExtentTableID;

        uint16_t ExtentID;
        uint16_t ExtentTableID;
        uint8_t Availible;
        uint8_t NextExtentUsed;

        uint8_t padding[6];
    } __attribute__((packed)) VXFS_EXTENT;

    typedef struct 
    {
        uint16_t InodeID;
        uint16_t InodeTableID;
        uint16_t Flags;
        uint16_t Permissions;
        uint16_t ExtentID;
        uint16_t ExtentTableID;

        uint8_t free;
        uint64_t SizeInBytes;
        uint16_t NextFreeByte;
    }__attribute((packed))VXFS_INODE;

    typedef struct
    {
        uint16_t InodeID;
        uint16_t InodeTableID;
        uint8_t valid;
        uint8_t paddign;
        uint16_t NameLenght;
        //Follows Name length but not in struct because it can vary
    }__attribute((packed))VXFS_DIRENTRY;

    typedef enum
    {
        DIR = BIT(0),
        SYSTEM = BIT(1),
        SYSLINK = BIT(2),
        HARDLINK = BIT(3)
    }VXFS_FLAGS;
    class VXFS
    {
    public:
        VXFS(const char* drive);

        bool readfile(const char* file,void* buffer,size_t* osize);
    private:
        fid_t sd;
        fs::SataDevice* satadevice;

        VXFS_SUPERBLOCK* superblock;
        VXFS_INODE* rootinode;
        VXFS_EXTENT* rootextent;

        void FindDirectory(VXFS_INODE** node,VXFS_EXTENT** extent,char* name);

        const char* NextEntry(const char* path, char* out)
        {
            // Skip leading '/'
            while (*path == '/')
                path++;

            if (*path == 0)
                return nullptr;

            // Copy until next '/'
            while (*path && *path != '/')
            {
                *out++ = *path++;
            }

            *out = 0;

            return path;
        }
    };   
}
