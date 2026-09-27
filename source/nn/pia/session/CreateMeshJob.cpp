#include "nn/pia/session/CreateMeshJob.hpp"

#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"

namespace nn { namespace pia { namespace session {

namespace
{
typedef unsigned int (CreateMeshJob::*CreateStep)();

void SelectStep(common::StepSequenceJob::Step& slot, CreateStep step)
{
    union
    {
        CreateStep derived;
        unsigned int (common::StepSequenceJob::*base)();
    } value;
    value.derived = step;
    slot.m_function = value.base;
}
}

CreateMeshJob::CreateMeshJob()
    : common::StepSequenceJob(), m_callerContext(0)
{
}

CreateMeshJob::~CreateMeshJob()
{
}

void CreateMeshJob::SetHostInfoToMonitoringData()
{
    SetHostInfoToMonitoringDataImpl();
}

void CreateMeshJob::Startup(common::CallContext* callerContext)
{
    if (callerContext != 0)
    {
        m_callerContext = callerContext;
        callerContext->InitiateCall();
    }

    const unsigned int result = StartupImpl();
    if (result == cExecuteResultContinue && g_piaMesh != 0)
    {
        g_piaMesh->CleanupMonitoringBeginData();
        SetHostInfoToMonitoringData();
    }
}

unsigned int CreateMeshJob::SetupLocalPlayerInfo()
{
    if (g_piaMesh == 0)
        return CompleteFailure();
    g_piaMesh->SetupLocalPlayerInfo();
    SetNextStepAtSetupLocalPlayerInfoImpl();
    return cExecuteResultEntryForeground;
}

void CreateMeshJob::Cleanup()
{
    if (IsRunning() && IsBackground())
    {
        m_cancelRequested = true;
        do
        {
            WaitForCompletion(2);
        } while (IsRunning());
    }
    CancelCleanup();
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
    }
    CleanupImpl();
}

unsigned int CreateMeshJob::CompleteFailure()
{
    if (m_callerContext != 0)
    {
        m_callerContext->SignalFailure(static_cast<Result>(0x6c65));
        m_callerContext = 0;
    }
    Cleanup();
    return cExecuteResultFinished;
}

unsigned int CreateMeshJob::SetupLocalStation()
{
    if (g_piaMesh == 0 || transport::g_piaStationManager == 0)
        return CompleteFailure();
    g_piaMesh->SetupProtocols();
    transport::Station* station = transport::g_piaStationManager->CreateLocalStation();
    if (station == 0)
        return CompleteFailure();
    station->Startup();
    g_piaMesh->SetMeshState(Mesh::cStateStarted);
    if (m_callerContext != 0)
    {
        m_callerContext->SignalSuccess(0);
        m_callerContext = 0;
    }
    Cleanup();
    return cExecuteResultFinished;
}

void CreateMeshJob::Trace(unsigned long long) const
{
}

unsigned int CreateMeshJob::StartupImpl()
{
    Reset(true);
    m_state = 0xffff;
    m_currentStep.m_name = "CreateMeshJob::SetupLocalPlayerInfo";
    SetNextStepAtSetupLocalPlayerInfoImpl();
    return cExecuteResultContinue;
}

void CreateMeshJob::SetNextStepAtSetupLocalPlayerInfoImpl()
{
    SelectStep(m_currentStep, &CreateMeshJob::SetupLocalStation);
    m_currentStep.m_name = "CreateMeshJob::SetupLocalStation";
    m_state = 0xffff;
}

void CreateMeshJob::CleanupImpl()
{
}

void CreateMeshJob::SetHostInfoToMonitoringDataImpl()
{
}

}}}
