#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SERVICECLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SERVICECLIENT_HPP

#include <cstdint>

namespace nn
{
namespace nex
{

class Credentials;
class Protocol;
class ServiceClient
{
public:
    explicit ServiceClient(std::uint32_t serviceId);
    ~ServiceClient();

    std::uint32_t Bind(Credentials* credentials);
    std::uint32_t Unbind();
    void SetDefaultCredentials(Credentials* credentials);
    void SetCredentials(Credentials* credentials);
    std::uint32_t PrepareForConnection(Credentials* credentials);
    void RegisterExtraProtocol(Protocol* protocol, std::uint16_t protocolId);
    void RegisterExtraProtocol(Protocol* protocol);
    void RegisterProtocol(Protocol* protocol);
    std::uint32_t UnregisterProtocol(Protocol* protocol);

protected:
    std::uint32_t m_serviceId;
    void* m_serviceReference;
    bool m_threadRegistrationActive;
    void* m_serviceDescriptor;
    Credentials* m_credentials;
    Protocol* m_registeredProtocol;
};

}
}

#endif
