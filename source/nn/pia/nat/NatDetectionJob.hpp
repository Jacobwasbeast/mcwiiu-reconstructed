#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NAT_DETECTION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NAT_DETECTION_JOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn { namespace pia { namespace common { class CallContext; } namespace nat {

class NatDetecter;
class NatTraverser;

class NatDetectionJob : public common::StepSequenceJob
{
public:
    NatDetectionJob();
    virtual ~NatDetectionJob();

    common::Result Startup(common::CallContext* context, NatDetecter* detecter,
                           NatTraverser* traverser);
    void Cleanup();
    virtual void CancelCleanup();
    ExecuteResult StepPreTest();
    ExecuteResult StepStart();
    void StopReceivingMessage();
    ExecuteResult StepSend();
    ExecuteResult StepWait();
    ExecuteResult StepEnd();
    ExecuteResult StepComplete();

private:
    common::CallContext* m_context;
    NatDetecter* m_detecter;
    NatTraverser* m_traverser;
    unsigned int m_field4c;
    unsigned long long m_endTime;
    unsigned long long m_waitTime;
    bool m_sendStarted;
    bool m_receivedFirstReply;
    unsigned char m_padding62[6];
    unsigned long long m_firstSendTime;
    bool m_success;
    unsigned char m_padding71[7];
};

}}}

#endif
