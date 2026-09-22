#include "nn/pia/inet/NexJoinRandomSessionSetting.hpp"

namespace nn { namespace pia { namespace inet {

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
void NexJoinRandomSessionSetting::SetCreateSessionSetting(const NexCreateSessionSetting* setting)
{
    if (setting) m_createSessionSetting.Copy(*setting);
}
void NexJoinRandomSessionSetting::SetSessionSearchCriteria(const NexSessionSearchCriteria* criteria, unsigned int count)
{
    if (count > 2 || (count != 0 && criteria == 0)) return;
    for (unsigned int i = 0; i < count; ++i) {
        m_sessionSearchCriteria[i].Copy(criteria[i]);
    }
    m_sessionSearchCriteriaSize = count;
}
void NexJoinRandomSessionSetting::Trace(unsigned long long p0) const
{
    (void)p0;
}

}}}
