#include "game/ui/fuiImportAsset.hpp"

void fuiImportAsset::EndianSwap(void* pData)
{
    static_cast<fuiImportAsset*>(pData)->EndianSwap();
}
