#include "nn/pia/session/CommunityManagementJob.hpp"

namespace nn { namespace pia { namespace session {

namespace {
const int kInvalidArgument = 0x10c07;
}

CommunityManagementJob::CommunityManagementJob()
    : common::StepSequenceJob(), m_callerContext(0), m_callContext()
{
}

CommunityManagementJob::~CommunityManagementJob()
{
}

Result CommunityManagementJob::BrowseCommunityStartup(common::CallContext* context,
                                                       const CommunitySearchCriteria* criteria)
{
    Result result = 0;
    if (context == 0 || criteria == 0)
        result = static_cast<Result>(kInvalidArgument);
    else
    {
        BrowseCommunityStartupImpl(&result);
        if (result == 0)
        {
            m_callerContext = context;
            context->InitiateCall();
            Reset(true);
            return 0;
        }
    }
    return result;
}

Result CommunityManagementJob::CreateCommunityStartup(common::CallContext* context,
                                                       const CreateCommunitySetting* setting)
{
    Result result = 0;
    if (context == 0 || setting == 0)
        result = static_cast<Result>(kInvalidArgument);
    else
    {
        CreateCommunityStartupImpl(&result);
        if (result == 0)
        {
            m_callerContext = context;
            context->InitiateCall();
            Reset(true);
            return 0;
        }
    }
    return result;
}

Result CommunityManagementJob::JoinCommunityStartup(common::CallContext* context,
                                                     const JoinCommunitySetting* setting)
{
    Result result = 0;
    if (context == 0)
        result = static_cast<Result>(kInvalidArgument);
    else
    {
        JoinCommunityStartupImpl(&result, setting);
        if (result == 0)
        {
            m_callerContext = context;
            context->InitiateCall();
            Reset(true);
            return 0;
        }
    }
    return result;
}

Result CommunityManagementJob::UpdateCommunityStartup(common::CallContext* context,
                                                       const UpdateCommunitySetting* setting)
{
    Result result = 0;
    if (context == 0)
        result = static_cast<Result>(kInvalidArgument);
    else
    {
        UpdateCommunityStartupImpl(&result, setting);
        if (result == 0)
        {
            m_callerContext = context;
            context->InitiateCall();
            Reset(true);
            return 0;
        }
    }
    return result;
}

Result CommunityManagementJob::DestroyCommunityStartup(common::CallContext* context,
                                                        unsigned int communityId)
{
    Result result = 0;
    if (context == 0)
        result = static_cast<Result>(kInvalidArgument);
    else
    {
        DestroyCommunityStartupImpl(&result, communityId);
        if (result == 0)
        {
            m_callerContext = context;
            context->InitiateCall();
            Reset(true);
            return 0;
        }
    }
    return result;
}

Result CommunityManagementJob::LeaveCommunityStartup(common::CallContext* context,
                                                      unsigned int communityId)
{
    Result result = 0;
    if (context == 0)
        result = static_cast<Result>(kInvalidArgument);
    else
    {
        LeaveCommunityStartupImpl(&result, communityId);
        if (result == 0)
        {
            m_callerContext = context;
            context->InitiateCall();
            Reset(true);
            return 0;
        }
    }
    return result;
}

void CommunityManagementJob::Cleanup()
{
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
    }
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();
    m_callContext.Reset();
}

void CommunityManagementJob::Trace(unsigned long long) const
{
}

}}}
