#include "nn/nex/_DDL_ResultRange.hpp"

#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t RESULT_RANGE_MAX_STRUCTURE_VERSION = 0;
const unsigned int SIZE_FIELD_LENGTH = 4;

}

_DDL_ResultRange::_DDL_ResultRange()
{
    m_uiStructureVersion = 0;
}

_DDL_ResultRange::~_DDL_ResultRange()
{
}

_DDL_ResultRange& _DDL_ResultRange::operator=(const _DDL_ResultRange& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiOffset = rSource.m_uiOffset;
    m_uiSize = rSource.m_uiSize;
    return *this;
}

void _DDL_ResultRange::Add(Message* pMessage, const _DDL_ResultRange& rSource)
{
    const bool bCompactFormat = pMessage->IsCompactFormat();
    unsigned int uiSizePosition = 0;

    if (!bCompactFormat)
    {
        const uint8_t uiVersion = rSource.m_uiStructureVersion;
        const uint8_t uiMaxVersion = RESULT_RANGE_MAX_STRUCTURE_VERSION;

        pMessage->Append(uiMaxVersion < uiVersion ? &uiMaxVersion : &uiVersion, 1, 1);

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;

        pMessage->Append(reinterpret_cast<const unsigned char*>(&uiSize), SIZE_FIELD_LENGTH, 1);
    }

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiOffset), 4, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiSize), 4, 1);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        const int32_t iSize =
            static_cast<int32_t>(uiEndPosition - uiSizePosition - SIZE_FIELD_LENGTH);

        pMessage->Append(reinterpret_cast<const unsigned char*>(&iSize), SIZE_FIELD_LENGTH, 1);
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_ResultRange::Extract(Message* pMessage, _DDL_ResultRange* pDestination)
{
    pDestination->m_uiStructureVersion = 0;

    const bool bCompactFormat = pMessage->IsCompactFormat();
    uint32_t uiSize = 0;

    if (!bCompactFormat)
    {
        pMessage->ExtractPrimitive(&pDestination->m_uiStructureVersion);

        if (pMessage->HasError())
        {
            return;
        }

        pMessage->ExtractPrimitive(&uiSize);

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

    pMessage->ExtractPrimitive(&pDestination->m_uiOffset);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiSize);

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
