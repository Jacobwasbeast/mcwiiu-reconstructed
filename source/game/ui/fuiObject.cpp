#include "game/ui/fuiObject.hpp"
#include "game/ui/fuiSymbol.hpp"

#include <cstring>

void Swap4Bytes(unsigned int& value)
{
    unsigned int swapped = value;
    swapped = ((swapped << 24) | ((swapped & 0x0000ff00) << 8) |
               (swapped >> 24) | ((swapped >> 8) & 0x0000ff00));
    value = swapped;
}

void Swap4Bytes(eFuiObjectType& value)
{
    unsigned int swapped = static_cast<unsigned int>(value);
    Swap4Bytes(swapped);
    value = static_cast<eFuiObjectType>(swapped);
}

void Swap2Bytes(unsigned short& value)
{
    value = static_cast<unsigned short>((value << 8) | (value >> 8));
}

void Swap2Bytes(short& value)
{
    unsigned short swapped = static_cast<unsigned short>(value);
    Swap2Bytes(swapped);
    value = static_cast<short>(swapped);
}

void Swap4Bytes(int& value)
{
    unsigned int swapped = static_cast<unsigned int>(value);
    Swap4Bytes(swapped);
    value = static_cast<int>(swapped);
}

void Swap4Bytes(float& value)
{
    unsigned int bits;
    std::memcpy(&bits, &value, sizeof(bits));
    Swap4Bytes(bits);
    std::memcpy(&value, &bits, sizeof(value));
}

void fuiObject::EndianSwap()
{
    Swap4Bytes(symbolIndex);
}
