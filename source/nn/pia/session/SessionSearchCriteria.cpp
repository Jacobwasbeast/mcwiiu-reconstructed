#include "nn/pia/Reconstruction.hpp"

namespace nn { namespace pia { namespace session {

SessionSearchCriteria::SessionSearchCriteria()
    : m_resultRangeOffset(0),
      m_resultRangeSize(20),
      m_keyType(0)
{
}

SessionSearchCriteria::~SessionSearchCriteria()
{
}

Result SessionSearchCriteria::SetResultRange(unsigned int offset, unsigned int size)
{
    m_resultRangeOffset = offset;
    m_resultRangeSize = size;
    return 0;
}

void SessionSearchCriteria::Reset()
{
    m_resultRangeOffset = 0;
    m_resultRangeSize = 20;
    m_keyType = 0;
}

unsigned int SessionSearchCriteria::GetResultRangeOffset() const
{
    return m_resultRangeOffset;
}

unsigned int SessionSearchCriteria::GetResultRangeSize() const
{
    return m_resultRangeSize;
}

int SessionSearchCriteria::GetKeyType() const
{
    return m_keyType;
}

}}}
