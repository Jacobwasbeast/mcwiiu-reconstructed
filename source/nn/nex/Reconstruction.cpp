#include "nn/nex/Reconstruction.hpp"

namespace nn
{
namespace nex
{

MatchmakeExtensionClient::MatchmakeExtensionClient()
    : MatchMakingClient()
    , m_protocolRegistered(false)
    , m_matchmakeExtensionProtocolClient(0)
    , m_uiLastProgressScoreTime(0)
{
    m_matchmakeExtensionProtocolClient.SetProtocolID(0x6d);
}

MatchmakeExtensionClient::~MatchmakeExtensionClient()
{
    if (m_protocolRegistered)
    {
        UnregisterProtocol(&m_matchmakeExtensionProtocolClient);
    }
}

bool MatchmakeExtensionClient::Bind(Credentials* pCredentials)
{
    const bool bBound = ServiceClient::Bind(pCredentials);
    if (bBound && !m_protocolRegistered)
    {
        RegisterProtocol(&m_matchmakeExtensionProtocolClient);
        m_protocolRegistered = true;
    }
    return bBound;
}

bool MatchmakeExtensionClient::Unbind()
{
    const bool bUnbound = ServiceClient::Unbind();
    if (bUnbound && m_protocolRegistered)
    {
        UnregisterProtocol(&m_matchmakeExtensionProtocolClient);
    }
    m_protocolRegistered = false;
    return bUnbound;
}

uint32_t BackEndServices::RegisterNotificationEventHandler(
    NotificationEventHandler* handler)
{
    if (m_notificationEventManager == 0)
    {
        return 0;
    }
    return m_notificationEventManager->RegisterNotificationEventHandler(handler);
}

uint32_t BackEndServices::UnregisterNotificationEventHandler(
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
