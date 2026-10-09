#include "nn/pia/session/SessionStatusCheckJob.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/JointSessionJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/session/SessionInfo.hpp"

namespace nn { namespace pia { namespace session {

SessionStatusCheckJob::SessionStatusCheckJob()
    : common::StepSequenceJob(), m_callContext(), m_field54(false), m_field55(false)
{
}

SessionStatusCheckJob::~SessionStatusCheckJob()
{
}

void SessionStatusCheckJob::Trace(unsigned long long value) const
{
}

Result SessionStatusCheckJob::Startup()
{
    const Result result = g_piaSession->m_meshLayerController->CheckConnectedStatus();
    if (result != 0)
        return result;

    Reset(true);
    if (g_piaSession->m_jointSessionJob == 0)
    {
        m_currentStep.m_function =
            static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(
                &SessionStatusCheckJob::CheckSessionStatus);
        m_currentStep.m_name = "SessionStatusCheckJob::CheckSessionStatus";
    }
    else
    {
        m_currentStep.m_function =
            static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(
                &SessionStatusCheckJob::CheckSessionStatus4JointSession);
        m_currentStep.m_name = "SessionStatusCheckJob::CheckSessionStatus4JointSession";
    }
    return 0;
}

SessionStatusCheckJob::ExecuteResult SessionStatusCheckJob::CheckSessionStatus()
{
    const Result connectedStatus =
        g_piaSession->m_meshLayerController->CheckConnectedStatus();
    if (connectedStatus == static_cast<Result>(0xc406))
    {
        g_piaSession->m_disconnectStatus = 3;
        if (g_piaSession->m_sessionDisconnectReason != 2)
            g_piaSession->m_sessionDisconnectReason = 8;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (connectedStatus == static_cast<Result>(0xa471))
    {
        g_piaSession->m_disconnectStatus = 4;
        if (g_piaSession->m_sessionDisconnectReason != 2)
            g_piaSession->m_sessionDisconnectReason = 8;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (connectedStatus == static_cast<Result>(0xc420))
    {
        g_piaSession->m_disconnectStatus = 5;
        if (g_piaSession->m_sessionDisconnectReason != 2)
            g_piaSession->m_sessionDisconnectReason = 8;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (connectedStatus == static_cast<Result>(0xc421))
    {
        g_piaSession->m_disconnectStatus = 6;
        if (g_piaSession->m_sessionDisconnectReason != 2)
            g_piaSession->m_sessionDisconnectReason = 8;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (connectedStatus == static_cast<Result>(0x8c56))
    {
        g_piaSession->m_disconnectStatus = 2;
        if (g_piaMesh->CheckConnectionError() == static_cast<Result>(0x6c50))
        {
            const unsigned int disconnectReason = g_piaMesh->GetDisconnectReason();
            if (disconnectReason < 2 || (disconnectReason > 2 && disconnectReason < 10))
                g_piaSession->m_disconnectStatus = 2;
            if (g_piaSession->m_sessionDisconnectReason == 0)
                g_piaSession->m_sessionDisconnectReason = disconnectReason;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }

    if (transport::g_piaTransport->m_connectionError != 0)
    {
        g_piaSession->m_disconnectStatus = 3;
        if (g_piaSession->m_sessionDisconnectReason != 2)
            g_piaSession->m_sessionDisconnectReason = 8;
        return MakeExecuteResult(cExecuteKindFinished);
    }

    if (g_piaSession->m_matchmakeStatus == 2 &&
        g_piaMesh->CheckConnectionError() == static_cast<Result>(0x6c50))
    {
        const unsigned int disconnectReason = g_piaMesh->GetDisconnectReason();
        if (disconnectReason < 2 || (disconnectReason > 2 && disconnectReason < 10))
            g_piaSession->m_disconnectStatus = 2;
        if (g_piaSession->m_sessionDisconnectReason == 0)
            g_piaSession->m_sessionDisconnectReason = disconnectReason;
        return MakeExecuteResult(cExecuteKindFinished);
    }

    if (Session::s_debugSetting.m_matchmakeAnalysisEnabled != 0 && g_piaSession->IsHost())
    {
        IMatchmakeSession* matchmakeSession =
            g_piaSession->m_matchmakeSession[g_piaSession->m_currentSessionIndex];
        if (!Session::SessionJoiningApprovalCallback(0))
            matchmakeSession->GetSessionInfo()->IsOpened();
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

SessionStatusCheckJob::ExecuteResult SessionStatusCheckJob::CheckSessionStatus4JointSession()
{
    const unsigned int matchmakeStatus = g_piaSession->m_matchmakeStatus;
    const Result connectedStatus =
        g_piaSession->m_meshLayerController->CheckConnectedStatus();
    if (connectedStatus == static_cast<Result>(0xc406))
    {
        g_piaSession->m_disconnectStatus = 3;
        if (g_piaSession->m_sessionDisconnectReason != 2)
            g_piaSession->m_sessionDisconnectReason = 8;
        return MakeExecuteResult(cExecuteKindSuspend);
    }
    if (connectedStatus == static_cast<Result>(0xa471))
    {
        g_piaSession->m_disconnectStatus = 4;
        if (g_piaSession->m_sessionDisconnectReason != 2)
            g_piaSession->m_sessionDisconnectReason = 8;
        return MakeExecuteResult(cExecuteKindSuspend);
    }
    if (connectedStatus == static_cast<Result>(0xc420))
    {
        g_piaSession->m_disconnectStatus = 5;
        if (g_piaSession->m_sessionDisconnectReason != 2)
            g_piaSession->m_sessionDisconnectReason = 8;
        return MakeExecuteResult(cExecuteKindSuspend);
    }
    if (connectedStatus == static_cast<Result>(0xc421))
    {
        g_piaSession->m_disconnectStatus = 6;
        if (g_piaSession->m_sessionDisconnectReason != 2)
            g_piaSession->m_sessionDisconnectReason = 8;
        return MakeExecuteResult(cExecuteKindSuspend);
    }

    if (transport::g_piaTransport->m_connectionError != 0)
    {
        g_piaSession->m_disconnectStatus = 3;
        if (g_piaSession->m_sessionDisconnectReason != 2)
            g_piaSession->m_sessionDisconnectReason = 8;
        return MakeExecuteResult(cExecuteKindSuspend);
    }

    if (g_piaSession->m_disconnectStatus != 1)
        return MakeExecuteResult(cExecuteKindNextStep);

    const unsigned char sessionIndex = g_piaSession->m_currentSessionIndex;
    const unsigned char bufferSessionIndex = sessionIndex == 0 ? 1 : 0;
    const unsigned int currentSessionId = g_piaSession->m_sessionId[sessionIndex];
    const unsigned int bufferSessionId = g_piaSession->m_sessionId[bufferSessionIndex];

    if (matchmakeStatus != 2 && matchmakeStatus != 4)
    {
        if (matchmakeStatus == 3)
        {
            if (g_piaSession->m_jointSessionJob != 0 &&
                g_piaSession->m_jointSessionJob->m_sessionDisconnectDetected)
                return MakeExecuteResult(cExecuteKindNextStep);
            if (currentSessionId != 0 || bufferSessionId != 0)
                return MakeExecuteResult(cExecuteKindNextStep);
            g_piaSession->m_disconnectStatus = 3;
            if (g_piaSession->m_sessionDisconnectReason != 2)
                g_piaSession->m_sessionDisconnectReason = 8;
            return MakeExecuteResult(cExecuteKindSuspend);
        }
        if (connectedStatus != static_cast<Result>(0x8c56))
            return MakeExecuteResult(cExecuteKindNextStep);
        g_piaSession->m_disconnectStatus = 2;
        if (g_piaSession->m_sessionDisconnectReason != 2)
            g_piaSession->m_sessionDisconnectReason = 8;
        return MakeExecuteResult(cExecuteKindNextStep);
    }

    if (g_piaMesh->CheckConnectionError() != static_cast<Result>(0x6c50))
    {
        if (matchmakeStatus == 2)
        {
            if (currentSessionId == 0 ||
                g_piaSession->m_meshLayerController->IsJoinedUnknownSession(
                    currentSessionId, 0))
            {
                g_piaSession->m_disconnectStatus = 2;
                if (g_piaSession->m_sessionDisconnectReason != 2)
                    g_piaSession->m_sessionDisconnectReason = 8;
            }
            if (Session::s_debugSetting.m_matchmakeAnalysisEnabled == 0)
                return MakeExecuteResult(cExecuteKindNextStep);
            if (!g_piaSession->IsHost())
                return MakeExecuteResult(cExecuteKindNextStep);
            if (Session::SessionJoiningApprovalCallback(
                    g_piaSession->GetMatchmakeAnalysisToken()))
                return MakeExecuteResult(cExecuteKindNextStep);
            g_piaSession->m_matchmakeSession[sessionIndex]->GetSessionInfo()->IsOpened();
            return MakeExecuteResult(cExecuteKindNextStep);
        }

        if (matchmakeStatus != 4)
            return MakeExecuteResult(cExecuteKindNextStep);
        if (currentSessionId == 0 || bufferSessionId == 0 ||
            g_piaSession->m_meshLayerController->IsJoinedUnknownSession(
                currentSessionId, bufferSessionId))
        {
            g_piaSession->m_disconnectStatus = 2;
            if (g_piaSession->m_sessionDisconnectReason != 2)
                g_piaSession->m_sessionDisconnectReason = 8;
        }
        if (Session::s_debugSetting.m_matchmakeAnalysisEnabled == 0)
            return MakeExecuteResult(cExecuteKindNextStep);
        if (!g_piaSession->IsJointSessionHost())
            return MakeExecuteResult(cExecuteKindNextStep);
        if (Session::SessionJoiningApprovalCallback(
                g_piaSession->GetMatchmakeAnalysisToken()))
            return MakeExecuteResult(cExecuteKindNextStep);
        g_piaSession->m_matchmakeSession[bufferSessionIndex]->GetSessionInfo()->IsOpened();
        return MakeExecuteResult(cExecuteKindNextStep);
    }

    const unsigned int disconnectReason = g_piaMesh->GetDisconnectReason();
    ExecuteResult executeResult = MakeExecuteResult(cExecuteKindNextStep);
    if (disconnectReason < 2 || (disconnectReason > 2 && disconnectReason < 10))
    {
        g_piaSession->m_disconnectStatus = 2;
    }
    else if (disconnectReason != 2)
    {
        g_piaSession->m_disconnectStatus = 3;
        executeResult = MakeExecuteResult(cExecuteKindSuspend);
    }
    if (g_piaSession->m_sessionDisconnectReason == 0)
        g_piaSession->m_sessionDisconnectReason = disconnectReason;
    return executeResult;
}

void SessionStatusCheckJob::Cleanup()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();
    m_callContext.Reset();
    m_field54 = false;
    CancelCleanup();
    Reset(true);
}

}}}
