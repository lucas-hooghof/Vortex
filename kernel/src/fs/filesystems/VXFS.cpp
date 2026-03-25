#include <fs/filesystems/VXFS.h>

namespace fs
{
    VXFS::VXFS(const char* drive)
    {
        sd = fs::VFS::Open(drive,FD_READ | FD_WRITE);
        if (sd == -1)
        {
            return;
        }

        satadevice = (fs::SataDevice*)fs::VFS::GetInterface(sd);

        
    }
}