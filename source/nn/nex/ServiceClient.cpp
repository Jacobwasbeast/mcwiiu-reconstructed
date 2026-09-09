#include "nn/nex/ServiceClient.hpp"

#include "nn/nex/Credentials.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/Protocol.hpp"
#include "nn/nex/RVClientCore.hpp"

namespace nn
{
namespace nex
{

ServiceClient::ServiceClient(unsigned int uiServiceID)
    : m_uiServiceID(uiServiceID)
    , m_pCredentials(0)
    , m_bLocalLoopback(false)
{
    RVClientCore::AcquireInstance();
}

ServiceClient::~ServiceClient()
{
}

void ServiceClient::UseLocalLoopback(unsigned int, unsigned int)
{
}

bool ServiceClient::Bind(Credentials* pCredentials)
{
    return PrepareForConnection(pCredentials);
}

bool ServiceClient::Unbind()
{
    SetDefaultCredentials(0);
    return true;
}

void ServiceClient::ConnectionStateHasChanged()
{
}

bool ServiceClient::IsFaulty() const
{
    if (m_pCredentials == 0)
        return false;

    EndPoint* pConnection = GetConnection(11);
    return pConnection != 0 && pConnection->IsFaulty();
}

void ServiceClient::SetCredentials(Credentials* pCredentials)
{
    if (m_pCredentials != 0)
    {
        m_pCredentials->ReleaseRef();
        m_pCredentials = 0;
    }

    if (pCredentials == 0)
        return;

    m_pCredentials = pCredentials;
    m_pCredentials->AddRef();
}

void ServiceClient::SetDefaultCredentials(Credentials* pCredentials)
{
    UpdateProtocolsDefaultCredentials(pCredentials);
    SetCredentials(pCredentials);
}

bool ServiceClient::PrepareForConnection(Credentials* pCredentials)
{
    if (pCredentials == 0)
        return false;

    SetDefaultCredentials(pCredentials);
    return true;
}

void ServiceClient::RegisterProtocol(Protocol* pProtocol)
{
    RegisterExtraProtocol(pProtocol, pProtocol->GetProtocolID());
}

void ServiceClient::RegisterExtraProtocol(Protocol* pProtocol)
{
    RegisterExtraProtocol(pProtocol, pProtocol->GetProtocolID());
}

bool ServiceClient::UnregisterProtocol(Protocol*)
{
    return true;
}

EndPoint* ServiceClient::GetConnection(unsigned short uiConnectionType) const
{
    if (!m_bLocalLoopback && m_pCredentials != 0)
        return m_pCredentials->GetConnection(uiConnectionType);

    return 0;
}

}
}
