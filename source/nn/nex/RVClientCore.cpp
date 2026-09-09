#include "nn/nex/RVClientCore.hpp"

namespace nn
{
namespace nex
{

RVClientCore* RVClientCore::s_instance = 0;

RVClientCore::RVClientCore()
    : m_protocolRegistry()
    , m_globalNotificationEventManager(0)
    , m_notificationProtocol(0)
    , m_gameServerTime(0)
{
}

RVClientCore::~RVClientCore()
{
    s_instance = 0;
}

std::uint32_t RVClientCore::RegisterServerProtocol(std::uint16_t protocolId, ServerProtocol* protocol)
{
    if (protocol == 0)
        return 0;
    m_protocolRegistry.RegisterProtocol(protocolId, reinterpret_cast<Protocol*>(protocol));
    return 1;
}

RVClientCore* RVClientCore::AcquireInstance()
{
    if (s_instance == 0)
        s_instance = new RVClientCore();
    return s_instance;
}

void RVClientCore::ReleaseInstance()
{
    delete s_instance;
    s_instance = 0;
}

void RVClientCore::RegisterGlobalNotificationEventManager(NotificationEventManager* manager)
{
    m_globalNotificationEventManager = manager;
}

void RVClientCore::UnregisterGlobalNotificationEventManager(NotificationEventManager* manager)
{
    if (m_globalNotificationEventManager == manager)
        m_globalNotificationEventManager = 0;
}

std::uint32_t RVClientCore::UnregisterServerProtocol(std::uint16_t protocolId)
{
    Protocol* protocol = m_protocolRegistry.GetServerProtocol(protocolId);
    if (protocol == 0)
        return 0;
    m_protocolRegistry.UnregisterProtocol(protocol);
    return 1;
}

ServerProtocol* RVClientCore::GetServerProtocol(std::uint16_t protocolId)
{
    return reinterpret_cast<ServerProtocol*>(m_protocolRegistry.GetServerProtocol(protocolId));
}

void RVClientCore::SetGameServerTime(DateTime* time)
{
    m_gameServerTime = time;
}

}
}
