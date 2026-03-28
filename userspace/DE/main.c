#include <sys/devc.h>

int _start()
{
    fid_t fid = open("/dev/fb0", FD_WRITE);

    unsigned int pixel = 0xFFFF0000;

    write(fid, (void*)&pixel, 4);

    while (1) {}
}