#include "nn/nex/ServiceClient.hpp"

#include "nn/nex/Protocol.hpp"
#include "nn/nex/RVClientCore.hpp"

namespace nn
{
namespace nex
{

ServiceClient::ServiceClient(std::uint32_t serviceId)
    : m_serviceId(serviceId)
    , m_serviceReference(0)
    , m_threadRegistrationActive(false)
    , m_serviceDescriptor(0)
    , m_credentials(0)
    , m_registeredProtocol(0)
{
    RVClientCore::AcquireInstance();
}

ServiceClient::~ServiceClient()
{
}

std::uint32_t ServiceClient::Bind(Credentials* credentials)
{
    if (PrepareForConnection(credentials) == 0)
        return 0;
    return 1;
}

std::uint32_t ServiceClient::Unbind()
{
    m_registeredProtocol = 0;
    SetDefaultCredentials(0);
    return 1;
}

void ServiceClient::SetDefaultCredentials(Credentials* credentials)
{
    SetCredentials(credentials);
}

void ServiceClient::SetCredentials(Credentials* credentials)
{
    m_credentials = credentials;
}

std::uint32_t ServiceClient::PrepareForConnection(Credentials* credentials)
{
    if (credentials == 0)
        return 0;

    SetDefaultCredentials(credentials);
    return 1;
}

void ServiceClient::RegisterExtraProtocol(Protocol* protocol, std::uint16_t)
{
    m_registeredProtocol = protocol;
}

void ServiceClient::RegisterExtraProtocol(Protocol* protocol)
{
    RegisterExtraProtocol(protocol, protocol == 0 ? 0 : protocol->GetProtocolID());
}

void ServiceClient::RegisterProtocol(Protocol* protocol)
{
    RegisterExtraProtocol(protocol, protocol == 0 ? 0 : protocol->GetProtocolID());
}

std::uint32_t ServiceClient::UnregisterProtocol(Protocol* protocol)
{
    (void)protocol;
    m_registeredProtocol = 0;
    return 1;
}

}
}
