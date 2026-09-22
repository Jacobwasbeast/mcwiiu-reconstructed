#include "nn/pia/inet/NexSessionSearchCriteriaOwner.hpp"

namespace nn { namespace pia { namespace inet {

NexSessionSearchCriteriaOwner::~NexSessionSearchCriteriaOwner() = default;

nn::pia::common::Result NexSessionSearchCriteriaOwner::SetResultRange(uint32_t offset, uint32_t size)
{
    if (size < 0x65) {
        return nn::pia::session::SessionSearchCriteria::SetResultRange(offset, size);
    }
    return 0x10c07;
}

uint64_t NexSessionSearchCriteriaOwner::GetOwnerPrincipalId() const
{
    return owner_principal_id_;
}

void NexSessionSearchCriteriaOwner::Reset()
{
    nn::pia::session::SessionSearchCriteria::Reset();
    nn::pia::session::SessionSearchCriteria::SetResultRange(0xffffffffu, 100u);
    nn::pia::session::SessionSearchCriteria::SetKeyType(nn::pia::session::SessionSearchCriteria::cKeyTypeOwner);
    owner_principal_id_ = 0;
}

}}}
