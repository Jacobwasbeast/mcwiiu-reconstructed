#include "game/ui/fuiShapeComponent.hpp"

void Swap4Bytes(unsigned int& value);

void fuiShapeComponent::EndianSwap()
{
    fuiFillStyle::EndianSwap();
    Swap4Bytes(vertexCount);
    Swap4Bytes(vertexOffset);
}

void fuiShapeComponent::EndianSwap(void* pData)
{
    static_cast<fuiShapeComponent*>(pData)->EndianSwap();
}
