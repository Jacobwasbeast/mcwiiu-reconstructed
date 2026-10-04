#ifndef NN_PIA_NAT_NATPROBE_HPP
#define NN_PIA_NAT_NATPROBE_HPP

#include "nn/pia/transport/StationLocation.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/common/TimeSpan.hpp"

namespace nn { namespace pia { namespace nat {

class NatProbe
{
public:
    NatProbe(const transport::StationLocation& location, const common::Time& start,
             const common::TimeSpan& duration, unsigned char timeout,
             unsigned char sprayCount, unsigned char sprayRange);
    virtual ~NatProbe() {}

    bool UpdateTargetAddress(const transport::StationLocation& location);
    void UpdateTargetPort(const transport::StationLocation& location);
    unsigned long long UpdateRtt(const common::Time& sent, const common::Time& received);
    bool UpdateIsNeeded(common::Time now) const;
    void SprayTargetPort();
    short GetPortSprayCount();
    virtual void Trace(unsigned long long value) const;

    NatProbe()
        : m_stationLocation(), m_startTime(), m_deadline(), m_ticksPerTimeout(0)
    {
    }

    transport::StationLocation m_stationLocation;
    common::Time m_startTime;
    common::Time m_deadline;
    unsigned char m_received;
    unsigned char m_sent;
    unsigned char m_replied;
    int m_rtt;
    unsigned char m_timeout;
    unsigned int m_probeCount;
    unsigned int m_probeLimit;
    short m_firstPort;
    short m_lastPort;
    unsigned long long m_ticksPerTimeout;
};

}}}

#endif
