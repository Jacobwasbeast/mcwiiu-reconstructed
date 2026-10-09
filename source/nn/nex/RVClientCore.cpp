#include "nn/nex/RVClientCore.hpp"

#include "nn/nex/DateTime.hpp"
#include "nn/nex/GlobalNotificationEventManager.hpp"
#include "nn/nex/ServerProtocol.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

RVClientCore* RVClientCore::s_pInstance = 0;

RVClientCore::RVClientCore()
    : RefCountedObject()
    , m_instanceControl(ClassID)
    , m_serverProtocols()
    , m_uiGameServerTimeBase(0)
    , m_iGameServerEpochTime(0)
{
    m_instanceControl.SetDelegatorInstance(this);
    Core::AcquireInstance();
    m_pProtocolRegistry = new ProtocolRegistry();
    m_pGlobalNotificationEventManager = new GlobalNotificationEventManager();
    m_pGlobalNotificationEventManager->SetFlag(NOTIFICATION_PROTOCOL_FLAG);
    RegisterServerProtocol(NOTIFICATION_EVENT_PROTOCOL_ID, m_pGlobalNotificationEventManager);
}

RVClientCore::~RVClientCore()
{
    delete m_pProtocolRegistry;
    delete m_pGlobalNotificationEventManager;
    Core::ReleaseInstance();
}

RVClientCore* RVClientCore::AcquireInstance()
{
    if (s_pInstance == 0)
        s_pInstance = new RVClientCore;
    return s_pInstance;
}

void RVClientCore::ReleaseInstance()
{
    delete s_pInstance;
    s_pInstance = 0;
}

unsigned int RVClientCore::RegisterServerProtocol(unsigned short uiProtocolID,
                                                  ServerProtocol* pProtocol)
{
    if (m_serverProtocols.find(uiProtocolID) != m_serverProtocols.end())
        return 0;
    m_serverProtocols[uiProtocolID] = pProtocol;
    return 1;
}

void RVClientCore::RegisterGlobalNotificationEventManager(NotificationEventManager* pManager)
{
    m_pGlobalNotificationEventManager->RegisterGlobalNotificationEventManager(pManager);
}

void RVClientCore::UnregisterGlobalNotificationEventManager(NotificationEventManager* pManager)
{
    m_pGlobalNotificationEventManager->UnregisterGlobalNotificationEventManager(pManager);
}

unsigned int RVClientCore::UnregisterServerProtocol(unsigned short uiProtocolID)
{
    ServerProtocolMap::iterator entry = m_serverProtocols.find(uiProtocolID);
    if (entry != m_serverProtocols.end())
        m_serverProtocols.erase(entry);
    return 1;
}

ServerProtocol* RVClientCore::GetServerProtocol(unsigned short uiProtocolID)
{
    ServerProtocolMap::iterator entry = m_serverProtocols.find(uiProtocolID);
    if (entry == m_serverProtocols.end())
        return 0;
    return entry->second;
}

unsigned int RVClientCore::GetGameServerTime(DateTime* pTime) const
{
    if (pTime == 0 || m_uiGameServerTimeBase == 0)
        return 0;

    const unsigned long long uiNow = Time::GetTime();
    const long long iElapsed = static_cast<long long>(uiNow - m_uiGameServerTimeBase) / 1000;
    *pTime = DateTime::FromEpochTime(m_iGameServerEpochTime + iElapsed);
    return 1;
}

void RVClientCore::SetGameServerTime(DateTime& rTime)
{
    m_iGameServerEpochTime = static_cast<long long>(rTime.ToEpochTime());
    m_uiGameServerTimeBase = Time::GetTime();
}

}
}
