#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RVCLIENTCORE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RVCLIENTCORE_HPP

#include <map>
#include <utility>

#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/PseudoSingleton.hpp"
#include "nn/nex/RefCountedObject.hpp"

namespace nn
{
namespace nex
{

class DateTime;
class NotificationEventManager;
class ServerProtocol;

class RVClientCore : public RefCountedObject
{
public:
    RVClientCore();
    virtual ~RVClientCore();

    static RVClientCore* AcquireInstance();
    static void ReleaseInstance();

    unsigned int RegisterServerProtocol(unsigned short uiProtocolID, ServerProtocol* pProtocol);
    unsigned int UnregisterServerProtocol(unsigned short uiProtocolID);
    ServerProtocol* GetServerProtocol(unsigned short uiProtocolID);

    void RegisterGlobalNotificationEventManager(NotificationEventManager* pManager);
    void UnregisterGlobalNotificationEventManager(NotificationEventManager* pManager);

    unsigned int GetGameServerTime(DateTime* pTime) const;
    void SetGameServerTime(DateTime& rTime);

    ProtocolRegistry* GetProtocolRegistry() const
    {
        return m_pProtocolRegistry;
    }

    NotificationEventManager* GetGlobalNotificationEventManager() const
    {
        return m_pGlobalNotificationEventManager;
    }

private:
    typedef std::pair<const unsigned short, ServerProtocol*> ServerProtocolEntry;
    typedef std::map<unsigned short, ServerProtocol*, std::less<unsigned short>,
                     MemAllocator<ServerProtocolEntry> > ServerProtocolMap;

    static RVClientCore* s_pInstance;

    ProtocolRegistry* m_pProtocolRegistry;
    PseudoSingleton m_instanceControl;
    NotificationEventManager* m_pGlobalNotificationEventManager;
    ServerProtocolMap m_serverProtocols;
    unsigned long long m_uiGameServerTimeBase;
    long long m_iGameServerEpochTime;
};

}
}

#endif
