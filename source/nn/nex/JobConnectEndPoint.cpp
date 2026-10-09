#include "nn/nex/JobConnectEndPoint.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/ConnectionManager.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/NATTraversalEngine.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Router.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/StringStream.hpp"
#include "nn/nex/TransportAdapter.hpp"
#include "game/core/NS_JobConnectEndPoint_CPP.hpp"

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
    for (qList<StationURL>::iterator it = m_urls.begin(); it != m_urls.end(); ++it)
    {
    }
}

void JobConnectEndPoint::ProcessInTermination()
{
    CancelJob();
}

bool JobConnectEndPoint::ConnectCancelled()
{
    if (Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallContextId) == 0
        || Core::GetInstance()->GetTerminateImmediately() || m_cancelRequested)
        return true;

    return false;
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

void JobConnectEndPoint::SortURLs()
{
    if (Network::GetInstance() == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::ConnectionFailed), L""));
        return;
    }

    Network::GetInstance()->SortAndFilterTargetURLs(&m_urls);
    if (m_urls.size() == 0 || m_steps.size() == 0)
        m_timeout = 0;

    SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::SelectConnectionTechnique), L""));
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
    m_currentURL = m_urls.begin();
    m_isRouting = true;
    SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::TestCurrentURL), L""));
}

void JobConnectEndPoint::TryDirectConnect()
{
    m_currentURL = m_urls.begin();
    m_isRouting = false;
    if (m_urls.begin() == m_urls.end())
        SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::SelectConnectionTechnique), L""));
    else
        SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::TestCurrentURL), L""));
}

bool JobConnectEndPoint::IsRoutedURL(StationURL& rURL)
{
    InetAddress routeAddress;
    return RootTransport::GetInstance()->GetRouter()->GetRoutingStream()->GetRoutingTable()->Find(
        *rURL.GetInetAddress(), routeAddress);
}

StringStream& operator<<(StringStream& rStream, const StationURL& rURL)
{
    return rStream << rURL.GetURL();
}

void JobConnectEndPoint::RestoreOriginalPort()
{
    if (m_originalPort != 0)
    {
        m_currentURL->SetPortNumber(m_originalPort);
        m_originalPort = 0;
    }
}

bool JobConnectEndPoint::CheckForUpdatedURLImpl()
{
    if (Network::GetInstance()->GetNATTraversalEngine() != 0)
    {
        StationURL updatedURL;
        if (Network::GetInstance()->GetNATTraversalEngine()->GetUpdatedURL(*m_currentURL, &updatedURL)
            && m_currentURL->GetPortNumber() != updatedURL.GetPortNumber())
        {
            m_originalPort = m_currentURL->GetPortNumber();
            m_currentURL->SetPortNumber(updatedURL.GetPortNumber());
            return true;
        }
    }

    return false;
}

void JobConnectEndPoint::ResolveCurrentURL()
{
    m_natTraversalContext.SetTimeout(m_timeout);
    if (!m_isRouting)
        CheckForUpdatedURLImpl();

    g_transportAdapter->ResolveAddress(&m_natTraversalContext, &*m_currentURL);
    SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::WaitForURLResolution), L""));
}

void JobConnectEndPoint::WaitForURLResolution()
{
    if (m_natTraversalContext.GetState() != CallContext::CallInProgress)
    {
        if (m_natTraversalContext.GetState() == CallContext::CallSucceeded)
        {
            SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::TryCurrentURL), L""));
            m_natTraversalContext.Reset();
        }
        else
        {
            RestoreOriginalPort();
            ++m_currentURL;
            SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::TestCurrentURL), L""));
            m_natTraversalContext.Reset();
        }
    }
    else
    {
        SetToWaiting(WAIT_CONTEXT_URL_RESOLUTION);
    }
}

void JobConnectEndPoint::ReportConnectResult(qResult result)
{
    Trace(m_traceData);
    m_result = result;
    SetToReady();
    ReleaseRef();
}

void JobConnectEndPoint::TryCurrentURL()
{
    EndPoint* pEndPoint = m_pConnectionManager->OpenEndPoint(*m_currentURL);
    if (pEndPoint == 0)
    {
        RestoreOriginalPort();
        ++m_currentURL;
        SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::TestCurrentURL), L""));
    }
    else
    {
        m_pEndPoint = pEndPoint;
        if (m_isRouting)
            m_pConnectionManager->ConfigureEndPointForRouting(m_pEndPoint);

        m_result = m_pEndPoint->_Connect(m_pRequestBuffer, m_pResponseBuffer,
            NS_JobConnectEndPoint_CPP::ConnectCallback, UserContext(this), m_timeout);
        if (m_result == false)
        {
            m_pEndPoint->Close();
            m_pEndPoint = 0;
            RestoreOriginalPort();
            ++m_currentURL;
            SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::TestCurrentURL), L""));
        }
        else
        {
            AddRef();
            SetToSuspended();
            SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::ProcessConnectionResult), L""));
        }
    }
}

void JobConnectEndPoint::ProcessConnectionResult()
{
    if (m_result == true)
    {
        SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::ConnectionSucceeded), L""));
    }
    else
    {
        if (m_pEndPoint->IsConnected())
        {
            qResult result = m_pEndPoint->_Disconnect(NS_JobConnectEndPoint_CPP::DisconnectCallback,
                UserContext(this), ConnectionManager::s_uiDisconnectTimeout);
            if (result)
            {
                AddRef();
                SetToSuspended();
            }
        }

        SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::ProcessConnectionFailure), L""));
    }
}

void JobConnectEndPoint::CheckForUpdatedURL()
{
    if (Network::GetInstance() != 0 && Network::GetInstance()->GetNATTraversalEngine() != 0)
    {
        StationURL updatedURL;
        if (!CheckForUpdatedURLImpl())
        {
            RestoreOriginalPort();
            ++m_currentURL;
        }

        SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::TestCurrentURL), L""));
        return;
    }

    SetStep(Step(static_cast<Step::Function>(&JobConnectEndPoint::ConnectionFailed), L""));
}

void JobConnectEndPoint::SetFinalState(CallContext::State state, qResult result)
{
    CallContext* pCallContext =
        Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallContextId);
    if (pCallContext != 0)
        pCallContext->SetStateImpl(state, result, true);
}

void JobConnectEndPoint::ReportNATTraversalResult(bool bResult)
{
    if (Network::GetInstance() == 0 || Network::GetInstance()->GetNATTraversalEngine() == 0)
        return;

    if (m_flag194)
        return;

    NATTraversalEngine* pEngine = Network::GetInstance()->GetNATTraversalEngine();
    unsigned int uiRVConnectionID = 0;
    for (qList<StationURL>::iterator it = m_urls.begin(); it != m_urls.end(); ++it)
    {
        if (it->GetRVConnectionID() != 0)
        {
            uiRVConnectionID = it->GetRVConnectionID();
            break;
        }
    }

    if (pEngine != 0 && uiRVConnectionID != 0)
        pEngine->ReportNATTraversalResult(uiRVConnectionID, bResult, m_uiPingTime);
}

void JobConnectEndPoint::ConnectionFailed()
{
    ReportNATTraversalResult(false);

    if (m_flag1A0)
    {
        SetFinalState(CallContext::CallFailed, qResult(CONNECT_FAILED_FAULT_RESULT));
        SetToComplete();
    }
    else
    {
        SetFinalState(CallContext::CallFailed, qResult(CONNECT_FAILED_RESULT));
        SetToComplete();
    }
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

}
}
