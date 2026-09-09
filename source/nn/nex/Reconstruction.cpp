#include "nn/nex/Reconstruction.hpp"

namespace nn
{
namespace nex
{

MatchmakeExtensionClient::MatchmakeExtensionClient()
    : MatchMakingClient()
    , m_protocolRegistered(false)
    , m_matchmakeExtensionProtocolClient(0)
    , m_stateF0(0)
    , m_stateF4(0)
{
    m_matchmakeExtensionProtocolClient.SetProtocolID(0x6d);
}

int MatchmakeExtensionClient::Bind(Credentials* credentials)
{
    const std::uint32_t result = ServiceClient::Bind(credentials);
    if (result != 0 && !m_protocolRegistered)
    {
        RegisterProtocol(&m_matchmakeExtensionProtocolClient);
        m_protocolRegistered = true;
    }
    return static_cast<int>(result);
}

void MatchmakeExtensionClient::Unbind()
{
    const std::uint32_t result = ServiceClient::Unbind();
    if (result != 0 && m_protocolRegistered)
    {
        UnregisterProtocol(&m_matchmakeExtensionProtocolClient);
    }
    m_protocolRegistered = false;
}

std::uint32_t BackEndServices::RegisterNotificationEventHandler(
    NotificationEventHandler* handler)
{
    if (m_notificationEventManager == 0)
    {
        return 0;
    }
    return m_notificationEventManager->RegisterNotificationEventHandler(handler);
}

std::uint32_t BackEndServices::UnregisterNotificationEventHandler(
    NotificationEventHandler* handler)
{
    if (m_notificationEventManager == 0)
    {
        return 0;
    }
    return m_notificationEventManager->UnregisterNotificationEventHandler(handler);
}

Credentials* BackEndServices::GetCredentials()
{
    return m_credentials;
}

}
}
