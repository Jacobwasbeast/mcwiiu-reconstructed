#include "nn/nex/_DDL_DataStoreSearchResult.hpp"

#include "nn/nex/DDLClassMetaType.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t SEARCH_RESULT_MAX_STRUCTURE_VERSION = 0;
const unsigned int SIZE_FIELD_LENGTH = 4;

}

_DDL_DataStoreSearchResult::_DDL_DataStoreSearchResult()
    : m_Result()
{
    m_uiStructureVersion = 0;
}

_DDL_DataStoreSearchResult::~_DDL_DataStoreSearchResult()
{
}

void _DDL_DataStoreSearchResult::Add(Message* pMessage, const _DDL_DataStoreSearchResult& rSource)
{
    const bool bCompactFormat = pMessage->IsCompactFormat();
    unsigned int uiSizePosition = 0;

    if (!bCompactFormat)
    {
        const uint8_t uiVersion = rSource.m_uiStructureVersion;
        const uint8_t uiMaxVersion = SEARCH_RESULT_MAX_STRUCTURE_VERSION;

        pMessage->Append(uiMaxVersion < uiVersion ? &uiMaxVersion : &uiVersion, 1, 1);

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;

        pMessage->Append(reinterpret_cast<const unsigned char*>(&uiSize), SIZE_FIELD_LENGTH, 1);
    }

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiTotalCount), 4, 1);
    _Type_qvector<DDLClassMetaType<DataStoreMetaInfo> >::Add(pMessage, rSource.m_Result);
    pMessage->Append(&rSource.m_uiTotalCountType, 1, 1);

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

void _DDL_DataStoreSearchResult::Extract(Message* pMessage,
    _DDL_DataStoreSearchResult* pDestination)
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

    pMessage->ExtractPrimitive(&pDestination->m_uiTotalCount);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_qvector<DDLClassMetaType<DataStoreMetaInfo> >::Extract(pMessage, &pDestination->m_Result);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiTotalCountType);

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
