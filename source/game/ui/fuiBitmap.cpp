#include "game/ui/fuiBitmap.hpp"

void Swap4Bytes(unsigned int& value);

void Swap4Bytes(eFuiBitmapType& value)
{
    unsigned int swapped = static_cast<unsigned int>(value);
    Swap4Bytes(swapped);
    value = static_cast<eFuiBitmapType>(swapped);
}

void Swap4Bytes(int& value);

void fuiBitmap::EndianSwap()
{
    Swap4Bytes(type);
    Swap4Bytes(width);
    Swap4Bytes(height);
    Swap4Bytes(imageDataOffset);
    Swap4Bytes(imageDataSize);
    Swap4Bytes(jpegDataSize);
    Swap4Bytes(textureId);
    fuiObject::EndianSwap();
}

void fuiBitmap::EndianSwap(void* pData)
{
    static_cast<fuiBitmap*>(pData)->EndianSwap();
}
