#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RVCLIENTCORE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RVCLIENTCORE_HPP

#include <cstdint>

#include "nn/nex/ProtocolRegistry.hpp"

namespace nn
{
namespace nex
{

class DateTime;
class NotificationEventManager;
class ServerProtocol;

class RVClientCore
{
public:
    RVClientCore();
    ~RVClientCore();

    std::uint32_t RegisterServerProtocol(std::uint16_t protocolId, ServerProtocol* protocol);
    static RVClientCore* AcquireInstance();
    static void ReleaseInstance();
    void RegisterGlobalNotificationEventManager(NotificationEventManager* manager);
    void UnregisterGlobalNotificationEventManager(NotificationEventManager* manager);
    std::uint32_t UnregisterServerProtocol(std::uint16_t protocolId);
    ServerProtocol* GetServerProtocol(std::uint16_t protocolId);
    std::uint32_t GetGameServerTime(DateTime* time) const;
    void SetGameServerTime(DateTime* time);

private:
    static RVClientCore* s_instance;
    ProtocolRegistry m_protocolRegistry;
    NotificationEventManager* m_globalNotificationEventManager;
    ServerProtocol* m_notificationProtocol;
    DateTime* m_gameServerTime;
};

}
}

#endif
