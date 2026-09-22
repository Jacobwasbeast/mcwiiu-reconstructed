#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHCOMMUNITYMANAGEMENTJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHCOMMUNITYMANAGEMENTJOB_HPP

namespace nn { namespace pia { namespace inet {

class NexMatchCommunityManagementJob
{
public:
    NexMatchCommunityManagementJob();
    virtual ~NexMatchCommunityManagementJob();
    unsigned long long BrowseCommunityStartupImpl(unsigned long long p0, unsigned long long p1);
    void Cleanup();
    unsigned int CreateCommunity();
    unsigned long long CreateCommunityStartupImpl(unsigned long long p0);
    unsigned int DestroyCommunity();
    unsigned long long DestroyCommunityStartupImpl(unsigned long long p0, unsigned long long p1);
    unsigned int FindCommunityByCommunityId();
    unsigned int FindCommunityByOwner();
    unsigned int FindCommunityByParticipant();
    unsigned int FindOfficialCommunity();
    unsigned int JoinCommunity();
    unsigned long long JoinCommunityStartupImpl(unsigned long long p0);
    unsigned int LeaveCommunity();
    unsigned long long LeaveCommunityStartupImpl(unsigned long long p0, unsigned long long p1);
    void Trace(unsigned long long p0);
    unsigned int UpdateCommunity();
    unsigned long long UpdateCommunityStartupImpl(unsigned long long p0);
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
