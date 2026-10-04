#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHCOMMUNITYMANAGEMENTJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHCOMMUNITYMANAGEMENTJOB_HPP

#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace session { class CommunitySearchCriteria; class CreateCommunitySetting; class JoinCommunitySetting; class UpdateCommunitySetting; }}}

namespace nn { namespace pia { namespace inet {

class NexMatchCommunityManagementJob
{
public:
    NexMatchCommunityManagementJob();
    virtual ~NexMatchCommunityManagementJob();
    Result BrowseCommunityStartupImpl(const session::CommunitySearchCriteria* searchCriteria);
    void Cleanup();
    unsigned int CreateCommunity();
    Result CreateCommunityStartupImpl(const session::CreateCommunitySetting* setting);
    unsigned int DestroyCommunity();
    Result DestroyCommunityStartupImpl(unsigned int communityId);
    unsigned int FindCommunityByCommunityId();
    unsigned int FindCommunityByOwner();
    unsigned int FindCommunityByParticipant();
    unsigned int FindOfficialCommunity();
    unsigned int JoinCommunity();
    Result JoinCommunityStartupImpl(const session::JoinCommunitySetting* setting);
    unsigned int LeaveCommunity();
    Result LeaveCommunityStartupImpl(unsigned int communityId);
    void Trace(unsigned long long p0) const;
    unsigned int UpdateCommunity();
    Result UpdateCommunityStartupImpl(const session::UpdateCommunitySetting* setting);
    unsigned int WaitCreateCommunity();
    unsigned int WaitDestroyCommunity();
    unsigned int WaitFindCommunityByCommunityId();
    unsigned int WaitFindCommunityByOwner();
    unsigned int WaitFindCommunityByParticipant();
    unsigned int WaitFindOfficialCommunity();
    unsigned int WaitJoinCommunity();
    unsigned int WaitLeaveCommunity();
    unsigned int WaitUpdateCommunity();

private:
    unsigned int m_state;
};

}}}

#endif
