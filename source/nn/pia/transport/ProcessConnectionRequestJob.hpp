#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PROCESSCONNECTIONREQUESTJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PROCESSCONNECTIONREQUESTJOB_HPP

#include "nn/pia.h"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn { namespace pia { namespace transport {

class ProcessConnectionRequestJob : public common::StepSequenceJob
{
public:
    ProcessConnectionRequestJob();
    virtual ~ProcessConnectionRequestJob();
    unsigned int Startup(Station* station, int timeoutMsec, bool relay);
    unsigned int StartupRelayConnection(Station* station, int timeoutMsec, bool relay);
    unsigned int ConnectToRequesterStation();
    unsigned int WaitInverseConnection();
    unsigned int TimeOutCheck();
    unsigned int SendConnectionResponse();
    unsigned int WaitResponseAck();
    void Cleanup();
    virtual void Trace(unsigned long long value) const;

private:
    unsigned int m_deadlineHigh;
    unsigned int m_deadlineLow;
    unsigned int m_timeoutMsec;
    unsigned int m_connectResult;
    unsigned int m_responseResult;
    Station* m_station;
    common::CallContext* m_callContext;
    bool m_relay;
};

}}}

#endif
