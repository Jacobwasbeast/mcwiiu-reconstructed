#include "nn/pia/nat/NatProbe.hpp"

#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/session/Mesh.hpp"

namespace nn { namespace pia { namespace nat {

namespace
{
unsigned short s_targetAddressUpdateCount = 0xffff;
}

NatProbe::NatProbe(const transport::StationLocation& location, const common::Time& start,
                   const common::TimeSpan& duration, unsigned char timeout,
                   unsigned char sprayCount, unsigned char sprayRange)
    : m_stationLocation(location),
      m_startTime(start),
      m_deadline(), m_received(0), m_sent(0),
      m_replied(0), m_rtt(-1), m_timeout(timeout), m_probeCount(0),
      m_probeLimit(sprayCount),
      m_firstPort(static_cast<short>(m_stationLocation.m_stationAddress.m_port)),
      m_lastPort(static_cast<short>(m_stationLocation.m_stationAddress.m_port)),
      m_ticksPerTimeout(0)
{
    m_ticksPerTimeout =
        static_cast<unsigned long long>(common::TimeSpan::GetTicksPerMSec().m_tick) * 90000ULL;

    common::Time now;
    now.SetNow();
    m_deadline.m_tick = now.m_tick + duration.m_tick;

    if (location.m_field22 == 0xff)
    {
        common::InetAddress adjusted(location.m_stationAddress.m_address,
                                     static_cast<unsigned short>(location.m_stationAddress.m_port + 10));
        m_stationLocation.m_stationAddress.SetInetAddress(adjusted);
    }

    m_firstPort = static_cast<short>(m_stationLocation.m_stationAddress.m_port);
    m_lastPort = static_cast<short>(m_firstPort + sprayRange);
    if (static_cast<unsigned short>(m_lastPort) < static_cast<unsigned short>(m_firstPort))
        m_lastPort = static_cast<short>(0xffff);
}

NatProbe::~NatProbe()
{
}

bool NatProbe::UpdateTargetAddress(const transport::StationLocation& location)
{
    const bool thisPrivate = m_stationLocation.m_stationAddress.IsPrivate() != 0;
    const bool targetPrivate = location.m_stationAddress.IsPrivate() != 0;

    if (thisPrivate && !targetPrivate)
    {
        if (session::g_piaMesh == 0 || !session::g_piaMesh->IsProcessedAtMeshBegin())
        {
            if (s_targetAddressUpdateCount == 0xffff)
                s_targetAddressUpdateCount = 1;
            else
                ++s_targetAddressUpdateCount;
        }
        if (m_rtt != -1 || m_startTime.m_tick != 0)
            return false;
    }
    else if (thisPrivate && targetPrivate)
    {
        if (m_startTime.m_tick != 0 &&
             !(m_stationLocation.m_stationAddress == location.m_stationAddress))
            return false;
    }
    else if (!thisPrivate && !targetPrivate)
    {
        if (m_startTime.m_tick != 0 &&
             !(m_stationLocation.m_stationAddress == location.m_stationAddress))
            return false;
    }
    else if (session::g_piaMesh == 0 || !session::g_piaMesh->IsProcessedAtMeshBegin())
    {
        if (s_targetAddressUpdateCount == 0xffff)
            s_targetAddressUpdateCount = 1;
        else
            ++s_targetAddressUpdateCount;
    }

    const unsigned long long oldKey = m_stationLocation.m_stationAddress.GetKey();
    m_stationLocation.m_stationAddress.SetInetAddress(location.m_stationAddress);
    m_firstPort = static_cast<short>(location.m_stationAddress.m_port);
    m_lastPort = m_firstPort;
    if (m_stationLocation.m_stationAddress.GetKey() != oldKey)
        m_probeCount = 0;
    return true;
}

void NatProbe::UpdateTargetPort(const transport::StationLocation& location)
{
    if (m_startTime.m_tick != 0 && m_stationLocation.m_field26 == 0)
        return;
    const unsigned short oldPort = m_stationLocation.m_stationAddress.m_port;
    m_stationLocation.m_stationAddress.m_port = location.m_stationAddress.m_port;
    if (oldPort != m_stationLocation.m_stationAddress.m_port)
        m_probeCount = 0;
}

unsigned long long NatProbe::UpdateRtt(const common::Time& sent, const common::Time& received)
{
    const unsigned long long elapsed = sent.m_tick - received.m_tick;
    const unsigned long long ticksPerMSec =
        static_cast<unsigned long long>(common::TimeSpan::GetTicksPerMSec().m_tick);
    const unsigned long long rtt = elapsed / ticksPerMSec;
    m_rtt = static_cast<int>(rtt);
    return rtt;
}

bool NatProbe::UpdateIsNeeded(common::Time now) const
{
    if (m_rtt != -1)
    {
        const unsigned long long current = now.m_tick - m_startTime.m_tick;
        return current > m_ticksPerTimeout;
    }
    return m_probeCount < m_probeLimit;
}

void NatProbe::SprayTargetPort()
{
    const unsigned short count = static_cast<unsigned short>(
        static_cast<unsigned short>(m_lastPort) - static_cast<unsigned short>(m_firstPort) + 1);
    if (count == 1)
        return;
    const unsigned short counter16 = static_cast<unsigned short>(m_probeCount);
    const unsigned short offset = counter16 % count;
    m_stationLocation.m_stationAddress.m_port = static_cast<unsigned short>(m_firstPort + offset);
}

short NatProbe::GetPortSprayCount()
{
    return static_cast<short>(m_stationLocation.m_stationAddress.m_port - m_firstPort);
}

void NatProbe::Trace(unsigned long long) const
{
}

}}}
