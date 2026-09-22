#include "nn/pia/inet/NexSessionSearchCriteriaParticipant.hpp"

namespace nn { namespace pia { namespace inet {

NexSessionSearchCriteriaParticipant::~NexSessionSearchCriteriaParticipant() = default;

const unsigned long long* NexSessionSearchCriteriaParticipant::GetParticipantPrincipalId() const
{
    return principal_ids_;
}

uint32_t NexSessionSearchCriteriaParticipant::GetParticipantPrincipalIdSize() const
{
    return principal_id_size_;
}

void NexSessionSearchCriteriaParticipant::Reset()
{
    nn::pia::session::SessionSearchCriteria::Reset();
    principal_id_size_ = 0;
    for (unsigned int index = 0; index != 100; ++index) {
        principal_ids_[index] = 0;
    }
    network_service_account_id_used_ = false;
    my_block_list_used_ = true;
    others_block_list_used_ = false;
    application_data_enabled_ = true;
}

void NexSessionSearchCriteriaParticipant::SetResultRange(uint32_t, nn::pia::common::Result* result)
{
    if (result == 0) {
        result = new nn::pia::common::Result;
    }
    if (result != 0) {
        *result = 0x10c08;
    }
}

}}}
