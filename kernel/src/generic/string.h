#pragma once

#include <generic/stdint.h>

void* memcpy(void* dest,const void* src,size_t count);

void* memset(void* dst,int c,size_t size);
int64_t memcmp(void *m1, const void *m2, size_t len);

int64_t strlen(const char* str);