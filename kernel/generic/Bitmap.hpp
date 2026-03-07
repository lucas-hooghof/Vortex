#pragma once

#include <generic/stdint.hpp>

struct Bitmap
{
    size_t size;
    uint8_t* addr;

    bool operator[](size_t index)
    {
        if (index > size * 8) return false;
        size_t byteindex = index / 8;
        uint8_t bitindex = index % 8;

        uint8_t bitindexer = 0b10000000 >> bitindex;

        if ((addr[byteindex] & bitindexer) > 0){
            return true;
        }
        return false;
    }
    bool Set(size_t index,bool value)
    {
        if (index > size * 8) return false;
        uint64_t byteIndex = index / 8;
        uint8_t bitIndex = index % 8;
        uint8_t bitIndexer = 0b10000000 >> bitIndex;
        addr[byteIndex] &= ~bitIndexer;
        if (value){
            addr[byteIndex] |= bitIndexer;
        }
        return true;
    }
};