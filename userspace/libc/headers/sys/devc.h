#ifndef DEVC_H
#define DEVC_H

#ifndef SIZE_T
typedef unsigned long size_t;
#endif

typedef int fid_t;

#define FD_READ  0x01
#define FD_WRITE 0x02

fid_t open(const char* location, int flags);

size_t write(fid_t file, void* buffer, size_t size);

#endif