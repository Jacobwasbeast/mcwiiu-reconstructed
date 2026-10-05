#include "nn/nex/SecureEndPoint.hpp"

#include "nn/nex/Core.hpp"
#include "nn/nex/JobConnectSecureEndPoint.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum ConnectResult
{
    CONNECT_PENDING_RESULT = 0x10001,
    CONNECT_TIMEOUT_RESULT = static_cast<int>(0x80050001u),
    CONNECT_INCOMPLETE_RESULT = static_cast<int>(0x80050008u),
    DISCONNECT_SUCCEEDED_RESULT = 0x10001
};

}

SecureEndPoint::SecureEndPoint(ConnectionOrientedStream* pStream, const StationURL* pStationURL)
    : EndPoint(pStream, pStationURL)
    , m_pAssociatedEndPoint(0)
    , m_pDisconnectCallback(0)
    , m_disconnectContext()
{
}

SecureEndPoint::~SecureEndPoint()
{
}

EndPoint* SecureEndPoint::GetAssociatedEndPoint()
{
    return m_pAssociatedEndPoint;
}

void SecureEndPoint::SetAssociatedEndPoint(EndPoint* pEndPoint)
{
    m_pAssociatedEndPoint = pEndPoint;
    pEndPoint->m_pSecureEndPoint = this;
    pEndPoint->RegisterEventHandler(m_pEventHandler);
}

unsigned int SecureEndPoint::GetRTT() const
{
    if (m_pAssociatedEndPoint == 0)
        return 0;

    return m_pAssociatedEndPoint->GetRTT();
}

unsigned int SecureEndPoint::GetRTTAverage() const
{
    if (m_pAssociatedEndPoint == 0)
        return 0;

    return m_pAssociatedEndPoint->GetRTTAverage();
}

qResult SecureEndPoint::RegisterEventHandler(EndPointEventHandler* pEventHandler)
{
    qResult result = EndPoint::RegisterEventHandler(pEventHandler);
    if (!(result == false) && m_pAssociatedEndPoint != 0)
    {
        result = m_pAssociatedEndPoint->RegisterEventHandler(m_pEventHandler);
        static_cast<void>(result == false);
    }

    return result;
}

bool SecureEndPoint::IsNotConnected()
{
    if (m_pAssociatedEndPoint == 0)
        return true;

    return m_pAssociatedEndPoint->IsNotConnected();
}

bool SecureEndPoint::IsConnecting()
{
    if (m_pAssociatedEndPoint == 0)
        return false;

    return m_pAssociatedEndPoint->IsConnecting();
}

bool SecureEndPoint::IsDisconnecting()
{
    if (m_pAssociatedEndPoint == 0)
        return false;

    return m_pAssociatedEndPoint->IsDisconnecting();
}

bool SecureEndPoint::IsFaulty()
{
    if (m_pAssociatedEndPoint == 0)
        return false;

    return m_pAssociatedEndPoint->IsFaulty();
}

bool SecureEndPoint::IsConnected()
{
    if (m_pAssociatedEndPoint == 0)
        return false;

    return m_pAssociatedEndPoint->IsConnected();
}

bool SecureEndPoint::PeerIsConnected()
{
    if (m_pAssociatedEndPoint == 0)
        return false;

    return m_pAssociatedEndPoint->PeerIsConnected();
}

bool SecureEndPoint::PeerIsDisconnected()
{
    if (m_pAssociatedEndPoint == 0)
        return true;

    return m_pAssociatedEndPoint->PeerIsDisconnected();
}

void SecureEndPoint::SetPeerConnected()
{
    if (m_pAssociatedEndPoint != 0)
        m_pAssociatedEndPoint->SetPeerConnected();
}

void SecureEndPoint::SetPeerDisconnected()
{
    if (m_pAssociatedEndPoint != 0)
        m_pAssociatedEndPoint->SetPeerDisconnected();
}

EndPoint::_ConnectionState SecureEndPoint::GetConnectionState() const
{
    if (m_pAssociatedEndPoint == 0)
        return NotConnected;

    return m_pAssociatedEndPoint->GetConnectionState();
}

bool SecureEndPoint::SetConnectionState(_ConnectionState state)
{
    if (m_pAssociatedEndPoint == 0)
        return false;

    return m_pAssociatedEndPoint->SetConnectionState(state);
}

void SecureEndPoint::SetKeepAliveTimeout(unsigned int uiTimeout)
{
    if (m_pAssociatedEndPoint != 0)
        m_pAssociatedEndPoint->SetKeepAliveTimeout(uiTimeout);
}

void SecureEndPoint::SetMaxSilenceTime(unsigned int uiSilenceTime)
{
    if (m_pAssociatedEndPoint != 0)
        m_pAssociatedEndPoint->SetMaxSilenceTime(uiSilenceTime);
}

unsigned int SecureEndPoint::GetKeepAliveTimeout()
{
    if (m_pAssociatedEndPoint == 0)
        return 0;

    return m_pAssociatedEndPoint->GetKeepAliveTimeout();
}

unsigned int SecureEndPoint::GetMaxSilenceTime()
{
    if (m_pAssociatedEndPoint == 0)
        return 0;

    return m_pAssociatedEndPoint->GetMaxSilenceTime();
}

bool SecureEndPoint::IsAnyPacketProcessed()
{
    if (m_pAssociatedEndPoint == 0)
        return false;

    return m_pAssociatedEndPoint->IsAnyPacketProcessed();
}

unsigned int SecureEndPoint::GetSignaledFaultError() const
{
    if (m_pAssociatedEndPoint == 0)
        return 0;

    return m_pAssociatedEndPoint->GetSignaledFaultError();
}

void SecureEndPoint::CompleteDisconnect()
{
    m_pAssociatedEndPoint->SetPrincipalID(0);
    m_pAssociatedEndPoint->SetConnectionID(0);
    SetPrincipalID(0);
    SetConnectionID(0);
    m_pAssociatedEndPoint->m_pSecureEndPoint = 0;
    m_pAssociatedEndPoint->Close();
    m_pAssociatedEndPoint = 0;
}

void SecureEndPoint::SetEncryptionKey(const Key& rKey)
{
    if (m_pAssociatedEndPoint != 0)
        m_pAssociatedEndPoint->SetEncryptionKey(rKey);
}

Key& SecureEndPoint::GetEncryptionKey()
{
    if (m_pAssociatedEndPoint == 0)
    {
        static Key s_emptyKey;
        return s_emptyKey;
    }

    return m_pAssociatedEndPoint->GetEncryptionKey();
}

qResult SecureEndPoint::_Connect(Buffer* pRequestData, Buffer* pResponseData,
    ConnectionCallback pCallback, const UserContext& rUserContext, unsigned int uiTimeout)
{
    JobConnectSecureEndPoint* pJob = new JobConnectSecureEndPoint(this, &m_stationURL,
        pRequestData, pResponseData, pCallback, rUserContext, uiTimeout);
    pJob->AddRef();
    Core::GetInstance()->GetScheduler()->Queue(pJob, false);

    if (pCallback != 0)
    {
        pJob->ReleaseRef();
        return qResult(static_cast<int>(CONNECT_PENDING_RESULT));
    }

    qResult result(static_cast<int>(CONNECT_TIMEOUT_RESULT));
    pJob->WaitForCompletion(uiTimeout);
    if (pJob->GetState() != Job::Complete)
    {
        result = qResult(static_cast<int>(CONNECT_INCOMPLETE_RESULT));
        pJob->ReleaseRef();
        return result;
    }

    result = pJob->GetResult();
    pJob->ReleaseRef();
    return result;
}

void SecureEndPoint::DisconnectCallback(EndPoint* pEndPoint, qResult result,
    const UserContext*)
{
    CriticalSection& csScheduler = Core::GetInstance()->GetScheduler()->GetCriticalSection();
    csScheduler.Enter();

    SecureEndPoint* pSecureEndPoint = pEndPoint->m_pSecureEndPoint;
    if (pSecureEndPoint != 0)
    {
        pSecureEndPoint->CompleteDisconnect();
        ConnectionCallback pCallback = pSecureEndPoint->m_pDisconnectCallback;
        pSecureEndPoint->m_pDisconnectCallback = 0;
        pCallback(pSecureEndPoint, result, &pSecureEndPoint->m_disconnectContext);
    }

    csScheduler.Leave();
}

qResult SecureEndPoint::_Disconnect(ConnectionCallback pCallback, const UserContext& rUserContext,
    unsigned int uiTimeout)
{
    qResult result(static_cast<int>(DISCONNECT_SUCCEEDED_RESULT));
    if (pCallback == 0)
    {
        result = m_pAssociatedEndPoint->_Disconnect(0, rUserContext, uiTimeout);
        CompleteDisconnect();
        return result;
    }

    m_pDisconnectCallback = pCallback;
    m_disconnectContext = rUserContext;
    UserContext context(this);
    result = m_pAssociatedEndPoint->_Disconnect(DisconnectCallback, context, uiTimeout);
    return result;
}

qResult SecureEndPoint::_Send(Buffer* pBuffer, unsigned int uiSize, bool bReliable,
    unsigned char uiSubStreamID, unsigned int uiFlags)
{
    EndPoint* pAssociatedEndPoint = m_pAssociatedEndPoint;
    CriticalSection& csScheduler = Core::GetInstance()->GetScheduler()->GetCriticalSection();
    csScheduler.Enter();
    qResult result =
        pAssociatedEndPoint->_Send(pBuffer, uiSize, bReliable, uiSubStreamID, uiFlags);
    csScheduler.Leave();
    return result;
}

unsigned long long SecureEndPoint::GetSupportedFunctionsFlags() const
{
    if (m_pAssociatedEndPoint == 0)
        return 0;

    return m_pAssociatedEndPoint->GetSupportedFunctionsFlags();
}

void SecureEndPoint::SignalFaultEvent(unsigned int uiError, bool bDisconnected)
{
    if (m_pAssociatedEndPoint != 0)
        m_pAssociatedEndPoint->SignalFaultEvent(uiError, bDisconnected);
}

}
}
