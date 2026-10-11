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
    if (pTime == 0 || m_gameServerClock.m_timeBase == 0)
        return 0;

    const unsigned long long uiNow = Time::GetTime();
    const long long iElapsed = static_cast<long long>(uiNow - m_gameServerClock.m_timeBase) / 1000;
    const long long iEpochTime = static_cast<long long>(m_gameServerClock.m_epochTime);
    *pTime = DateTime::FromEpochTime(iEpochTime + iElapsed);
    return 1;
}

void RVClientCore::SetGameServerTime(DateTime& rTime)
{
    m_gameServerClock.m_epochTime = static_cast<long long>(rTime.ToEpochTime());
    m_gameServerClock.m_timeBase = Time::GetTime();
}

}
}
