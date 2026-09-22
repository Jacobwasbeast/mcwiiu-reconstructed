#include "nn/pia/nat/NatDetectionJob.hpp"

#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/nat/Api.hpp"
#include "nn/pia/nat/NatDetecter.hpp"
#include "nn/pia/nat/NatTraverser.hpp"

namespace nn { namespace pia { namespace nat {

namespace {
typedef unsigned int (common::StepSequenceJob::*BaseStep)();

template <typename T>
BaseStep stepOf(unsigned int (T::*method)())
{
    return static_cast<BaseStep>(method);
}

void setStep(NatDetectionJob* job, unsigned int (NatDetectionJob::*method)(),
             const char* name)
{
    job->m_currentStep.m_function = stepOf(method);
    job->m_currentStep.m_name = name;
}

unsigned long long now()
{
    common::Time time;
    time.SetNow();
    return time.m_tick;
}

unsigned long long milliseconds(unsigned int value)
{
    return static_cast<unsigned long long>(common::TimeSpan::GetTicksPerMSec().m_tick) * value;
}

bool reached(unsigned long long current, unsigned long long deadline)
{
    return current >= deadline;
}

}

NatDetectionJob::NatDetectionJob()
    : common::StepSequenceJob(), m_context(0), m_detecter(0), m_traverser(0),
      m_field4c(0), m_endTime(0), m_waitTime(0), m_sendStarted(false),
      m_receivedFirstReply(false), m_padding62(), m_firstSendTime(0),
      m_success(false), m_padding71()
{
}

NatDetectionJob::~NatDetectionJob()
{
}

void NatDetectionJob::Startup(common::CallContext* context, NatDetecter* detecter,
                              NatTraverser* traverser)
{
    m_endTime = 0;
    m_detecter = detecter;
    m_waitTime = 0;
    m_firstSendTime = 0;
    setStep(this, &NatDetectionJob::StepPreTest, "_0_NatDetectionJob__StepPreTest");
    m_traverser = traverser;
    Reset(true);
    m_context = context;
    if (m_context == 0 || !m_context->InitiateCall())
        return;
    m_sendStarted = false;
    m_receivedFirstReply = false;
    m_success = false;
}

void NatDetectionJob::Cleanup()
{
    if (m_detecter != 0)
    {
        m_detecter->CloseSocket();
        m_detecter = 0;
    }
}

void NatDetectionJob::CancelCleanup()
{
    if (m_context != 0 && m_context->m_state == common::CallContext::cStateInProgress)
        m_context->SignalCancel();
    Cleanup();
}

unsigned int NatDetectionJob::StepPreTest()
{
    if (m_traverser->IsStartupCancelled())
    {
        setStep(this, &NatDetectionJob::StepComplete, "NatDetectionJob__StepComplete");
        return cExecuteResultEntryForeground;
    }
    const Result result = m_detecter->OpenSocket();
    if (result != 0)
    {
        m_context->SignalFailure(result);
        Cleanup();
        return cExecuteResultFinished;
    }
    setStep(this, &NatDetectionJob::StepStart, "NatDetectionJob__StepStart");
    return cExecuteResultContinue;
}

unsigned int NatDetectionJob::StepStart()
{
    if (m_traverser->IsStartupCancelled())
    {
        setStep(this, &NatDetectionJob::StepEnd, "NatDetectionJob__StepEnd");
        return cExecuteResultContinue;
    }
    const unsigned int timeout = m_detecter->GetDetectionTimeout();
    m_endTime = now() + milliseconds(timeout);
    m_detecter->InitializeNatCheckReply();
    m_detecter->StartSendingNatCheckProbe();
    setStep(this, &NatDetectionJob::StepSend, "NatDetectionJob__StepSend");
    return cExecuteResultContinue;
}

void NatDetectionJob::StopReceivingMessage()
{
    if (m_detecter->CheckRetry() == 0)
        setStep(this, &NatDetectionJob::StepEnd, "NatDetectionJob__StepEnd");
    else
    {
        m_detecter->Retry();
        setStep(this, &NatDetectionJob::StepStart, "NatDetectionJob__StepStart");
    }
}

unsigned int NatDetectionJob::StepSend()
{
    if (m_traverser->IsStartupCancelled())
    {
        setStep(this, &NatDetectionJob::StepEnd, "NatDetectionJob__StepEnd");
        return cExecuteResultContinue;
    }
    if (reached(now(), m_endTime))
    {
        StopReceivingMessage();
        return cExecuteResultContinue;
    }
    if (!m_sendStarted)
    {
        m_sendStarted = true;
        m_firstSendTime = now();
    }
    if (m_detecter->SendAllQueuedProbe() == 0)
    {
        m_waitTime = now() + milliseconds(20);
        setStep(this, &NatDetectionJob::StepWait, "NatDetectionJob__StepWait");
        return cExecuteResultContinue;
    }
    if (m_detecter->ReceiveNatCheckReply() == 0)
    {
        m_waitTime = now() + milliseconds(100);
        setStep(this, &NatDetectionJob::StepWait, "NatDetectionJob__StepWait");
        return cExecuteResultContinue;
    }
    if (!m_receivedFirstReply)
    {
        m_receivedFirstReply = true;
        const unsigned long long elapsed = now() - m_firstSendTime;
        *reinterpret_cast<unsigned int*>(reinterpret_cast<unsigned char*>(m_detecter) + 0x324) =
            static_cast<unsigned int>(elapsed / static_cast<unsigned long long>(common::TimeSpan::GetTicksPerMSec().m_tick));
    }
    if (m_detecter->IsReceivedAllMessage())
    {
        StopReceivingMessage();
        return cExecuteResultContinue;
    }
    m_waitTime = now() + milliseconds(100);
    setStep(this, &NatDetectionJob::StepWait, "NatDetectionJob__StepWait");
    return cExecuteResultContinue;
}

unsigned int NatDetectionJob::StepWait()
{
    if (m_traverser->IsStartupCancelled())
    {
        setStep(this, &NatDetectionJob::StepEnd, "NatDetectionJob__StepEnd");
        return cExecuteResultContinue;
    }
    const unsigned long long current = now();
    if (reached(current, m_waitTime))
    {
        setStep(this, &NatDetectionJob::StepSend, "NatDetectionJob__StepSend");
        return cExecuteResultContinue;
    }
    return cExecuteResultWait | 5;
}

unsigned int NatDetectionJob::StepEnd()
{
    m_success = (m_detecter->CloseSocket() == 0);
    setStep(this, &NatDetectionJob::StepComplete, "NatDetectionJob__StepComplete");
    return cExecuteResultEntryForeground;
}

unsigned int NatDetectionJob::StepComplete()
{
    if (m_traverser->IsStartupCancelled())
    {
        m_context->SignalFailure(0x6c05);
        Cleanup();
        return cExecuteResultFinished;
    }
    if (m_success)
    {
        m_context->SignalFailure(0x6434);
        Cleanup();
        return cExecuteResultFinished;
    }
    if (m_detecter->HandleResult() != 0)
    {
        const DebugSetting* setting = GetDebugSetting();
        if (setting != 0 && setting->options[2] == 0)
        {
            m_context->SignalSuccess(0);
            Cleanup();
            return cExecuteResultFinished;
        }
    }
    m_context->SignalFailure(0x6434);
    Cleanup();
    return cExecuteResultFinished;
}

}}}
