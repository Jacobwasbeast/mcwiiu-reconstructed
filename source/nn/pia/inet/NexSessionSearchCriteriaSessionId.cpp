#include "nn/pia/inet/NexSessionSearchCriteriaSessionId.hpp"

namespace nn { namespace pia { namespace inet {

NexSessionSearchCriteriaSessionId::~NexSessionSearchCriteriaSessionId() {}

unsigned int NexSessionSearchCriteriaSessionId::GetSessionId() const
{
    return m_sessionId;
}

void NexSessionSearchCriteriaSessionId::Reset()
{
    session::SessionSearchCriteria::Reset();
    SetResultRange(static_cast<unsigned int>(-1), 100);
    SetKeyType(session::SessionSearchCriteria::cKeyTypeSessionId);
    m_sessionId = 0;
}

nn::pia::common::Result NexSessionSearchCriteriaSessionId::SetResultRange(unsigned int offset, unsigned int size)
{
    return session::SessionSearchCriteria::SetResultRange(offset, size);
}

}}}
