#include "game/ui/fuiObject.hpp"

void Swap4Bytes(unsigned int& value);

void fuiObject::EndianSwap()
{
    Swap4Bytes(symbolIndex);
}
