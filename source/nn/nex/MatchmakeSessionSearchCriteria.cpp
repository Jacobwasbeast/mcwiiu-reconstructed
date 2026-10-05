#include "nn/nex/MatchmakeSessionSearchCriteria.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int ATTRIBUTE_COUNT = 6;
const unsigned int MAX_ATTRIBUTE_VALUE_COUNT = 100;

}

MatchmakeSessionSearchCriteria::MatchmakeSessionSearchCriteria()
{
    Reset();
}

MatchmakeSessionSearchCriteria::MatchmakeSessionSearchCriteria(unsigned int uiGameMode, MatchmakeSystemType type)
{
    Reset();
    SetGameMode(uiGameMode);
    SetMatchmakeSystemType(type);
}

MatchmakeSessionSearchCriteria::~MatchmakeSessionSearchCriteria()
{
}

void MatchmakeSessionSearchCriteria::Reset()
{
    m_Attributes.assign(ATTRIBUTE_COUNT, String(L""));
    m_strGameMode = L"";
    m_strMinParticipants = L"";
    m_strMaxParticipants = L"";
    m_strMatchmakeSystemType = L"";
    m_bVacantOnly = false;
    m_bExcludeUserPasswordSet = true;
    m_bExcludeNonHostPid = true;
    m_bExcludeSystemPasswordSet = true;
    m_uiSelectionMethod = 0;
    m_uiVacantParticipants = 1;
    m_uiReferGatheringID = 0;
    m_bExcludeLocked = true;
}

void MatchmakeSessionSearchCriteria::SetGameMode(unsigned int uiGameMode)
{
    m_strGameMode.Format(L"%u", uiGameMode);
}

void MatchmakeSessionSearchCriteria::SetMatchmakeSystemType(MatchmakeSystemType type)
{
    m_strMatchmakeSystemType.Format(L"%u", type);
}

void MatchmakeSessionSearchCriteria::SetMinParticipants(unsigned short uiMinParticipants)
{
    m_strMinParticipants.Format(L"%u", uiMinParticipants);
}

void MatchmakeSessionSearchCriteria::SetMinParticipantsWithRange(unsigned short uiMinimum, unsigned short uiMaximum)
{
    m_strMinParticipants.Format(L"%u,%u", uiMinimum, uiMaximum);
}

void MatchmakeSessionSearchCriteria::SetMaxParticipants(unsigned short uiMaxParticipants)
{
    m_strMaxParticipants.Format(L"%u", uiMaxParticipants);
}

void MatchmakeSessionSearchCriteria::SetMaxParticipantsWithRange(unsigned short uiMinimum, unsigned short uiMaximum)
{
    m_strMaxParticipants.Format(L"%u,%u", uiMinimum, uiMaximum);
}

void MatchmakeSessionSearchCriteria::SetAttribute(unsigned int uiIndex, unsigned int uiValue)
{
    m_Attributes[uiIndex].Format(L"%u", uiValue);
}

bool MatchmakeSessionSearchCriteria::SetAttribute(unsigned int uiIndex, const qVector<unsigned int>& rValues)
{
    const unsigned int uiCount = rValues.size();
    if (uiCount == 0 || uiCount > MAX_ATTRIBUTE_VALUE_COUNT)
        return false;
    qVector<unsigned int>::const_iterator it = rValues.begin();
    m_Attributes[uiIndex].Format(L"%u", *it);
    for (++it; it != rValues.end(); ++it)
    {
        String strValue(L"");
        strValue.Format(L"|%u", *it);
        m_Attributes[uiIndex] += strValue;
    }
    return true;
}

void MatchmakeSessionSearchCriteria::SetAttributeWithRange(unsigned int uiIndex, unsigned int uiMinimum, unsigned int uiMaximum)
{
    m_Attributes[uiIndex].Format(L"%u,%u", uiMinimum, uiMaximum);
}

void MatchmakeSessionSearchCriteria::SetExcludeLocked(bool bExcludeLocked)
{
    m_bExcludeLocked = bExcludeLocked;
}

void MatchmakeSessionSearchCriteria::SetVacantOnly(bool bVacantOnly)
{
    m_bVacantOnly = bVacantOnly;
    m_uiVacantParticipants = 1;
}

void MatchmakeSessionSearchCriteria::SetVacantOnly(bool bVacantOnly, unsigned short uiVacantParticipants)
{
    m_bVacantOnly = bVacantOnly;
    m_uiVacantParticipants = uiVacantParticipants;
}

void MatchmakeSessionSearchCriteria::SetExcludeNonHostPid(bool bExcludeNonHostPid)
{
    m_bExcludeNonHostPid = bExcludeNonHostPid;
}

void MatchmakeSessionSearchCriteria::SetExcludeUserPasswordSet(bool bExcludeUserPasswordSet)
{
    m_bExcludeUserPasswordSet = bExcludeUserPasswordSet;
}

void MatchmakeSessionSearchCriteria::SetSelectionMethod(MatchmakeSelectionMethod method)
{
    m_uiSelectionMethod = method;
}

void MatchmakeSessionSearchCriteria::SetSelectionMethod(MatchmakeSelectionMethod method, const MatchmakeParam& rParam)
{
    m_uiSelectionMethod = method;
    m_MatchmakeParam = rParam;
}

}
}
