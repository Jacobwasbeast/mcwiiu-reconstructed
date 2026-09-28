#include "nn/nex/_DDL_DataStoreRatingInfoWithSlot.hpp"

#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t RATING_INFO_WITH_SLOT_MAX_STRUCTURE_VERSION = 0;
const unsigned int SIZE_FIELD_LENGTH = 4;

}

_DDL_DataStoreRatingInfoWithSlot::_DDL_DataStoreRatingInfoWithSlot()
    : m_Rating()
{
    m_uiStructureVersion = 0;
}

_DDL_DataStoreRatingInfoWithSlot::~_DDL_DataStoreRatingInfoWithSlot()
{
}

_DDL_DataStoreRatingInfoWithSlot& _DDL_DataStoreRatingInfoWithSlot::operator=(
    const _DDL_DataStoreRatingInfoWithSlot& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_iSlot = rSource.m_iSlot;
    m_Rating = rSource.m_Rating;
    return *this;
}

void _DDL_DataStoreRatingInfoWithSlot::Add(Message* pMessage,
    const _DDL_DataStoreRatingInfoWithSlot& rSource)
{
    const bool bCompactFormat = pMessage->IsCompactFormat();
    unsigned int uiSizePosition = 0;

    if (!bCompactFormat)
    {
        const uint8_t uiVersion = rSource.m_uiStructureVersion;
        const uint8_t uiMaxVersion = RATING_INFO_WITH_SLOT_MAX_STRUCTURE_VERSION;

        pMessage->Append(uiMaxVersion < uiVersion ? &uiMaxVersion : &uiVersion, 1, 1);

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;

        pMessage->Append(reinterpret_cast<const unsigned char*>(&uiSize), SIZE_FIELD_LENGTH, 1);
    }

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_iSlot), 1, 1);
    _DDL_DataStoreRatingInfo::Add(pMessage, rSource.m_Rating);

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

void _DDL_DataStoreRatingInfoWithSlot::Extract(Message* pMessage,
    _DDL_DataStoreRatingInfoWithSlot* pDestination)
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

    pMessage->ExtractPrimitive(&pDestination->m_iSlot);

    if (pMessage->HasError())
    {
        return;
    }

    _DDL_DataStoreRatingInfo::Extract(pMessage, &pDestination->m_Rating);

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
