#ifndef NN_PIA_NAT_NATPROBE_HPP
#define NN_PIA_NAT_NATPROBE_HPP

#include "nn/pia/transport/StationLocation.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/common/TimeSpan.hpp"

namespace nn { namespace pia { namespace nat {

class NatProbe : public transport::StationLocation
{
public:
    NatProbe(const transport::StationLocation& location, const common::Time& start,
             const common::TimeSpan& duration, unsigned char timeout,
             unsigned char sprayCount, unsigned char sprayRange);
    virtual ~NatProbe();

    bool UpdateTargetAddress(const transport::StationLocation& location);
    void UpdateTargetPort(const transport::StationLocation& location);
    unsigned long long UpdateRtt(const common::Time& sent, const common::Time& received);
    bool UpdateIsNeeded(common::Time now) const;
    void SprayTargetPort();
    short GetPortSprayCount();
    virtual void Trace(unsigned long long value) const;

private:
    unsigned int m_startTick;
    unsigned int m_startTickHigh;
    unsigned int m_deadlineTick;
    unsigned int m_deadlineTickHigh;
    unsigned char m_received;
    unsigned char m_sent;
    unsigned char m_replied;
    int m_rtt;
    unsigned char m_timeout;
    unsigned int m_probeCount;
    unsigned int m_probeLimit;
    short m_firstPort;
    short m_lastPort;
    unsigned int m_ticksPerTimeout;
    unsigned int m_ticksPerTimeoutHigh;
};

}}}

#endif
