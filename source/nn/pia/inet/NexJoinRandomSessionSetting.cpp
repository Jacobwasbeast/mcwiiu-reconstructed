#include "nn/pia/inet/NexJoinRandomSessionSetting.hpp"

namespace nn { namespace pia { namespace inet {

namespace {
const int kInvalidParameterResult = 0x10c07;
}

NexJoinRandomSessionSetting::NexJoinRandomSessionSetting()
    : m_sessionSearchCriteriaSize(0), m_matchmakeUniqueKeyword(false),
      m_myBlockListUsed(true), m_othersBlockListUsed(false)
{
    Reset();
}

NexJoinRandomSessionSetting::~NexJoinRandomSessionSetting()
{}

NexCreateSessionSetting* NexJoinRandomSessionSetting::GetCreateSessionSetting()
{
    return &m_createSessionSetting;
}
const NexCreateSessionSetting* NexJoinRandomSessionSetting::GetCreateSessionSetting() const
{
    return &m_createSessionSetting;
}
NexSessionSearchCriteria* NexJoinRandomSessionSetting::GetSessionSearchCriteria(unsigned char index)
{
    return index < 2 ? &m_sessionSearchCriteria[index] : 0;
}
const NexSessionSearchCriteria* NexJoinRandomSessionSetting::GetSessionSearchCriteria(unsigned char index) const
{
    return index < 2 ? &m_sessionSearchCriteria[index] : 0;
}
unsigned int NexJoinRandomSessionSetting::GetSessionSearchCriteriaSize()
{
    return m_sessionSearchCriteriaSize;
}
unsigned int NexJoinRandomSessionSetting::GetSessionSearchCriteriaSize() const
{
    return m_sessionSearchCriteriaSize;
}
void NexJoinRandomSessionSetting::Reset()
{
    m_createSessionSetting.Reset();
    for (unsigned int i = 0; i < 2; ++i) m_sessionSearchCriteria[i].Reset();
    m_sessionSearchCriteriaSize = 0;
    m_matchmakeUniqueKeyword = false;
    m_myBlockListUsed = true;
    m_othersBlockListUsed = false;
}
Result NexJoinRandomSessionSetting::SetCreateSessionSetting(const session::CreateSessionSetting* setting)
{
    if (setting == 0) return Result(kInvalidParameterResult);
    m_createSessionSetting.Copy(static_cast<const NexCreateSessionSetting&>(*setting));
    return Result(0);
}
Result NexJoinRandomSessionSetting::SetSessionSearchCriteria(const session::SessionSearchCriteria* criteria, unsigned int count)
{
    if (count > 2 || criteria == 0) return Result(kInvalidParameterResult);
    const NexSessionSearchCriteria* nexCriteria = static_cast<const NexSessionSearchCriteria*>(criteria);
    for (unsigned int i = 0; i < count; ++i) {
        if (nexCriteria[i].GetKeyType() != 0) return Result(kInvalidParameterResult);
    }
    for (unsigned int i = 0; i < count; ++i) {
        m_sessionSearchCriteria[i].Copy(nexCriteria[i]);
    }
    m_sessionSearchCriteriaSize = count;
    return Result(0);
}
void NexJoinRandomSessionSetting::Trace(unsigned long long p0) const
{
    (void)p0;
}

}}}
