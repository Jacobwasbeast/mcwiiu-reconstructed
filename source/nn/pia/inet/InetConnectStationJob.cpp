#include "nn/pia/inet/InetConnectStationJob.hpp"
#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/inet/Api.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/inet/NexNatRelay.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"

namespace nn { namespace pia { namespace inet {

namespace
{
typedef common::Job::ExecuteResult (InetConnectStationJob::*Step)();

enum
{
    cTraceStation = 0x800,
    cTraceStationLocation = 0x1000,
    cTraceSession = 0x08000000
};

enum
{
    cResultInvalidState = 0x10c08,
    cResultPortDetectionFailed = 0x6434,
    cResultNatTraversalFailed = 0x643f,
    cResultNatTraversalFailedEimToEim = 0x6442,
    cResultNatTraversalFailedEimToEdm = 0x6443,
    cResultNatTraversalFailedEdmToEim = 0x6444,
    cResultNatTraversalFailedEdmToEdm = 0x6445,
    cResultNatTraversalTimeout = 0x644a,
    cResultNatTraversalFailedSameNatEimToEim = 0x6467,
    cResultNatTraversalFailedSameNatEimToEdm = 0x6468,
    cResultNatTraversalFailedSameNatEdmToEim = 0x6469,
    cResultNatTraversalFailedSameNatEdmToEdm = 0x646a
};

enum
{
    cNatMappingEndpointIndependent = 1,
    cNatMappingEndpointDependent = 2
};

enum
{
    cHostStationIndex = 0xfe,
    cNatTraversalStartTimeoutMsec = 15000,
    cRelayTraversalTimeoutExtensionMsec = 5000,
    cWaitIntervalMsec = 100,
    cEmulatedResultKinds = 10
};

inline void SelectStep(common::StepSequenceJob::Step& slot, Step function, const char* name)
{
    const common::StepSequenceJob::Step step(static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}

inline void CountRetry(unsigned char& counter)
{
    if (counter == 0xff)
        counter = 1;
    else
        ++counter;
}

inline void SetDeadline(common::Time& deadline, int timeoutMsec)
{
    const common::TimeSpan span = common::TimeSpan::FromMSec(timeoutMsec);
    common::Time now;
    now.SetNow();
    deadline.m_tick = now.m_tick + span.m_tick;
}

Result EmulatedNatTraversalResult()
{
    switch (common::g_piaJobScheduler->m_dispatchTime.m_tick % cEmulatedResultKinds)
    {
    case 9:
        return cResultNatTraversalTimeout;
    case 8:
        return cResultNatTraversalFailedSameNatEdmToEdm;
    case 7:
        return cResultNatTraversalFailedEdmToEdm;
    case 6:
        return cResultNatTraversalFailedSameNatEdmToEim;
    case 5:
        return cResultNatTraversalFailedEdmToEim;
    case 4:
        return cResultNatTraversalFailedSameNatEimToEdm;
    case 3:
        return cResultNatTraversalFailedEimToEdm;
    case 2:
        return cResultNatTraversalFailedSameNatEimToEim;
    case 1:
        return cResultNatTraversalFailedEimToEim;
    case 0:
        return cResultNatTraversalFailed;
    default:
        return cResultNatTraversalFailed;
    }
}
}

InetConnectStationJob::InetConnectStationJob()
    : m_stationLocation(), m_stationConnectionInfo(), m_natTraversalDeadline(), m_natTraversalContext()
{
    m_traceFlag = cTraceStation;
}

InetConnectStationJob::~InetConnectStationJob()
{
    m_natTraversalContext.SignalCancel();
    m_natTraversalContext.Reset();
}

Result InetConnectStationJob::StartupImpl(common::CallContext* context, transport::Station* station,
    const transport::StationConnectionInfo& info, bool secure)
{
    m_station = station;
    m_secure = secure;
    info.Trace(cTraceStation);
    if (IsPublic(info.m_stationLocation))
    {
        m_stationConnectionInfo = info;
    }
    else
    {
        m_stationConnectionInfo.m_stationLocation.SetStationLocation(info.m_localStationLocation);
        m_stationConnectionInfo.m_localStationLocation.SetStationLocation(info.m_stationLocation);
    }
    if (!m_stationConnectionInfo.m_stationLocation.m_field24)
        m_stationConnectionInfo.m_stationLocation.m_field24 = m_stationConnectionInfo.m_localStationLocation.m_field24;
    if (!m_stationConnectionInfo.m_stationLocation.m_field23)
        m_stationConnectionInfo.m_stationLocation.m_field23 = m_stationConnectionInfo.m_localStationLocation.m_field23;
    if (!m_stationConnectionInfo.m_stationLocation.m_field25)
        m_stationConnectionInfo.m_stationLocation.m_field25 = m_stationConnectionInfo.m_localStationLocation.m_field25;
    m_stationConnectionInfo.Trace(cTraceStation);
    m_station->GetStationAddress().Trace(cTraceStation);
    session::g_piaMesh->TraceStationConnectionInfoList(cTraceStationLocation);
    m_stationConnectionInfo.m_localStationLocation.m_field1c = m_stationConnectionInfo.m_stationLocation.m_field1c;

    if (!NexFacade::GetInstance()->m_natSessionActive)
        return cResultInvalidState;

    transport::StationConnectionInfoTable* table = transport::g_stationConnectionInfoTable;
    table->GetLocalStationConnectionInfo().Trace(cTraceStation);
    if (m_stationConnectionInfo.m_stationLocation.m_stationAddress.m_inetAddress.m_address ==
        table->GetLocalStationConnectionInfo().m_stationLocation.m_stationAddress.m_inetAddress.m_address)
    {
        if (IsGlobal(table->GetLocalStationConnectionInfo().m_stationLocation))
            return cResultInvalidState;
        m_stationLocation = m_stationConnectionInfo.m_localStationLocation;
    }
    else
    {
        m_stationLocation = m_stationConnectionInfo.m_stationLocation;
    }
    if (m_secure)
        m_stationLocation.SetStationAddress(m_station->m_stationAddress);

    m_natTraversalRetryCount = 0;
    m_natTraversalStartRetryCount = 0;
    if (m_station->m_field10 == cHostStationIndex)
    {
        transport::g_stationConnectionInfoTable->m_hostStationConnectionInfo = m_stationConnectionInfo;
        transport::g_stationConnectionInfoTable->GetHostStationConnectionInfo().Trace(cTraceStation);
        transport::g_stationConnectionInfoTable->m_hostStationAddress = m_stationLocation.m_stationAddress;
        transport::g_stationConnectionInfoTable->GetHostStationAddress().Trace(cTraceStation);
        if (IsEdmMapping(m_stationLocation))
        {
            m_natTraversalStartRetryCount = 1;
            if (!IsEdmMapping(table->GetLocalStationConnectionInfo().m_stationLocation))
                m_natTraversalRetryCount = 1;
        }
        if (NexFacade::GetInstance()->IsBeginMonitoringDataAvailable())
        {
            common::g_sessionBeginMonitoringContent.m_metric148 = 0;
            common::g_sessionBeginMonitoringContent.m_metric149 = 0;
        }
    }

    transport::StationConnectionInfo connectionInfo;
    connectionInfo.m_stationLocation.SetStationLocation(m_stationLocation);
    const Result result = ConnectStationJob::StartupImpl(context, m_station, connectionInfo, secure);
    if (result != 0)
        return result;
    SelectStep(m_currentStep, &InetConnectStationJob::TestCurrentAddress, "InetConnectStationJob::TestCurrentAddress");
    return 0;
}

void InetConnectStationJob::CleanupImpl()
{
    m_natTraversalContext.SignalCancel();
    m_natTraversalContext.Reset();
    if (m_stationLocation.m_field1c)
    {
        session::Session::DisconnectionEventListener listener = transport::g_piaTransport->m_disconnectionEventListener;
        bool clearMonitoringData = true;
        if (listener && m_stationLocation.m_principalId)
        {
            transport::StationIdTable::Entry entry;
            if (transport::g_piaTransport->m_stationIdTable->FindByUniqueId(&entry, m_stationLocation.m_principalId) == 0 &&
                listener(&entry) == 1)
                clearMonitoringData = false;
        }
        NexFacade::GetInstance()->NotifyLeave(m_stationLocation, clearMonitoringData, false);
    }
}

bool InetConnectStationJob::CheckErrorOccurredImpl()
{
    session::Session* session = session::g_piaSession;
    if (session == 0)
        return false;
    session::MeshLayerController* meshLayerController = session->m_meshLayerController;
    if (meshLayerController == 0)
        return false;
    unsigned int sessionId = session->m_sessionId[session->m_currentSessionIndex == 0 ? 1 : 0];
    if (sessionId == 0)
        sessionId = session->m_sessionId[session->m_currentSessionIndex];
    unsigned long long principalId = m_stationConnectionInfo.m_stationLocation.m_principalId;
    if (principalId == 0)
    {
        principalId = m_stationConnectionInfo.m_localStationLocation.m_principalId;
        if (principalId == 0)
            return false;
    }
    if (meshLayerController->IsParticipated(sessionId, principalId))
        return false;
    meshLayerController->Trace(cTraceSession);
    return true;
}

void InetConnectStationJob::Trace(unsigned long long) const
{
}

InetConnectStationJob::ExecuteResult InetConnectStationJob::TestCurrentAddress()
{
    if (m_station->m_field10 == cHostStationIndex)
    {
        m_stationLocation.Trace(cTraceStationLocation);
        SelectStep(m_currentStep, &InetConnectStationJob::PrepareNatTraversal, "InetConnectStationJob::PrepareNatTraversal");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (IsBehindNat(m_stationLocation) && m_secure)
    {
        m_stationLocation.Trace(cTraceStationLocation);
        NexFacade::GetInstance()->ResetProbeStatus(m_stationLocation, m_secure);
        SelectStep(m_currentStep, &InetConnectStationJob::ResolveCurrentAddress, "InetConnectStationJob::ResolveCurrentAddress");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (IsPublic(m_stationLocation) && m_secure)
    {
        m_stationLocation.Trace(cTraceStationLocation);
        NexFacade::GetInstance()->ResetProbeStatus(m_stationLocation, m_secure);
        SelectStep(m_currentStep, &InetConnectStationJob::ResolveCurrentAddress, "InetConnectStationJob::ResolveCurrentAddress");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    SelectStep(m_currentStep, &InetConnectStationJob::PrepareNatTraversal, "InetConnectStationJob::PrepareNatTraversal");
    return MakeExecuteResult(cExecuteKindContinue);
}

InetConnectStationJob::ExecuteResult InetConnectStationJob::PrepareNatTraversal()
{
    int timeoutMsec = NexFacade::GetInstance()->m_natTraversalTimeout;
    if (m_secure)
    {
        if (IsEdmMapping(transport::g_stationConnectionInfoTable->GetLocalStationConnectionInfo().m_stationLocation))
            timeoutMsec += NexFacade::GetInstance()->m_natTraversalTimeoutEdmExtension;
        if (IsEdmMapping(m_stationConnectionInfo.m_stationLocation))
            timeoutMsec += NexFacade::GetInstance()->m_natTraversalTimeoutEdmExtension;
    }
    if (!NexFacade::GetInstance()->PrepareNatTraversal(m_stationLocation, m_stationConnectionInfo, &m_natTraversalContext, m_secure,
                                     false, static_cast<StationIndex>(m_station->m_field10)))
    {
        SelectStep(m_currentStep, &InetConnectStationJob::ResolveCurrentAddress, "InetConnectStationJob::ResolveCurrentAddress");
    }
    else if (!m_secure)
    {
        SetDeadline(m_natTraversalDeadline, cNatTraversalStartTimeoutMsec);
        SelectStep(m_currentStep, &InetConnectStationJob::WaitForNatTraversalStarted, "InetConnectStationJob::WaitForNatTraversalStarted");
    }
    else
    {
        SetDeadline(m_natTraversalDeadline, timeoutMsec);
        SelectStep(m_currentStep, &InetConnectStationJob::WaitForNatTraversalCompleted, "InetConnectStationJob::WaitForNatTraversalCompleted");
    }
    return MakeExecuteResult(cExecuteKindContinue);
}

InetConnectStationJob::ExecuteResult InetConnectStationJob::WaitForNatTraversalStarted()
{
    switch (m_natTraversalContext.m_state)
    {
    case common::CallContext::cStateInProgress:
    {
            int timeoutMsec = NexFacade::GetInstance()->m_natTraversalTimeout;
        if (IsEdmMapping(transport::g_stationConnectionInfoTable->GetLocalStationConnectionInfo().m_stationLocation))
            timeoutMsec += NexFacade::GetInstance()->m_natTraversalTimeoutEdmExtension;
        if (m_station->m_field10 == cHostStationIndex || IsEdmMapping(m_stationConnectionInfo.m_stationLocation))
            timeoutMsec += NexFacade::GetInstance()->m_natTraversalTimeoutEdmExtension;
        SetDeadline(m_natTraversalDeadline, timeoutMsec);
        SelectStep(m_currentStep, &InetConnectStationJob::WaitForNatTraversalCompleted, "InetConnectStationJob::WaitForNatTraversalCompleted");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    case common::CallContext::cStateSuccess:
    case common::CallContext::cStateFailure:
    case common::CallContext::cStateCancelled:
        SelectStep(m_currentStep, &InetConnectStationJob::WaitForNatTraversalCompleted, "InetConnectStationJob::WaitForNatTraversalCompleted");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (m_station->m_field10 == cHostStationIndex)
    {
        common::Time now;
        now.SetNow();
        if (m_natTraversalDeadline < now)
        {
            if (m_natTraversalStartRetryCount <= 0)
            {
                m_stationLocation.Trace(cTraceStation);
                m_context->SignalFailure(cResultNatTraversalTimeout);
                NexFacade::GetInstance()->ReportNatTraversalResult(m_stationLocation, cResultNatTraversalTimeout, false);
                NexFacade::GetInstance()->NotifyLeave(m_stationLocation, true, true);
                SelectStep(m_currentStep, &ConnectStationJob::ConnectionFailed, "ConnectStationJob::ConnectionFailed");
                return MakeExecuteResult(cExecuteKindContinue);
            }
            --m_natTraversalStartRetryCount;
            if (NexFacade::GetInstance()->IsBeginMonitoringDataAvailable())
                common::g_sessionBeginMonitoringContent.m_metric148 = static_cast<unsigned char>(1 - m_natTraversalStartRetryCount);
            SetDeadline(m_natTraversalDeadline, cNatTraversalStartTimeoutMsec);
            return MakeExecuteResult(cExecuteKindWait, cWaitIntervalMsec);
        }
    }
    return MakeExecuteResult(cExecuteKindWait, cWaitIntervalMsec);
}

InetConnectStationJob::ExecuteResult InetConnectStationJob::WaitForNatTraversalCompleted()
{
    if (NexFacade::GetInstance()->IsTraversed(m_stationLocation.m_field1c))
    {
        SelectStep(m_currentStep, &InetConnectStationJob::ResolveCurrentAddress, "InetConnectStationJob::ResolveCurrentAddress");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    common::Time now;
    now.SetNow();
    if (m_natTraversalDeadline < now || m_natTraversalContext.m_state != common::CallContext::cStateInProgress)
    {
        bool sameNat = false;
        if (m_stationConnectionInfo.m_stationLocation.m_stationAddress.m_inetAddress.m_address ==
            transport::g_stationConnectionInfoTable->GetLocalStationConnectionInfo().m_stationLocation.m_stationAddress.m_inetAddress.m_address)
            sameNat = true;
        Result result = m_natTraversalContext.m_result;
        if (m_natTraversalContext.m_state == common::CallContext::cStateFailure && result == cResultNatTraversalTimeout)
        {
            NexFacade::GetInstance()->ReportNatTraversalResult(m_stationLocation, cResultNatTraversalTimeout, false);
        }
        else if (m_natTraversalContext.m_state != common::CallContext::cStateFailure || result != cResultPortDetectionFailed)
        {
            if (NexFacade::GetInstance()->GetNatPropertyMapping() == cNatMappingEndpointDependent && IsEdmMapping(m_stationConnectionInfo.m_stationLocation))
                result = sameNat ? cResultNatTraversalFailedSameNatEdmToEdm : cResultNatTraversalFailedEdmToEdm;
            else if (NexFacade::GetInstance()->GetNatPropertyMapping() == cNatMappingEndpointDependent && IsEimMapping(m_stationConnectionInfo.m_stationLocation))
                result = sameNat ? cResultNatTraversalFailedSameNatEdmToEim : cResultNatTraversalFailedEdmToEim;
            else if (NexFacade::GetInstance()->GetNatPropertyMapping() == cNatMappingEndpointIndependent && IsEdmMapping(m_stationConnectionInfo.m_stationLocation))
                result = sameNat ? cResultNatTraversalFailedSameNatEimToEdm : cResultNatTraversalFailedEimToEdm;
            else if (NexFacade::GetInstance()->GetNatPropertyMapping() == cNatMappingEndpointIndependent && IsEimMapping(m_stationConnectionInfo.m_stationLocation))
                result = sameNat ? cResultNatTraversalFailedSameNatEimToEim : cResultNatTraversalFailedEimToEim;
            else
                result = cResultNatTraversalFailed;
        }
        NexFacade::GetInstance()->NotifyLeave(m_stationLocation, true, true);
        if (result == cResultNatTraversalFailedSameNatEimToEim)
        {
            common::StationAddress address;
            address.SetInetAddress(common::InetAddress(m_stationConnectionInfo.m_stationLocation.m_stationAddress.m_inetAddress.m_address,
                                                       m_stationLocation.m_stationAddress.m_inetAddress.m_port));
            transport::StationLocation location(m_stationConnectionInfo.m_stationLocation);
            location.SetStationAddress(address);
            NexFacade::GetInstance()->NotifyLeave(location, true, true);
        }

        if (m_natTraversalRetryCount > 0)
        {
            --m_natTraversalRetryCount;
            if (NexFacade::GetInstance()->IsBeginMonitoringDataAvailable())
            {
                common::g_sessionBeginMonitoringContent.m_metric149 = static_cast<unsigned char>(1 - m_natTraversalRetryCount);
                CountRetry(common::g_sessionBeginMonitoringContent.m_metric119);
            }
            m_natTraversalContext.SignalCancel();
            m_natTraversalContext.Reset();
            SelectStep(m_currentStep, &InetConnectStationJob::PrepareNatTraversal, "InetConnectStationJob::PrepareNatTraversal");
            return MakeExecuteResult(cExecuteKindWait, cWaitIntervalMsec);
        }
        if (!m_secure && NexFacade::GetInstance()->IsRelayServiceEnabled() && NexFacade::GetInstance()->m_natRelay->IsRelayServerAvailable())
        {
            m_natTraversalContext.SignalCancel();
            m_natTraversalContext.Reset();
            SelectStep(m_currentStep, &InetConnectStationJob::PrepareNatTraversalByRelay, "InetConnectStationJob::PrepareNatTraversalByRelay");
            return MakeExecuteResult(cExecuteKindContinue);
        }

        if (GetDebugSetting()->natTraversalResult != DebugSetting::cNatTraversalResultNotEmulated)
        {
            if (GetDebugSetting()->natTraversalResult != 0)
                result = GetDebugSetting()->natTraversalResult;
            else
                result = EmulatedNatTraversalResult();
        }
        if (result != 0 && m_context)
            m_context->SignalFailure(result);
        NexFacade::GetInstance()->ReportNatTraversalResult(m_stationLocation, result, !m_secure);
        SelectStep(m_currentStep, &ConnectStationJob::ConnectionFailed, "ConnectStationJob::ConnectionFailed");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindWait, cWaitIntervalMsec);
}

InetConnectStationJob::ExecuteResult InetConnectStationJob::PrepareNatTraversalByRelay()
{
    m_stationLocation = m_stationConnectionInfo.m_stationLocation;
    if (!NexFacade::GetInstance()->PrepareNatTraversal(m_stationLocation, m_stationConnectionInfo, &m_natTraversalContext,
                                                      m_secure, true, static_cast<StationIndex>(m_station->m_field10)))
    {
        SelectStep(m_currentStep, &InetConnectStationJob::ResolveCurrentAddress, "InetConnectStationJob::ResolveCurrentAddress");
    }
    else
    {
        SetDeadline(m_natTraversalDeadline, cNatTraversalStartTimeoutMsec);
        SelectStep(m_currentStep, &InetConnectStationJob::WaitForNatTraversalStartedByRelay, "InetConnectStationJob::WaitForNatTraversalStartedByRelay");
    }
    return MakeExecuteResult(cExecuteKindContinue);
}

InetConnectStationJob::ExecuteResult InetConnectStationJob::WaitForNatTraversalStartedByRelay()
{
    switch (m_natTraversalContext.m_state)
    {
    case common::CallContext::cStateInProgress:
        SetDeadline(m_natTraversalDeadline, NexFacade::GetInstance()->m_natTraversalTimeout + cRelayTraversalTimeoutExtensionMsec);
        SelectStep(m_currentStep, &InetConnectStationJob::WaitForNatTraversalCompletedByRelay, "InetConnectStationJob::WaitForNatTraversalCompletedByRelay");
        return MakeExecuteResult(cExecuteKindContinue);
    case common::CallContext::cStateSuccess:
    case common::CallContext::cStateFailure:
    case common::CallContext::cStateCancelled:
        SelectStep(m_currentStep, &InetConnectStationJob::WaitForNatTraversalCompletedByRelay, "InetConnectStationJob::WaitForNatTraversalCompletedByRelay");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (m_station->m_field10 == cHostStationIndex)
    {
        common::Time now;
        now.SetNow();
        if (m_natTraversalDeadline < now)
        {
            m_stationLocation.Trace(cTraceStation);
            m_context->SignalFailure(cResultNatTraversalTimeout);
            NexFacade::GetInstance()->ReportNatTraversalResult(m_stationLocation, cResultNatTraversalTimeout, false);
            NexFacade::GetInstance()->NotifyLeave(m_stationLocation, true, true);
            SelectStep(m_currentStep, &ConnectStationJob::ConnectionFailed, "ConnectStationJob::ConnectionFailed");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindWait, cWaitIntervalMsec);
}

InetConnectStationJob::ExecuteResult InetConnectStationJob::WaitForNatTraversalCompletedByRelay()
{
    if (NexFacade::GetInstance()->IsTraversed(m_stationLocation.m_field1c))
    {
        SelectStep(m_currentStep, &InetConnectStationJob::ResolveCurrentAddress, "InetConnectStationJob::ResolveCurrentAddress");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    common::Time now;
    now.SetNow();
    if (now <= m_natTraversalDeadline && m_natTraversalContext.m_state == common::CallContext::cStateInProgress)
        return MakeExecuteResult(cExecuteKindWait, cWaitIntervalMsec);

    bool sameNat = false;
    if (m_stationConnectionInfo.m_stationLocation.m_stationAddress.m_inetAddress.m_address ==
        transport::g_stationConnectionInfoTable->GetLocalStationConnectionInfo().m_stationLocation.m_stationAddress.m_inetAddress.m_address)
        sameNat = true;
    Result result = m_natTraversalContext.m_result;
    if (m_natTraversalContext.m_state == common::CallContext::cStateFailure && result == cResultNatTraversalTimeout)
    {
        NexFacade::GetInstance()->ReportNatTraversalResult(m_stationLocation, cResultNatTraversalTimeout, false);
    }
    else if (m_natTraversalContext.m_state != common::CallContext::cStateFailure || result != cResultPortDetectionFailed)
    {
        if (NexFacade::GetInstance()->GetNatPropertyMapping() == cNatMappingEndpointDependent && IsEdmMapping(m_stationConnectionInfo.m_stationLocation))
            result = sameNat ? cResultNatTraversalFailedSameNatEdmToEdm : cResultNatTraversalFailedEdmToEdm;
        else if (NexFacade::GetInstance()->GetNatPropertyMapping() == cNatMappingEndpointDependent && IsEimMapping(m_stationConnectionInfo.m_stationLocation))
            result = sameNat ? cResultNatTraversalFailedSameNatEdmToEim : cResultNatTraversalFailedEdmToEim;
        else if (NexFacade::GetInstance()->GetNatPropertyMapping() == cNatMappingEndpointIndependent && IsEdmMapping(m_stationConnectionInfo.m_stationLocation))
            result = sameNat ? cResultNatTraversalFailedSameNatEimToEdm : cResultNatTraversalFailedEimToEdm;
        else if (NexFacade::GetInstance()->GetNatPropertyMapping() == cNatMappingEndpointIndependent && IsEimMapping(m_stationConnectionInfo.m_stationLocation))
            result = sameNat ? cResultNatTraversalFailedSameNatEimToEim : cResultNatTraversalFailedEimToEim;
        else
            result = cResultNatTraversalFailed;
    }

    if (GetDebugSetting()->natTraversalResult != DebugSetting::cNatTraversalResultNotEmulated)
    {
        if (GetDebugSetting()->natTraversalResult != 0)
            result = GetDebugSetting()->natTraversalResult;
        else
            result = EmulatedNatTraversalResult();
    }
    if (result != 0 && m_context)
        m_context->SignalFailure(result);
    NexFacade::GetInstance()->ReportNatTraversalResult(m_stationLocation, result, !m_secure);
    NexFacade::GetInstance()->NotifyLeave(m_stationLocation, true, true);
    SelectStep(m_currentStep, &ConnectStationJob::ConnectionFailed, "ConnectStationJob::ConnectionFailed");
    return MakeExecuteResult(cExecuteKindContinue);
}

InetConnectStationJob::ExecuteResult InetConnectStationJob::ResolveCurrentAddress()
{
    if (NexFacade::GetInstance()->CheckLatestStationLocation(m_stationLocation) && m_station->m_field10 == cHostStationIndex)
    {
        transport::g_stationConnectionInfoTable->m_hostStationAddress = m_stationLocation.m_stationAddress;
        transport::g_stationConnectionInfoTable->GetHostStationAddress().Trace(cTraceStation);
    }
    SelectStep(m_currentStep, &InetConnectStationJob::TryCurrentAddress, "InetConnectStationJob::TryCurrentAddress");
    return MakeExecuteResult(cExecuteKindContinue);
}

InetConnectStationJob::ExecuteResult InetConnectStationJob::TryCurrentAddress()
{
    m_station->m_stationAddress = m_stationLocation.m_stationAddress;
    m_station->Trace(cTraceStationLocation);
    NexFacade::GetInstance()->ReportNatTraversalResult(m_stationLocation, 0, !m_secure);
    NexFacade::GetInstance()->ResetProbeStatus(m_stationLocation, m_secure);
    SelectStep(m_currentStep, &ConnectStationJob::SendConnectionRequest, "ConnectStationJob::SendConnectionRequest");
    return MakeExecuteResult(cExecuteKindContinue);
}

}}}
