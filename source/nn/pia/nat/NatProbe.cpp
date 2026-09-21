#include "nn/pia/nat/NatProbe.hpp"

#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/session/Mesh.hpp"

namespace nn { namespace pia { namespace nat {

namespace
{
unsigned short s_targetAddressUpdateCount = 0xffff;
}

NatProbe::NatProbe()
    : transport::StationLocation()
{
}

NatProbe::NatProbe(const transport::StationLocation& location, const common::Time& start,
                   const common::TimeSpan& duration, unsigned char timeout,
                   unsigned char sprayCount, unsigned char sprayRange)
    : transport::StationLocation(location),
      m_startTick(static_cast<unsigned int>(start.m_tick)),
      m_startTickHigh(static_cast<unsigned int>(start.m_tick >> 32)),
      m_deadlineTick(0), m_deadlineTickHigh(0), m_received(0), m_sent(0),
      m_replied(0), m_rtt(-1), m_timeout(timeout), m_probeCount(0),
      m_probeLimit(sprayCount),
      m_firstPort(static_cast<short>(m_port)),
      m_lastPort(static_cast<short>(m_port)),
      m_ticksPerTimeout(0), m_ticksPerTimeoutHigh(0)
{
    const unsigned long long ticks =
        static_cast<unsigned long long>(common::TimeSpan::GetTicksPerMSec().m_tick) * 90000ULL;
    m_ticksPerTimeout = static_cast<unsigned int>(ticks);
    m_ticksPerTimeoutHigh = static_cast<unsigned int>(ticks >> 32);

    common::Time now;
    now.SetNow();
    const unsigned long long deadline = now.m_tick + duration.m_tick;
    m_deadlineTick = static_cast<unsigned int>(deadline);
    m_deadlineTickHigh = static_cast<unsigned int>(deadline >> 32);

    if (location.m_field22 == 0xff)
    {
        common::InetAddress adjusted(location.m_address,
                                     static_cast<unsigned short>(location.m_port + 10));
        SetInetAddress(adjusted);
    }

    m_firstPort = static_cast<short>(m_port);
    m_lastPort = static_cast<short>(m_firstPort + sprayRange);
    if (static_cast<unsigned short>(m_lastPort) < static_cast<unsigned short>(m_firstPort))
        m_lastPort = static_cast<short>(0xffff);
}

NatProbe::~NatProbe()
{
}

bool NatProbe::UpdateTargetAddress(const transport::StationLocation& location)
{
    const bool thisPrivate = IsPrivate() != 0;
    const bool targetPrivate = location.IsPrivate() != 0;

    if (thisPrivate && !targetPrivate)
    {
        if (session::g_piaMesh == 0 || !session::g_piaMesh->IsProcessedAtMeshBegin())
        {
            if (s_targetAddressUpdateCount == 0xffff)
                s_targetAddressUpdateCount = 1;
            else
                ++s_targetAddressUpdateCount;
        }
        if (m_rtt != -1 || m_startTick != 0 || m_startTickHigh != 0)
            return false;
    }
    else if (thisPrivate && targetPrivate)
    {
        if ((m_startTick != 0 || m_startTickHigh != 0) &&
             !(static_cast<const transport::StationAddress&>(*this).operator==(
                 static_cast<const transport::StationAddress&>(location))))
            return false;
    }
    else if (!thisPrivate && !targetPrivate)
    {
        if ((m_startTick != 0 || m_startTickHigh != 0) &&
             !(static_cast<const transport::StationAddress&>(*this).operator==(
                 static_cast<const transport::StationAddress&>(location))))
            return false;
    }
    else if (session::g_piaMesh == 0 || !session::g_piaMesh->IsProcessedAtMeshBegin())
    {
        if (s_targetAddressUpdateCount == 0xffff)
            s_targetAddressUpdateCount = 1;
        else
            ++s_targetAddressUpdateCount;
    }

    const unsigned long long oldKey = GetKey();
    common::InetAddress address;
    address = location;
    SetInetAddress(address);
    m_firstPort = static_cast<short>(location.m_port);
    m_lastPort = m_firstPort;
    if (GetKey() != oldKey)
        m_probeCount = 0;
    return true;
}

void NatProbe::UpdateTargetPort(const transport::StationLocation& location)
{
    if ((m_startTick != 0 || m_startTickHigh != 0) && m_field26 == 0)
        return;
    const unsigned short oldPort = m_port;
    m_port = location.m_port;
    if (oldPort != m_port)
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
        const unsigned long long start =
            (static_cast<unsigned long long>(m_startTickHigh) << 32) | m_startTick;
        const unsigned long long current = now.m_tick - start;
        const unsigned long long timeout =
            (static_cast<unsigned long long>(m_ticksPerTimeoutHigh) << 32) | m_ticksPerTimeout;
        return current > timeout;
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
    m_port = static_cast<unsigned short>(m_firstPort + offset);
}

short NatProbe::GetPortSprayCount()
{
    return static_cast<short>(m_port - m_firstPort);
}

void NatProbe::Trace(unsigned long long) const
{
}

}}}
