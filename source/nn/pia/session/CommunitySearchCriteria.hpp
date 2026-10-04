#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_COMMUNITY_SEARCH_CRITERIA_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_COMMUNITY_SEARCH_CRITERIA_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/session/SessionSearchCriteria.hpp"

namespace nn { namespace pia { namespace session {

class CommunitySearchCriteria
{
public:
    CommunitySearchCriteria();
    CommunitySearchCriteria(SessionSearchCriteria::SearchCriteriaKey keyType);
    ~CommunitySearchCriteria();
    void Copy(const CommunitySearchCriteria& other);
    Result SetResultRange(unsigned int offset, unsigned int size);
    SessionSearchCriteria::SearchCriteriaKey GetKeyType() const;
    unsigned int GetResultRangeOffset() const;
    unsigned int GetResultRangeSize() const;
    void Reset();

private:
    SessionSearchCriteria::SearchCriteriaKey m_keyType;
    unsigned int m_resultRangeOffset;
    unsigned int m_resultRangeSize;
};

}}}

#endif
