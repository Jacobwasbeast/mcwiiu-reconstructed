#include "nn/nex/DataStorePermission.hpp"

namespace nn
{
namespace nex
{

DataStorePermission::DataStorePermission(uint8_t uiPermission,
                                         const qVector<uint32_t>& rRecipientIds)
{
    m_uiPermission = uiPermission;
    m_RecipientIds = rRecipientIds;
}

}
}
