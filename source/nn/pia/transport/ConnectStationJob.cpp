#include "nn/pia/transport/ConnectStationJob.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/transport/ResendingMessageManager.hpp"
#include "nn/pia/transport/StationProtocol.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/Transport.hpp"

#include <cstring>

namespace nn { namespace pia { namespace transport {

ConnectStationJob::ConnectStationJob()
    : m_connectionDeadline(), m_timeoutMsec(10000), m_requestAckId(0),
      m_context(0), m_station(0), m_connectionRejection(0)
{
}

ConnectStationJob::~ConnectStationJob()
{
}

int ConnectStationJob::Startup(common::CallContext* context, Station* station,
    const StationConnectionInfo& info, bool secure, int timeoutMsec)
{
    m_timeoutMsec = timeoutMsec;
    return StartupImpl(context, station, info, secure);
}

int ConnectStationJob::StartupImpl(common::CallContext* context, Station* station,
    const StationConnectionInfo& info, bool secure)
{
    if (!station)
        return 0x10c07;
    station->Trace(0x08000000);
    if (m_station)
        m_station->Trace(0x08000000);
    if (GetState() != cStateNotEntried && GetState() != cStateCompleted)
    {
        Trace(0x8000);
        return 0x10c08;
    }
    if (!context)
        return 0x10c07;
    m_context = context;
    context->InitiateCall();
    m_station = station;
    station->m_stationAddress = info.m_stationLocation.m_stationAddress;
    m_secure = secure;
    Reset(true);
    m_station->m_field56 = 1;
    m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
        &ConnectStationJob::SendConnectionRequest);
    m_currentStep.m_name = "ConnectStationJob::SendConnectionRequest";
    return 0;
}

void ConnectStationJob::Cleanup()
{
    CleanupImpl();
    m_station = 0;
    if (m_requestAckId)
    {
        if (g_piaResendingMessageManager)
            g_piaResendingMessageManager->StopResending(m_requestAckId);
        m_requestAckId = 0;
    }
    m_connectionDeadline.m_tick = 0;
    if (m_context)
    {
        if (static_cast<unsigned int>(m_context->m_state) - 2 > 2)
            m_context->SignalCancel();
        m_context = 0;
    }
    if (IsRunning())
        Reset(false);
}

void ConnectStationJob::Trace(unsigned long long) const
{
}

void ConnectStationJob::CleanupImpl()
{
}

bool ConnectStationJob::CheckErrorOccurredImpl()
{
    return false;
}

bool ConnectStationJob::CheckErrorOccurred()
{
    return CheckErrorOccurredImpl();
}

bool ConnectStationJob::CheckContextCallCancelled()
{
    if (!m_context || !m_context->m_cancelRequested)
        return false;
    if (static_cast<unsigned int>(m_context->m_state) - 2 > 2)
        m_context->SignalCancel();
    m_context = 0;
    return true;
}

bool ConnectStationJob::CheckTimeout()
{
    if (!(m_connectionDeadline <= common::g_piaJobScheduler->m_dispatchTime))
        return false;
    if (m_context)
    {
        if (static_cast<unsigned int>(m_context->m_state) - 2 > 2)
            m_context->SignalFailure(0x6c0e);
        m_context = 0;
    }
    return true;
}

unsigned int ConnectStationJob::SendConnectionRequest()
{
    if (CheckContextCallCancelled())
    {
        if (m_station && m_station->m_field20 == 4)
            m_station->m_field20 = 6;
        return cExecuteResultFinished;
    }
    if (!m_station->m_field54)
    {
        common::Time now;
        now.SetNow();
        m_station->m_field54 = static_cast<unsigned char>(now.m_tick % 0xfe + 2);
    }
    if (!CheckErrorOccurred() && SendConnectionRequestMessage())
    {
        if (m_station->m_field10 > 0x1f)
        {
            m_station->m_field58 = static_cast<int>(g_piaTransport->m_time.m_tick >> 32);
            m_station->m_field5c = static_cast<int>(g_piaTransport->m_time.m_tick);
        }
        m_connectionRejection = 0;
        m_connectionDeadline.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick +
            common::TimeSpan::GetTicksPerMSec().m_tick * m_timeoutMsec;
        m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
            &ConnectStationJob::WaitRequestAck);
        m_currentStep.m_name = "ConnectStationJob::WaitRequestAck";
        return cExecuteResultNextStep;
    }
    m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
        &ConnectStationJob::ConnectionFailed);
    m_currentStep.m_name = "ConnectStationJob::ConnectionFailed";
    return cExecuteResultContinue;
}

unsigned int ConnectStationJob::WaitRequestAck()
{
    if (!g_piaResendingMessageManager->CheckNowResending(m_requestAckId))
    {
        m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
            &ConnectStationJob::WaitForConnection);
        m_currentStep.m_name = "ConnectStationJob::WaitForConnection";
        m_requestAckId = 0;
        return cExecuteResultContinue;
    }
    if (CheckContextCallCancelled())
    {
        g_piaResendingMessageManager->StopResending(m_requestAckId);
        m_requestAckId = 0;
        if (m_station && m_station->m_field20 == 4)
            m_station->m_field20 = 6;
        return cExecuteResultFinished;
    }
    if (CheckTimeout() || CheckErrorOccurred())
    {
        g_piaResendingMessageManager->StopResending(m_requestAckId);
        m_requestAckId = 0;
        m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
            &ConnectStationJob::ConnectionFailed);
        m_currentStep.m_name = "ConnectStationJob::ConnectionFailed";
        return cExecuteResultContinue;
    }
    if (m_station->m_field20 == 5)
    {
        g_piaResendingMessageManager->StopResending(m_requestAckId);
        m_requestAckId = 0;
        m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
            &ConnectStationJob::ConnectionSucceeded);
        m_currentStep.m_name = "ConnectStationJob::ConnectionSucceeded";
        return cExecuteResultContinue;
    }
    if (m_connectionRejection)
    {
        if (m_context)
        {
            if (static_cast<unsigned int>(m_context->m_state) - 2 > 2)
                m_context->SignalFailure(m_connectionRejection == 1 ? 0xc24 :
                    m_connectionRejection == 2 ? 0x10c25 : 0xc23);
            m_context = 0;
        }
        g_piaResendingMessageManager->StopResending(m_requestAckId);
        m_requestAckId = 0;
        if (m_station && m_station->m_field20 == 4)
            m_station->m_field20 = 6;
        return cExecuteResultFinished;
    }
    return cExecuteResultNextStep;
}

unsigned int ConnectStationJob::WaitForConnection()
{
    if (m_station->m_field20 == 5)
    {
        m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
            &ConnectStationJob::ConnectionSucceeded);
        m_currentStep.m_name = "ConnectStationJob::ConnectionSucceeded";
        return cExecuteResultContinue;
    }
    if (CheckContextCallCancelled())
    {
        if (m_station && m_station->m_field20 == 4)
            m_station->m_field20 = 6;
        return cExecuteResultFinished;
    }
    if (!CheckTimeout() && !CheckErrorOccurred())
        return cExecuteResultNextStep;
    m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
        &ConnectStationJob::ConnectionFailed);
    m_currentStep.m_name = "ConnectStationJob::ConnectionFailed";
    return cExecuteResultContinue;
}

unsigned int ConnectStationJob::ConnectionFailed()
{
    if (m_station && m_station->m_field20 == 4)
        m_station->m_field20 = 6;
    if (m_context && static_cast<unsigned int>(m_context->m_state) - 2 > 2)
        m_context->SignalFailure(0xc23);
    m_context = 0;
    return cExecuteResultFinished;
}

unsigned int ConnectStationJob::ConnectionSucceeded()
{
    if (CheckContextCallCancelled())
        return cExecuteResultFinished;
    if (CheckErrorOccurred())
    {
        m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
            &ConnectStationJob::ConnectionFailed);
        m_currentStep.m_name = "ConnectStationJob::ConnectionFailed";
        return cExecuteResultContinue;
    }
    if (static_cast<int>(m_station->m_field10) < 32)
        g_piaTransport->OutputStreamUpdateEvent();
    if (m_context && static_cast<unsigned int>(m_context->m_state) - 2 > 2)
        m_context->SignalSuccess(0);
    m_context = 0;
    return cExecuteResultFinished;
}

unsigned int ConnectStationJob::SendRelayConnectionRequest()
{
    if (CheckContextCallCancelled())
    {
        if (m_station && m_station->m_field20 == 4)
            m_station->m_field20 = 6;
        return cExecuteResultFinished;
    }
    if (SendRelayConnectionRequestMessage())
    {
        m_connectionRejection = 0;
        m_connectionDeadline.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick +
            common::TimeSpan::GetTicksPerMSec().m_tick * m_timeoutMsec;
        m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
            &ConnectStationJob::WaitRequestAck);
        m_currentStep.m_name = "ConnectStationJob::WaitRequestAck";
    }
    return cExecuteResultNextStep;
}

bool ConnectStationJob::SendConnectionRequestMessage()
{
    if (!m_station->m_field0)
        return false;
    StationProtocol* protocol = m_station->m_field28;

    unsigned long long principalId;
    if (m_station->GetPrincipalId(&principalId) != 0)
        return false;

    StationConnectionInfo info;
    if (g_stationConnectionInfoTable->GetStationConnectionInfo(m_station, &info) != 0)
        return false;

    unsigned char buffer[128] = {};
    protocol->MakeConnectionRequestData(buffer, m_station->m_field54, principalId,
                                         static_cast<unsigned int>(info.m_stationLocation.m_field18),
                                         m_station->m_field55, false, m_secure);

    common::StationAddress stationAddress;
    const int stationIndex = static_cast<int>(m_station->m_field10);
    if (g_piaTransport->m_relayRouteManager != 0 || stationIndex == 254)
    {
        stationAddress = m_station->m_stationAddress;
        return g_piaResendingMessageManager->SetSendMessageByStationAddress(
            &m_requestAckId, buffer, protocol->GetConnectionRequestDataSize(),
            stationAddress, protocol->mProtocolId, 0) == 0;
    }
    if (stationIndex <= 31)
    {
        return g_piaResendingMessageManager->SetSendMessageByStationIndex(
            &m_requestAckId, buffer, protocol->GetConnectionRequestDataSize(),
            static_cast<StationIndex>(stationIndex), protocol->mProtocolId, false, 0) == 0;
    }
    stationAddress = m_station->m_stationAddress;
    return g_piaResendingMessageManager->SetSendMessageByStationAddress(
        &m_requestAckId, buffer, protocol->GetConnectionRequestDataSize(), stationAddress,
        protocol->mProtocolId, 0) == 0;
}

bool ConnectStationJob::SendRelayConnectionRequestMessage()
{
    if (!m_station->m_field0 || static_cast<int>(m_station->m_field10) > 31)
        return false;
    StationProtocol* protocol = m_station->m_field28;

    unsigned long long principalId;
    if (m_station->GetPrincipalId(&principalId) != 0)
        return false;

    StationConnectionInfo info;
    if (g_stationConnectionInfoTable->GetStationConnectionInfo(m_station, &info) != 0)
        return false;

    unsigned char buffer[128] = {};
    protocol->MakeConnectionRequestData(buffer, m_station->m_field54, principalId,
                                         static_cast<unsigned int>(info.m_stationLocation.m_field18),
                                         m_station->m_field55, true, m_secure);
    return g_piaResendingMessageManager->SetSendMessageByStationIndex(
        &m_requestAckId, buffer, protocol->GetConnectionRequestDataSize(),
        static_cast<StationIndex>(m_station->m_field10), protocol->mProtocolId, false, 0) == 0;
}

}}}
