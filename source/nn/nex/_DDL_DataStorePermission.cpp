#include "nn/nex/_DDL_DataStorePermission.hpp"

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t PERMISSION_MAX_STRUCTURE_VERSION = 0;
const unsigned int SIZE_FIELD_LENGTH = 4;

}

_DDL_DataStorePermission::_DDL_DataStorePermission()
    : m_RecipientIds()
{
    m_uiStructureVersion = 0;
}

_DDL_DataStorePermission::~_DDL_DataStorePermission()
{
}

_DDL_DataStorePermission& _DDL_DataStorePermission::operator=(
    const _DDL_DataStorePermission& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiPermission = rSource.m_uiPermission;
    m_RecipientIds = rSource.m_RecipientIds;
    return *this;
}

void _DDL_DataStorePermission::Add(Message* pMessage, const _DDL_DataStorePermission& rSource)
{
    const bool bCompactFormat = pMessage->IsCompactFormat();
    unsigned int uiSizePosition = 0;

    if (!bCompactFormat)
    {
        const uint8_t uiVersion = rSource.m_uiStructureVersion;
        const uint8_t uiMaxVersion = PERMISSION_MAX_STRUCTURE_VERSION;

        pMessage->Append(uiMaxVersion < uiVersion ? &uiMaxVersion : &uiVersion, 1, 1);

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;

        pMessage->Append(reinterpret_cast<const unsigned char*>(&uiSize), SIZE_FIELD_LENGTH, 1);
    }

    pMessage->Append(&rSource.m_uiPermission, 1, 1);
    _Type_qvector<_Type_uint32>::Add(pMessage, rSource.m_RecipientIds);

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

void _DDL_DataStorePermission::Extract(Message* pMessage,
    _DDL_DataStorePermission* pDestination)
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

    pMessage->ExtractPrimitive(&pDestination->m_uiPermission);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_qvector<_Type_uint32>::Extract(pMessage, &pDestination->m_RecipientIds);

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
