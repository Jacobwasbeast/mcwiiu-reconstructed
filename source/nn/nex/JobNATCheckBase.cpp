#include "nn/nex/JobNATCheckBase.hpp"

#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum CheckResult
{
    NAT_CHECK_SUCCEEDED_RESULT = 0x10001
};

enum MessageType
{
    MESSAGE_TYPE_TEST1_REQUEST = 0x65
};

const unsigned int BIND_ATTEMPT_COUNT = 5;
const int RESOLVE_WAIT = 10;
const int EXECUTE_WAIT = 10;

unsigned int s_uiMaxAttemptCount = 3;

}

JobNATCheckBase::JobNATCheckBase(CallContext* pCallContext, const InetAddress& rAddress)
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"JobNATCheckBase"))
    , m_localAddress(rAddress)
    , m_socket(0, 1000)
    , m_queue()
    , m_recvMessage()
    , m_receivedFrom()
    , m_aReceived()
    , m_deadline(0)
    , m_callContext()
{
    m_uiCallId = pCallContext->m_uiCallId;
    m_uiAttempt = 0;
}

JobNATCheckBase::~JobNATCheckBase()
{
}

void JobNATCheckBase::StepDNSResolve()
{
    CallContext* pCallContext =
        Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobNATCheckBase::StepFinish), L""));
        return;
    }
    switch (m_callContext.GetState())
    {
    default:
        SetStep(Step(static_cast<Step::Function>(&JobNATCheckBase::StepFinish), L""));
        return;
    case CallContext::CallInProgress:
        SetToWaiting(RESOLVE_WAIT);
        return;
    case CallContext::CallFailed:
        SetStep(Step(static_cast<Step::Function>(&JobNATCheckBase::StepFinish), L""));
        return;
    case CallContext::CallSucceeded:
        SetStep(Step(static_cast<Step::Function>(&JobNATCheckBase::StepInitializeImpl), L""));
        return;
    case CallContext::Ready:
        SetStep(Step(static_cast<Step::Function>(&JobNATCheckBase::StepFinish), L""));
        return;
    }
}

void JobNATCheckBase::StepFinish()
{
    CallContext* pCallContext =
        Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        m_socket.Close();
        if (m_callContext.GetState() == CallContext::CallInProgress)
            m_callContext.Cancel(CallContext::CallCanceled);
        SetToComplete();
        return;
    }
    m_socket.Close();
    if (m_callContext.GetState() == CallContext::CallFailed)
    {
        pCallContext->SignalFailure(m_callContext.GetResult());
        SetToComplete();
        return;
    }
    if (m_callContext.GetState() == CallContext::CallInProgress)
        m_callContext.Cancel(CallContext::CallCanceled);
    qResult result = HandleResult();
    if (result)
    {
        pCallContext->SetStateImpl(CallContext::CallSucceeded,
            qResult(NAT_CHECK_SUCCEEDED_RESULT), true);
        SetToComplete();
        return;
    }
    if (++m_uiAttempt < s_uiMaxAttemptCount)
    {
        SetStep(Step(static_cast<Step::Function>(&JobNATCheckBase::StepInitialize), L""));
        return;
    }
    pCallContext->SignalFailure(result);
    SetToComplete();
}

bool JobNATCheckBase::BindRandomPort()
{
    unsigned int uiAttempt = 0;
    do
    {
        m_localAddress.SetPortNumber(RootTransport::GetNextPortNumber());
        if (m_socket.Bind(&m_localAddress, true))
            return true;
        ++uiAttempt;
    } while (uiAttempt < BIND_ATTEMPT_COUNT);
    return false;
}

bool JobNATCheckBase::InitializeSocket()
{
    if (RootTransport::GetInstance() == 0 ||
        !RootTransport::GetInstance()->m_bNATCheckServerAddressesSet || !m_socket.Open(true))
        return false;
    if (m_uiAttempt != 0)
        m_localAddress.SetPortNumber(0);
    bool bBound = false;
    if (m_localAddress.GetPortNumber() != 0)
        bBound = m_socket.Bind(&m_localAddress, true);
    if (!bBound)
        bBound = BindRandomPort();
    return bBound;
}

void JobNATCheckBase::ExecuteNATCheck(unsigned int uiTimeout)
{
    m_deadline = static_cast<unsigned long long>(Time::GetTime()) + uiTimeout;
    SetStep(Step(static_cast<Step::Function>(&JobNATCheckBase::StepExecute), L""));
}

bool JobNATCheckBase::CanRetry()
{
    return m_uiAttempt + 1 < s_uiMaxAttemptCount;
}

bool JobNATCheckBase::SendNextQueuedMessage()
{
    if (m_queue.empty())
        return false;
    QueuedMessage& rMessage = m_queue.front();
    bool bSent = m_socket.Send(reinterpret_cast<const unsigned char*>(&rMessage.second),
                     sizeof(NATCheckMessage), rMessage.first) == SocketDriver::RESULT_SUCCESS;
    m_queue.pop_front();
    return bSent;
}

void JobNATCheckBase::StepExecute()
{
    CallContext* pCallContext =
        Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobNATCheckBase::StepFinish), L""));
        return;
    }
    Time now(Time::GetTime());
    if (now > m_deadline)
    {
        SetStep(Step(static_cast<Step::Function>(&JobNATCheckBase::StepFinish), L""));
        return;
    }
    bool bSent;
    do
    {
        bSent = SendNextQueuedMessage();
    } while (bSent);
    unsigned int uiReceived = 0;
    if (m_socket.RecvNonBlock(reinterpret_cast<unsigned char*>(&m_recvMessage),
            sizeof(NATCheckMessage), &uiReceived, &m_receivedFrom) == SocketDriver::RESULT_SUCCESS &&
        uiReceived == sizeof(NATCheckMessage))
    {
        m_recvMessage.FromNetworkByteOrder();
        unsigned int uiIndex = m_recvMessage.m_uiType - MESSAGE_TYPE_TEST1_REQUEST;
        if (uiIndex < RECEIVED_MESSAGE_COUNT)
        {
            m_aReceived[uiIndex].m_message = m_recvMessage;
            ++m_aReceived[uiIndex].m_uiCount;
        }
    }
    if (!Finished())
    {
        SetToWaiting(EXECUTE_WAIT);
        return;
    }
    SetStep(Step(static_cast<Step::Function>(&JobNATCheckBase::StepFinish), L""));
}

}
}
