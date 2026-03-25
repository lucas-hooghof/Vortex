#include <fs/filesystems/VXFS.h>

#include <generic/string.h>
#include <memory/heap.h>

namespace fs
{
    VXFS::VXFS(const char* drive)
    {
        sd = fs::VFS::Open(drive, FD_READ | FD_WRITE);
        if (sd == -1)
            return;

        satadevice = (fs::SataDevice*)fs::VFS::GetInterface(sd);

        // Allocate 1 page
        superblock = (VXFS_SUPERBLOCK*)PageAllocater::GetInstance()->RequestPage();

        // Read superblock
        satadevice->Read(512, superblock);

        if (memcmp(superblock->Header, VXFS_HEADER, 4))
            return;

        // Read inode table
        satadevice->Seek(superblock->InodeTablesStart * 512);

        VXFS_INODE* basetabel = (VXFS_INODE*)((uint64_t)superblock + sizeof(VXFS_SUPERBLOCK));
        satadevice->Read(512, basetabel);

        rootinode = &basetabel[1];

        // Read extent table
        satadevice->Seek(superblock->ExtentTableStart * 512);

        VXFS_EXTENT* extents = (VXFS_EXTENT*)((uint64_t)basetabel + 512);
        satadevice->Read(512, extents);

        rootextent = &extents[0];
    }

    bool VXFS::readfile(const char* file, void* buffer,size_t* osize)
    {
        VXFS_INODE* currentinode = rootinode;
        VXFS_EXTENT* currentextent = rootextent;

        char name[256];


        while (true)
        {
            file = NextEntry(file, name);

            if (!file)
                break;

            FindDirectory(&currentinode, &currentextent, name);

            if (!currentinode || !currentextent)
            {
                Logger::Log("Path not found: %s\n", LOG_LEVEL::ERROR, name);
                return false;
            }
        }


        if (currentinode->Flags & VXFS_FLAGS::DIR)
        {
            Logger::Log("Tried to read a directory\n", LOG_LEVEL::ERROR);
            return false;
        }

        if (buffer == nullptr)
        {
            *osize = currentextent->SizeInSectors * 512;
            return true;
        }

        size_t size = currentextent->SizeInSectors * 512;

        void* tempbuffer = PageAllocater::GetInstance()->RequestPages((size + 4095) / 4096);

        satadevice->Seek(currentextent->StartSector * 512);
        satadevice->Read(size, tempbuffer);

        memcpy(buffer,tempbuffer,size);

        return true;
    }

    void VXFS::FindDirectory(VXFS_INODE** node, VXFS_EXTENT** extent, char* name)
    {
        size_t size = (*extent)->SizeInSectors * 512;

        void* pages = PageAllocater::GetInstance()->RequestPages(
            (size + 4095) / 4096
        );

        satadevice->Seek((*extent)->StartSector * 512);
        satadevice->Read(size, pages);

        size_t offset = 0;

        while (offset < size)
        {
            if (offset + sizeof(VXFS_DIRENTRY) > size)
                break;

            VXFS_DIRENTRY* entry = (VXFS_DIRENTRY*)((uint64_t)pages + offset);

            if (!entry->valid)
            {
                offset += sizeof(VXFS_DIRENTRY);
                continue;
            }

            char* entryName = (char*)((uint64_t)entry + sizeof(VXFS_DIRENTRY));

            if (offset + sizeof(VXFS_DIRENTRY) + entry->NameLenght > size)
                break;

            if (entry->NameLenght == strlen(name) &&
                !memcmp(entryName, name, entry->NameLenght))
            {

                uint64_t inodeSector =
                    superblock->InodeTablesStart +
                    entry->InodeTableID * superblock->InodeTableSize +
                    ((entry->InodeID * sizeof(VXFS_INODE)) / 512);

                satadevice->Seek(inodeSector * 512);

                void* inodePage = PageAllocater::GetInstance()->RequestPage();
                satadevice->Read(512, inodePage);

                VXFS_INODE* table = (VXFS_INODE*)inodePage;

                VXFS_INODE* newNode =
                    &table[entry->InodeID % (512 / sizeof(VXFS_INODE))];

                uint64_t extentSector =
                    superblock->ExtentTableStart +
                    newNode->ExtentTableID * superblock->ExtentTableSize +
                    ((newNode->ExtentID * sizeof(VXFS_EXTENT)) / 512);

                satadevice->Seek(extentSector * 512);

                void* extentPage = PageAllocater::GetInstance()->RequestPage();
                satadevice->Read(512, extentPage);

                VXFS_EXTENT* extentTable = (VXFS_EXTENT*)extentPage;

                VXFS_EXTENT* newExtent =
                    &extentTable[newNode->ExtentID % (512 / sizeof(VXFS_EXTENT))];


                *node = newNode;
                *extent = newExtent;

                return;
            }

            offset += sizeof(VXFS_DIRENTRY) + entry->NameLenght;
        }

        *node = nullptr;
        *extent = nullptr;
    }
}