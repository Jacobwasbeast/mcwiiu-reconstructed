#include "nn/nex/JobConnectSecureEndPoint.hpp"

#include "nn/nex/BitStream.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/JobConnectEndPoint.hpp"
#include "nn/nex/KerberosAuthentication.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/SecureEndPoint.hpp"
#include "nn/nex/Ticket.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum ConnectResult
{
    CONNECT_FAILED_RESULT = static_cast<int>(0x80050001u),
    CONNECT_INVALID_URL_RESULT = static_cast<int>(0x80050003u),
    CONNECT_INVALID_RESPONSE_RESULT = static_cast<int>(0x8005000Au)
};

}

void JobConnectSecureEndPoint::ParseURL()
{
    m_uiConnectionID = m_stationURL.GetConnectionID();
    m_uiPrincipalID = m_stationURL.GetPrincipalID();

    bool bHasAddress = false;
    if (m_stationURL.GetAddress().GetLength() != 0 && m_stationURL.GetPortNumber() != 0 &&
        m_stationURL.GetStreamID() != 0)
    {
        bHasAddress = true;
    }

    if (bHasAddress)
    {
        StationURL url;
        url.SetInetAddress(m_stationURL.GetInetAddress());
        url.SetStreamID(m_stationURL.GetStreamID());

        ConnectionData connectionData;
        connectionData.m_StationURL = url;
        connectionData.m_uiValue = m_uiConnectionID;
        m_connectionData.push_back(connectionData);
    }

    if (m_uiPrincipalID == 0 && m_uiConnectionID == 0)
    {
        m_result = qResult(static_cast<int>(CONNECT_INVALID_URL_RESULT));
        m_uiStep = STEP_COMPLETE;
        SetToComplete();
    }
}

bool JobConnectSecureEndPoint::ReadyToPerformConnect()
{
    if (m_uiPrincipalID != 0 && !m_connectionData.empty() && m_pTicket != 0)
        return true;

    return false;
}

void JobConnectSecureEndPoint::CheckConnectionData()
{
    m_uiStep = STEP_REQUEST_CONNECTION_DATA;
    if (ReadyToPerformConnect())
        m_uiStep = STEP_PERFORM_CONNECT;
}

void JobConnectSecureEndPoint::BuildListURLs(qList<StationURL>* pURLs)
{
    for (qList<ConnectionData>::iterator it = m_connectionData.begin();
        it != m_connectionData.end(); ++it)
    {
        StationURL url(it->m_StationURL);
        url.SetConnectionID(it->m_uiValue);
        pURLs->push_back(url);
    }
}

void JobConnectSecureEndPoint::ProcessConnectResult()
{
    if (m_pConnectJob != 0)
    {
        m_pConnectJob->ReleaseRef();
        m_pConnectJob = 0;
    }

    if (m_result == true)
    {
        m_pResponseStream->AdjustLength();
        if (!KerberosAuthentication::ValidateConnectionResponse(m_pResponseStream, m_uiNonce))
            m_result = qResult(static_cast<int>(CONNECT_INVALID_RESPONSE_RESULT));

        if (m_pSecureEndPoint != 0)
            m_pSecureEndPoint->SetEncryptionKey(m_pTicket->GetKey());
    }

    m_uiStep = STEP_COMPLETE;
    SetToComplete();
}

void JobConnectSecureEndPoint::ConnectCompletion(CallContext* pCallContext)
{
    CriticalSection& csScheduler = Core::GetInstance()->GetScheduler()->GetCriticalSection();
    csScheduler.Enter();

    if (pCallContext->GetState() != CallContext::CallSucceeded)
    {
        m_result = qResult(static_cast<int>(CONNECT_FAILED_RESULT));
        Resume();
        csScheduler.Leave();
        return;
    }

    m_pSecureEndPoint->SetAssociatedEndPoint(m_pEndPoint);
    m_pEndPoint->SetPrincipalID(m_uiPrincipalID);
    m_pSecureEndPoint->SetPrincipalID(m_uiPrincipalID);
    m_pSecureEndPoint->SetConnectionID(m_pEndPoint->GetConnectionID());
    Resume();
    csScheduler.Leave();
}

void JobConnectSecureEndPoint::ConnectCompletionCallback(CallContext* pCallContext,
    const UserContext* pUserContext)
{
    static_cast<JobConnectSecureEndPoint*>(pUserContext->GetPointer())
        ->ConnectCompletion(pCallContext);
}

void JobConnectSecureEndPoint::ExecuteStep()
{
    switch (m_uiStep)
    {
    case STEP_PARSE_URL:
        ParseURL();
        m_uiStep = STEP_CHECK_CONNECTION_DATA;
        break;
    case STEP_REQUEST_CONNECTION_DATA:
        RequestConnectionData();
        break;
    case STEP_CHECK_CONNECTION_DATA:
        CheckConnectionData();
        break;
    case STEP_PERFORM_CONNECT:
        PerformConnect();
        break;
    case STEP_PROCESS_CONNECT_RESULT:
        ProcessConnectResult();
        break;
    }
}

void JobConnectSecureEndPoint::Execute()
{
    do
    {
        ExecuteStep();
    } while (GetState() == Running);

    if (GetState() == Complete && m_pCallback != 0)
        m_pCallback(m_pSecureEndPoint, m_result, &m_userContext);
}

void JobConnectSecureEndPoint::CancelJob()
{
    if (m_pConnectJob != 0)
        m_pConnectJob->CancelJob();
}

}
}
