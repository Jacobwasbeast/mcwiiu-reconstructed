#include "nn/nex/EndPoint.hpp"

#include "nn/nex/ConnectionOrientedStream.hpp"
#include "nn/nex/TransportAdapter.hpp"

namespace nn
{
namespace nex
{

namespace
{

const wchar_t* const s_pConnectionStateStrings[] =
{
    L"NotConnected",
    L"Connecting",
    L"Connected",
    L"Disconnecting",
    L"Faulty"
};

const unsigned int CONNECTION_STATE_STRING_COUNT = 5;

const int ENDPOINT_RESULT_SUCCESS = 0x00010001;
const int ENDPOINT_RESULT_ALREADY_REGISTERED = 0x8001000E;

const unsigned int KEEP_ALIVE_DISABLED = 0xFFFFFFFF;

}

EndPointInfoInterface::~EndPointInfoInterface()
{
}

EndPoint::EndPoint(ConnectionOrientedStream* pStream, const StationURL* pStationURL)
    : m_pStream(pStream)
    , m_stationURL(*pStationURL)
    , m_pEventHandler(0)
    , m_pSecureEndPoint(0)
    , m_uiConnectionID(0)
    , m_uiPrincipalID(0)
    , m_iOpenCount(0)
    , m_BroadcastState(BroadcastDisabled)
    , m_uiCtorFlag(0)
{
}

EndPoint::~EndPoint()
{
    SetConnectionID(0);
    SetPrincipalID(0);
    g_transportAdapter->ReleaseAddress(m_stationURL);
}

void EndPoint::DisableKeepAlive()
{
    SetKeepAliveTimeout(KEEP_ALIVE_DISABLED);
}

qResult EndPoint::RegisterEventHandler(EndPointEventHandler* pEventHandler)
{
    if (m_pEventHandler != 0)
    {
        return qResult(ENDPOINT_RESULT_ALREADY_REGISTERED);
    }

    m_pEventHandler = pEventHandler;
    return qResult(ENDPOINT_RESULT_SUCCESS);
}

void EndPoint::SetErrorHolder(FirstErrorHolder* pErrorHolder)
{
    m_pErrorHolder = pErrorHolder;
}

void EndPoint::UnsetErrorHolder()
{
    m_pErrorHolder = 0;
}

void EndPoint::SignalFaultEvent(unsigned int, bool)
{
}

void EndPoint::Open()
{
    m_pStream->OpenEndPoint(this);
}

void EndPoint::Close()
{
    m_pStream->CloseEndPoint(this);
}

void EndPoint::SetConnectionID(unsigned int uiConnectionID)
{
    if (m_pStream != 0)
    {
        if (m_uiConnectionID != 0)
        {
            m_pStream->RemoveCIDEndPointAssociation(m_uiConnectionID, this);
        }

        if (uiConnectionID != 0)
        {
            m_pStream->AddCIDEndPointAssociation(uiConnectionID, this);
        }
    }

    m_uiConnectionID = uiConnectionID;
}

void EndPoint::SetPrincipalID(unsigned int uiPrincipalID)
{
    if (m_pStream != 0)
    {
        if (m_uiPrincipalID != 0)
        {
            m_pStream->RemovePIDEndPointAssociation(m_uiPrincipalID, this);
        }

        if (uiPrincipalID != 0)
        {
            m_pStream->AddPIDEndPointAssociation(uiPrincipalID, this);
        }
    }

    m_uiPrincipalID = uiPrincipalID;
}

void EndPoint::_Open()
{
    m_iOpenCount = m_iOpenCount + 1;
}

void EndPoint::_Close()
{
    m_iOpenCount = m_iOpenCount - 1;
}

void EndPoint::SetBroadcastState(_BroadcastState state)
{
    m_BroadcastState = state;
}

const wchar_t* EndPoint::GetConnectionStateString() const
{
    const unsigned int uiState = static_cast<unsigned int>(GetConnectionState());

    if (uiState < CONNECTION_STATE_STRING_COUNT)
    {
        return s_pConnectionStateStrings[uiState];
    }

    return L"Unknown state";
}

}
}
