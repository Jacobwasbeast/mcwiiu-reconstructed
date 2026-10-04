#include "nn/nex/JobConnectEndPoint.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/StationURL.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint64_t TRACE_CONNECTION = 0x4000;
const int WAIT_CONTEXT_URL_RESOLUTION = 100;

enum ConnectResult
{
    CONNECT_CANCELLED_RESULT = static_cast<int>(0x80010004u),
    CONNECT_FAILED_RESULT = static_cast<int>(0x80050002u),
    CONNECT_FAILED_FAULT_RESULT = static_cast<int>(0x8068000Bu),
    CONNECT_SUCCEEDED_RESULT = 0x10001
};

}

JobConnectEndPoint::~JobConnectEndPoint()
{
    if (m_pRequestBuffer != 0)
        m_pRequestBuffer->ReleaseRef();

    if (m_pResponseBuffer != 0)
        m_pResponseBuffer->ReleaseRef();
}

void JobConnectEndPoint::CancelJob()
{
    if (m_cancelRequested)
        return;

    Trace(TRACE_CONNECTION);
    m_cancelRequested = true;

    if (m_pEndPoint != 0 && m_pEndPoint->IsConnecting())
        m_pEndPoint->SetConnectionState(EndPoint::NotConnected);

    if (m_pCancelTarget != 0)
        m_pCancelTarget->CancelJob();
}

void JobConnectEndPoint::Trace(uint64_t)
{
    for (URLList::iterator it = m_urls.begin(); it != m_urls.end(); ++it)
    {
    }
}

void JobConnectEndPoint::ProcessInTermination()
{
    CancelJob();
}

void JobConnectEndPoint::TestSuspendedJobState()
{
    ConnectCancelled();
}

void JobConnectEndPoint::CheckExceptions()
{
    if (ConnectCancelled())
        SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::ConnectionCancelled), L""));
}

void JobConnectEndPoint::SelectConnectionTechnique()
{
    if (m_steps.empty())
    {
        SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::ConnectionFailed), L""));
        return;
    }

    SetStep(m_steps.front());
    m_steps.pop_front();
}

void JobConnectEndPoint::TryConnectViaRouting()
{
    m_isRouting = true;
    m_currentURL = m_urls.begin();
    SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::TestCurrentURL), L""));
}

void JobConnectEndPoint::TryDirectConnect()
{
    m_isRouting = false;
    m_currentURL = m_urls.begin();
    if (m_currentURL == m_urls.end())
        SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::SelectConnectionTechnique), L""));
    else
        SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::TestCurrentURL), L""));
}

void JobConnectEndPoint::RestoreOriginalPort()
{
    if (m_originalPort != 0)
    {
        m_currentURL->SetPortNumber(m_originalPort);
        m_originalPort = 0;
    }
}

void JobConnectEndPoint::WaitForURLResolution()
{
    if (m_natTraversalContext.GetState() == CallContext::CallInProgress)
    {
        SetToWaiting(WAIT_CONTEXT_URL_RESOLUTION);
        return;
    }

    if (m_natTraversalContext.GetState() == CallContext::CallSucceeded)
    {
        SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::TryCurrentURL), L""));
        m_natTraversalContext.Reset();
        return;
    }

    RestoreOriginalPort();
    ++m_currentURL;
    SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::TestCurrentURL), L""));
    m_natTraversalContext.Reset();
}

void JobConnectEndPoint::SetFinalState(CallContext::State state, qResult result)
{
    CallContext* pCallContext =
        Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallContextId);
    if (pCallContext != 0)
        pCallContext->SetStateImpl(state, result, true);
}

void JobConnectEndPoint::ConnectionCancelled()
{
    if (m_pEndPoint != 0)
    {
        m_pEndPoint->Close();
        m_pEndPoint = 0;
    }

    SetFinalState(CallContext::CallCanceled, qResult(CONNECT_CANCELLED_RESULT));
    SetToComplete();
}

void JobConnectEndPoint::ConnectionFailed()
{
    ReportNATTraversalResult(false);

    if (!m_flag1A0)
    {
        SetFinalState(CallContext::CallFailed, qResult(CONNECT_FAILED_RESULT));
        SetToComplete();
    }
    else
    {
        SetFinalState(CallContext::CallFailed, qResult(CONNECT_FAILED_FAULT_RESULT));
        SetToComplete();
    }
}

void JobConnectEndPoint::ConnectionSucceeded()
{
    ReportNATTraversalResult(true);

    if (m_currentURL->GetConnectionID() != 0)
        m_pEndPoint->SetConnectionID(m_currentURL->GetConnectionID());

    *m_ppEndPointOutput = m_pEndPoint;
    m_pEndPoint = 0;
    SetFinalState(CallContext::CallSucceeded, qResult(CONNECT_SUCCEEDED_RESULT));
    SetToComplete();
}

void JobConnectEndPoint::ReportConnectResult(qResult result)
{
    Trace(m_traceData);
    m_result = result;
    SetToReady();
    ReleaseRef();
}

}
}
