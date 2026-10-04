#include "nn/nex/_DDL_GatheringStats.hpp"

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

void _DDL_GatheringStats::Add(Message* pMessage, const _DDL_GatheringStats& rSource)
{
    const bool bCompactFormat = pMessage->IsCompactFormat();
    unsigned int uiSizePosition = 0;

    if (!bCompactFormat)
    {
        const uint8_t uiVersion = 0;
        _Type_byte::Add(pMessage, uiVersion);
        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    _Type_uint32::Add(pMessage, rSource.m_uiValue1);
    _Type_uint32::Add(pMessage, rSource.m_uiValue2);
    _Type_std_list<_Type_float>::Add(pMessage, rSource.m_Values);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();
        pMessage->SetLength(uiSizePosition);

        const uint32_t uiSize = uiEndPosition - uiSizePosition - sizeof(uint32_t);
        _Type_uint32::Add(pMessage, uiSize);
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_GatheringStats::Extract(Message* pMessage, _DDL_GatheringStats* pDestination)
{
    pDestination->m_uiStructureVersion = 0;

    const bool bCompactFormat = pMessage->IsCompactFormat();
    uint32_t uiSize = 0;

    if (!bCompactFormat)
    {
        _Type_byte::Extract(pMessage, &pDestination->m_uiStructureVersion);
        if (pMessage->HasError())
        {
            return;
        }

        _Type_uint32::Extract(pMessage, &uiSize);
        if (pMessage->HasError())
        {
            return;
        }
    }

    const unsigned int uiStartPosition = pMessage->GetPosition();
    if (pMessage->HasError())
    {
        return;
    }

    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue1);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue2);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_std_list<_Type_float>::Extract(pMessage, &pDestination->m_Values);
    if (pMessage->HasError() || bCompactFormat)
    {
        return;
    }

    const unsigned int uiEndPosition = uiStartPosition + uiSize;
    if (pMessage->GetPosition() <= uiEndPosition && uiEndPosition <= pMessage->GetLength())
    {
        pMessage->SetPosition(uiEndPosition);
        return;
    }

    pMessage->SetError();
}

}
}
