#pragma once

#include <generic/stdint.h>

struct Bitmap
{
    uint8_t* bitmap;
    size_t size;

    bool operator[](size_t index) const
    {
        if (index > size * 8) return false;

        uint8_t byteIndex = index / 8;
        uint8_t bitindex = index % 8;
        uint8_t bitindexer = (0b10000000 >> bitindex);

        return (bitmap[byteIndex] & bitindexer) > 0;
    }

    void Set(size_t index,bool value) 
    {

        uint8_t byteIndex = index / 8;
        uint8_t bitindex = index % 8;
        uint8_t bitindexer = (0b10000000 >> bitindex);

        bitmap[byteIndex] &= ~bitindexer;
        if (value)
        {
            bitmap[byteIndex] |= bitindexer;
        }
    }
};
