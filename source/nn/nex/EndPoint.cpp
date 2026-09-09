#include "nn/nex/EndPoint.hpp"

#include "nn/nex/ConnectionOrientedStream.hpp"

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
