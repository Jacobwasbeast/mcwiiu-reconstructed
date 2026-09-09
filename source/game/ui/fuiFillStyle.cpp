#include "game/ui/fuiFillStyle.hpp"

void Swap4Bytes(eFuiFillType& value);
void Swap4Bytes(unsigned int& value);

void Swap4Bytes(eFuiFillType& value)
{
    unsigned int bits = static_cast<unsigned int>(value);
    Swap4Bytes(bits);
    value = static_cast<eFuiFillType>(bits);
}

void fuiFillStyle::EndianSwap()
{
    Swap4Bytes(fillType);
    color.EndianSwap();
    Swap4Bytes(bitmapId);
    bitmapMatrix.EndianSwap();
}
