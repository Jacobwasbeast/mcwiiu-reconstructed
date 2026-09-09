#include "nn/nex/DataStorePermission.hpp"

namespace nn
{
namespace nex
{

DataStorePermission::DataStorePermission(std::uint8_t uiPermission,
                                         const qVector<std::uint32_t>& rRecipientIds)
{
    m_uiPermission = uiPermission;
    m_RecipientIds = rRecipientIds;
}

DataStorePermission::~DataStorePermission()
{
}

}
}
