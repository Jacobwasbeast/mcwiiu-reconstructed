#include "nn/nex/ServiceClient.hpp"

#include "nn/nex/ClientProtocol.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/Protocol.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/Scheduler.hpp"

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

bool ServiceClient::IsConnected() const
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();

    if (m_pCredentials == 0)
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }

    EndPoint* pConnection = GetConnection(11);
    const bool bConnected = pConnection != 0 && pConnection->IsConnected();

    pScheduler->GetCriticalSection().Leave();
    return bConnected;
}

bool ServiceClient::SetDependentRvConnection(ProtocolCallContext* pCallContext) const
{
    Credentials* pCredentials = pCallContext->GetCredentials();
    if (pCredentials == 0)
        pCredentials = m_pCredentials;

    if (pCredentials != 0)
    {
        EndPoint* pConnection = pCredentials->GetConnection(11);
        if (pConnection != 0)
        {
            pCallContext->SetDependentConnection(pConnection, pConnection->GetConnectionID());
            return true;
        }
    }

    return false;
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

void ServiceClient::RegisterExtraProtocol(Protocol* pProtocol, unsigned short uiProtocolID)
{
    ClientProtocol* pClientProtocol = static_cast<ClientProtocol*>(pProtocol);

    pClientProtocol->SetDefaultCredentials(m_pCredentials);
    pClientProtocol->SetProtocolID(uiProtocolID);
    GetProtocolRegistry()->RegisterProtocol(uiProtocolID, pClientProtocol->CreateResponder());
}

ProtocolRegistry* ServiceClient::GetProtocolRegistry() const
{
    return RVClientCore::AcquireInstance()->GetProtocolRegistry();
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
