#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_COMMUNITY_MANAGEMENT_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_COMMUNITY_MANAGEMENT_JOB_HPP

#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/CallContext.hpp"

namespace nn { namespace pia { namespace session {

class CommunitySearchCriteria;
class CreateCommunitySetting;
class JoinCommunitySetting;
class UpdateCommunitySetting;

class CommunityManagementJob : public common::StepSequenceJob
{
public:
    CommunityManagementJob();
    virtual ~CommunityManagementJob();

    Result BrowseCommunityStartup(common::CallContext*, const CommunitySearchCriteria*);
    Result CreateCommunityStartup(common::CallContext*, const CreateCommunitySetting*);
    Result JoinCommunityStartup(common::CallContext*, const JoinCommunitySetting*);
    Result UpdateCommunityStartup(common::CallContext*, const UpdateCommunitySetting*);
    Result DestroyCommunityStartup(common::CallContext*, unsigned int communityId);
    Result LeaveCommunityStartup(common::CallContext*, unsigned int communityId);
    void Cleanup();
    virtual void Trace(unsigned long long value) const;

protected:
    virtual void BrowseCommunityStartupImpl(Result* result) = 0;
    virtual void CreateCommunityStartupImpl(Result* result) = 0;
    virtual void JoinCommunityStartupImpl(Result* result, const JoinCommunitySetting* setting) = 0;
    virtual void UpdateCommunityStartupImpl(Result* result, const UpdateCommunitySetting* setting) = 0;
    virtual void DestroyCommunityStartupImpl(Result* result, unsigned int communityId) = 0;
    virtual void LeaveCommunityStartupImpl(Result* result, unsigned int communityId) = 0;

    common::CallContext* m_callerContext;
    common::CallContext m_callContext;
};

}}}

#endif
