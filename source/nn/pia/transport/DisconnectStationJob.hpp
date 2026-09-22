#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_DISCONNECT_STATION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_DISCONNECT_STATION_JOB_HPP

#include "nn/pia.h"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn { namespace pia { namespace transport {

class DisconnectStationJob : public common::StepSequenceJob
{
public:
    DisconnectStationJob();
    virtual ~DisconnectStationJob();

    void Startup(Station* station);
    unsigned int SendDisconnectionRequest();
    unsigned int WaitForDisconnection();
    unsigned int DisconnectionSucceeded();
    unsigned int CutRouteOfRelayConnection();
    void Cleanup();
    virtual void Trace(unsigned long long value) const;
    virtual unsigned int StartupImpl(Station* station);
    void ProcessDisconnection(Station* station);
    void DisconnectionFailed();

private:
    Station* m_station;
    unsigned int m_deadline;
    unsigned int m_retryDeadline;
    unsigned int m_timeout;
    unsigned int m_retryInterval;
    unsigned char m_waiting;
    unsigned char m_padding[3];
};

}}}

#endif
