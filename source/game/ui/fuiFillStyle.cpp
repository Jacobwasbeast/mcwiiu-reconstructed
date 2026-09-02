#include "game/ui/fuiFillStyle.hpp"

void Swap4Bytes(eFuiFillType& value);
void Swap4Bytes(unsigned int& value);

void fuiFillStyle::EndianSwap()
{
    Swap4Bytes(fillType);
    color.EndianSwap();
    Swap4Bytes(bitmapId);
    bitmapMatrix.EndianSwap();
}
