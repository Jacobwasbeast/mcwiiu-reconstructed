#include "nn/nex/_DDL_Data.hpp"

#include "nn/nex/Data.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t DATA_MAX_STRUCTURE_VERSION = 0;
const unsigned int SIZE_FIELD_LENGTH = 4;

}

_DDL_Data& _DDL_Data::operator=(const _DDL_Data&)
{
    return *this;
}

bool _DDL_Data::operator==(const _DDL_Data&) const
{
    return true;
}

String _DDL_Data::GetDataType() const
{
    return String(L"Data");
}

bool _DDL_Data::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"Data");
}

bool _DDL_Data::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"Data");
}

void _DDL_Data::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_Data::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

void _DDL_Data::Add(Message* pMessage, const _DDL_Data& rSource)
{
    if (!pMessage->IsCompactFormat())
    {
        const uint8_t uiVersion = rSource.m_uiStructureVersion;
        const uint8_t uiMaxVersion = DATA_MAX_STRUCTURE_VERSION;
        pMessage->Append(uiMaxVersion < uiVersion ? &uiMaxVersion : &uiVersion, 1, 1);
        const uint32_t uiSize = 0;
        pMessage->Append(reinterpret_cast<const unsigned char*>(&uiSize), SIZE_FIELD_LENGTH, 1);
    }
}

void _DDL_Data::Extract(Message* pMessage, _DDL_Data* pDestination)
{
    pDestination->m_uiStructureVersion = 0;
    uint32_t uiSize = 0;
    if (!pMessage->IsCompactFormat())
    {
        pMessage->ExtractPrimitive(&pDestination->m_uiStructureVersion);
        if (!pMessage->HasError())
            pMessage->ExtractPrimitive(&uiSize);
    }
}

Data* _DDL_Data::DynamicCast(Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"Data")))
        return pData;
    return 0;
}

const Data* _DDL_Data::DynamicCast(const Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"Data")))
        return pData;
    return 0;
}

}
}
