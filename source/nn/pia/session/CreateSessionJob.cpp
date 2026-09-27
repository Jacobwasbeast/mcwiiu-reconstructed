#include "nn/pia/session/CreateSessionJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/StationIdTable.hpp"

namespace nn { namespace pia { namespace session {
namespace {
unsigned int FailureForStatus(unsigned int status)
{
    if (status == 5) return 0xc406;
    if (status < 7) return 0xa471;
    if (status == 7) return 0xc420;
    if (status == 8) return 0xc421;
    return 0;
}
typedef unsigned int (CreateSessionJob::*Step)();
void SelectStep(common::StepSequenceJob::Step& slot, Step step)
{
    union { Step derived; unsigned int (common::StepSequenceJob::*base)(); } value;
    value.derived = step;
    slot.m_function = value.base;
}
}

CreateSessionJob::CreateSessionJob() : common::StepSequenceJob(), m_callerContext(0), m_callContext(), m_result(0)
{
}

CreateSessionJob::~CreateSessionJob()
{
    Cleanup();
}

Result CreateSessionJob::Startup(common::CallContext* callerContext, const CreateSessionSetting* setting)
{
    if (callerContext == 0 || setting == 0)
        return static_cast<Result>(0x10c07);
    m_callerContext = callerContext;
    const bool initiated = callerContext->InitiateCall();
    if (!initiated)
        return callerContext->m_result;
    Reset(true);
    SelectStep(m_currentStep, &CreateSessionJob::MeshStartup);
    m_currentStep.m_name = "CreateSessionJob::MeshStartup";
    return 0;
}

unsigned int CreateSessionJob::MeshStartup()
{
    if (g_piaSession == 0)
        return CompleteFailure();
    const unsigned int status = g_piaSession->GetStatus();
    const unsigned int failure = FailureForStatus(status);
    if (failure != 0)
    {
        m_result = failure;
        SelectStep(m_currentStep, &CreateSessionJob::CompleteFailure);
        m_currentStep.m_name = "CreateSessionJob__CompleteFailure";
        return cExecuteResultNextStep;
    }
    if (status < 5)
    {
        SelectStep(m_currentStep, &CreateSessionJob::CreateMesh);
        m_currentStep.m_name = "CreateSessionJob::CreateMesh";
        return cExecuteResultNextStep;
    }
    SelectStep(m_currentStep, &CreateSessionJob::CreateMesh);
    m_currentStep.m_name = "CreateSessionJob::CreateMesh";
    return cExecuteResultNextStep;
}

unsigned int CreateSessionJob::CreateMesh()
{
    if (g_piaMesh == 0)
        return CompleteFailure();
    const Result result = g_piaMesh->CreateMesh(&m_callContext);
    if (result != 0)
    {
        m_result = result;
        SelectStep(m_currentStep, &CreateSessionJob::CompleteFailure);
        return cExecuteResultNextStep;
    }
    g_piaMesh->SetSkipReport(true);
    SelectStep(m_currentStep, &CreateSessionJob::WaitCreateMesh);
    m_currentStep.m_name = "CreateSessionJob::WaitCreateMesh";
    return cExecuteResultNextStep;
}

unsigned int CreateSessionJob::WaitCreateMesh()
{
    if (g_piaSession == 0)
        return CompleteFailure();
    const unsigned int status = g_piaSession->GetStatus();
    const unsigned int failure = FailureForStatus(status);
    if (failure != 0)
    {
        m_result = failure;
        return CompleteFailure();
    }
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return cExecuteResultWait;
    if (m_callContext.m_state == common::CallContext::cStateFailure)
    {
        m_result = m_callContext.m_result;
        return CompleteFailure();
    }
    if (m_callContext.m_state != common::CallContext::cStateSuccess &&
        m_callContext.m_state != common::CallContext::cStateCancelled)
        return cExecuteResultWait;
    if (m_callContext.m_state == common::CallContext::cStateCancelled)
    {
        m_result = 0x10c08;
        return CompleteFailure();
    }
    if (transport::g_piaTransport != 0 && transport::g_piaTransport->m_stationIdTable != 0)
    {
        if (g_piaSession->UpdateStationIdToBecomeHost() != 0)
        {
            g_piaSession->m_currentSessionIndex = g_piaSession->m_currentSessionIndex;
            transport::g_piaTransport->m_stationIdTable->SetAvailableEntryNum(0);
            return CompleteProcess();
        }
    }
    m_result = 0x10c08;
    return CompleteFailure();
}

unsigned int CreateSessionJob::CompleteFailure()
{
    if (m_result == 0x10c08 || m_result == 0xc406)
        if (g_piaSession != 0) g_piaSession->ChangeStatusToDisconnectedNetwork();
    if (m_callerContext != 0)
    {
        m_callerContext->SignalFailure(static_cast<Result>(m_result));
        m_callerContext = 0;
    }
    Cleanup();
    return cExecuteResultFinished;
}

unsigned int CreateSessionJob::CompleteProcess()
{
    if (m_callerContext != 0)
    {
        m_callerContext->SignalSuccess(0);
        m_callerContext = 0;
    }
    if (g_piaMesh != 0)
    {
        g_piaMesh->SetSkipReport(m_result != 0);
        g_piaMesh->SendReport(0);
    }
    return cExecuteResultFinished;
}

void CreateSessionJob::Cleanup()
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
    m_result = 0;
}

void CreateSessionJob::Trace(unsigned long long) const
{
}

void CreateSessionJob::SetMatchmakeStatus(Session::MatchmakeStatus status)
{
    if (g_piaSession != 0) g_piaSession->m_matchmakeStatus = status;
}

void CreateSessionJob::SetConnectionStatus(Session::ConnectionStatus status)
{
    if (g_piaSession != 0) g_piaSession->m_disconnectStatus = status;
}

}}}
