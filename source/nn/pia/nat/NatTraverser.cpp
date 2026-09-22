#include "nn/pia/nat/NatTraverser.hpp"

#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/inet/IFacade.hpp"
#include "nn/pia/nat/NatTraversalProtocol.hpp"
#include "nn/pia/nat/NatTraversalTime.hpp"

namespace nn { namespace pia { namespace nat {

NatTraverser::NatTraverser()
    : m_activeContext(0), m_monitoringData(), m_servers(), m_detectionActive(0),
      m_reservedState(0), m_portDelta(0), m_serverCount(0), m_protocol(0),
      m_natPortDetecter(0), m_natPropertyDetecter(0), m_propertyContext(),
      m_portContext(), m_localServiceConnectionId(0), m_randomPortRangeMin(0),
      m_randomPortRangeMax(0), m_relayServerAddress(0), m_startupCancelled(false),
      m_facade(0), m_monitoringConnectionIds(), m_successCount(0), m_failureCount(0)
{
    for (unsigned int i = 0; i < 15; ++i)
        m_monitoringConnectionIds[i] = 0;
}

NatTraverser::~NatTraverser()
{
    m_natPortDetecter = 0;
    m_natPropertyDetecter = 0;
    m_protocol = 0;
    m_activeContext = 0;
}

common::Result NatTraverser::Startup(inet::IFacade* facade,
                                     common::CallContext* context)
{
    if (facade == 0 || context == 0)
        return static_cast<common::Result>(0x10c07);
    if (m_activeContext != 0 || m_propertyContext.m_state == common::CallContext::cStateInProgress ||
        m_portContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<common::Result>(0x10c08);
    if (m_detectionActive != 0 || m_protocol == 0)
        return static_cast<common::Result>(0x10c08);

    m_facade = facade;
    m_activeContext = context;
    m_startupCancelled = false;
    m_detectionActive = 1;
    return 0;
}

void NatTraverser::CompleteNatDetectionCommon()
{
    if (m_activeContext == 0)
        return;
    if (m_activeContext->m_cancelRequested)
        m_activeContext->SignalCancel();
    else if (m_detectionActive == 0)
        m_activeContext->SignalFailure(static_cast<common::Result>(0x10c08));
    else
        m_activeContext->SignalSuccess(0);
    m_activeContext = 0;
}

void NatTraverser::CompletedNatPropertyDetection(bool success)
{
    if (m_detectionActive == 0 || m_activeContext == 0)
        return;
    if (!success && m_propertyContext.m_result == static_cast<common::Result>(0x6c05))
        m_activeContext->SignalCancel();
    else if (!success)
        m_activeContext->SignalFailure(m_propertyContext.m_result);
    else
        CompleteNatDetectionCommon();
    m_propertyContext.Reset();
}

void NatTraverser::CompletedNatPortDetection(bool success)
{
    if (m_detectionActive == 0 || m_activeContext == 0)
        return;
    if (!success && m_portContext.m_result == static_cast<common::Result>(0x6c05))
        m_activeContext->SignalCancel();
    else if (!success)
        m_activeContext->SignalFailure(m_portContext.m_result);
    else
        CompleteNatDetectionCommon();
    m_portContext.Reset();
}

void NatTraverser::Cleanup()
{
    m_activeContext = 0;
    m_facade = 0;
    m_detectionActive = 0;
    m_startupCancelled = false;
    m_propertyContext.Reset();
    m_portContext.Reset();
}

bool NatTraverser::IsStartupCancelled()
{
    return m_startupCancelled || (m_activeContext != 0 && m_activeContext->m_cancelRequested);
}

int NatTraverser::GetNatMonitoringDataIndex(const transport::StationLocation& location)
{
    if (m_facade == 0)
        return 0xff;
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&location);
    const unsigned int connectionId = *reinterpret_cast<const unsigned int*>(bytes + 0x1c);
    for (unsigned int i = 0; i < 15; ++i)
        if (m_monitoringConnectionIds[i] == connectionId)
            return static_cast<int>(i);
    for (unsigned int i = 0; i < 15; ++i)
        if (m_monitoringConnectionIds[i] == 0xffffffffu)
        {
            m_monitoringConnectionIds[i] = connectionId;
            return static_cast<int>(i);
        }
    return 0xff;
}

void NatTraverser::IncrementNatTraversalSuccessCount(unsigned int)
{
    if (m_facade != 0)
        ++m_successCount;
}

void NatTraverser::IncrementNatTraversalFailureCount(unsigned int)
{
    if (m_facade != 0)
        ++m_failureCount;
}

void NatTraverser::ClearNatMonitoringData(const transport::StationLocation& location)
{
    const int index = GetNatMonitoringDataIndex(location);
    if (index >= 0 && index < 15)
        m_monitoringConnectionIds[index] = 0xffffffffu;
}

void NatTraverser::RelayProbeRequest(const transport::StationLocation& source,
                                     const transport::StationLocation& destination,
                                     bool relay, unsigned short port)
{
    m_facade->RelayProbeRequest(source, destination);
    if (!relay)
        return;

    NatTraversalTime* traversalTime = m_protocol->m_traversalTimes.Find(source);
    if (traversalTime != 0)
    {
        traversalTime->m_port = port;
        traversalTime->m_endTime.SetNow();
        traversalTime->m_count = 0;
        return;
    }

    NatTraversalTime newTraversalTime;
    newTraversalTime.m_location = source;
    newTraversalTime.m_startTime.SetNow();
    newTraversalTime.m_endTime.SetNow();
    newTraversalTime.m_count = 0;
    newTraversalTime.m_port = port;
    m_protocol->m_traversalTimes.Add(newTraversalTime);
}

bool NatTraverser::IsNatDetectionCompleted() const
{
    return m_detectionActive == 3;
}

bool NatTraverser::IsProcessingNatDetection() const
{
    return m_detectionActive != 0;
}

unsigned int NatTraverser::GetLocalServiceConnectionId() const
{
    return m_localServiceConnectionId;
}

unsigned short NatTraverser::GetRandomPortRangeMin() const
{
    return m_randomPortRangeMin;
}

unsigned short NatTraverser::GetRandomPortRangeMax() const
{
    return m_randomPortRangeMax;
}

unsigned int NatTraverser::GetRelayServerAddress() const
{
    return m_relayServerAddress;
}

void* NatTraverser::GetNatPortDetecter()
{
    return m_natPortDetecter;
}

void NatTraverser::SetLocalPublicLocation(const transport::StationLocation& location)
{
    if (m_protocol == 0)
        return;
    m_protocol->m_localLocation = location;
}

void NatTraverser::Trace(unsigned long long value) const { (void)value; }

}}}
