#include "nn/pia/lan/LanMatchMeshLayerController.hpp"

#include <coreinit/memory.h>

#include "nn/pia/lan/LanFacade.hpp"
#include "nn/pia/lan/LanSessionStartupSetting.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/transport/Station.hpp"
#include "platform/wiiu/pead.hpp"
#include "nn/pia/transport/StationManager.hpp"

namespace nn { namespace pia { namespace lan {

namespace
{
enum
{
    cResultDisconnectedNetwork = 0xc406,
    cResultDisconnectedSession = 0x8c56,
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cPayloadSize = 0x554,
    cCryptoKeySize = 0x10
};
}

LanMatchMeshLayerController::LanMatchMeshLayerController()
    : session::MeshLayerController()
{
    m_matchmakeSession = 0;
}

LanMatchMeshLayerController::~LanMatchMeshLayerController()
{
}

Result LanMatchMeshLayerController::Startup(const session::Session::StartupSetting& baseSetting)
{
    const LanSessionStartupSetting& setting = static_cast<const LanSessionStartupSetting&>(baseSetting);
    const Result playerInfoResult = session::g_piaMesh->CheckPlayerInfo(setting.m_playerInfo[0]);
    m_payloadSize = cPayloadSize;
    m_registeredPlayerInfo = false;
    m_playerInfoSize = 0;
    ResetPlayerInfo();
    if (playerInfoResult == 0)
    {
        transport::Station::PlayerInfo& playerInfo = m_playerInfo[m_playerInfoSize];
        m_registeredPlayerInfo = true;
        playerInfo.m_name.Copy(setting.m_playerInfo[0].m_name);
        playerInfo.m_language = setting.m_playerInfo[0].m_language;
        playerInfo.m_field70 = setting.m_playerInfo[0].m_field70;
        playerInfo.m_field78 = setting.m_playerInfo[0].m_field78;
        m_playerInfo[m_playerInfoSize].m_name.Trace(0x4000000ULL);
        ++m_playerInfoSize;
    }
    if (m_playerInfoSize == 0)
        m_playerInfoSize = 1;
    m_playersCountedAsParticipants = setting.m_playersCountedAsParticipants;
    m_cryptoSetting.m_mode = setting.m_cryptoSetting.m_mode;
    pead::Random random;
    for (unsigned int index = 0; index < cCryptoKeySize; ++index)
        m_cryptoSetting.m_key[index] = static_cast<unsigned char>(random.getU32());
    m_bandwidthCheckMeasurementCount = 0;
    m_recentPlayRecordEnabled = false;
    m_hostMigrationEnabled = true;
    m_bandwidthCheckIpPacketSize = 0;
    m_skipBandwidthCheck = true;
    m_bandwidthCheckMeasurementTime = 0;

    LanFacade* facade = LanFacade::GetInstance();
    if (facade == 0)
        return cResultInvalidState;
    if (setting.m_cryptoSetting.m_mode == 0)
        return cResultInvalidArgument;

    unsigned char zeroKey[cCryptoKeySize];
    OSBlockSet(zeroKey, 0, sizeof(zeroKey));
    bool keyIsZero = true;
    for (unsigned int index = 0; index < cCryptoKeySize; ++index)
    {
        if (setting.m_cryptoSetting.m_key[index] != zeroKey[index])
        {
            keyIsZero = false;
            break;
        }
    }
    if (keyIsZero)
        return cResultInvalidArgument;

    Result result = facade->Startup(setting.m_cryptoSetting);
    if (result != 0)
        return result;
    result = facade->StartLanSession();
    if (result != 0)
        return result;
    m_connectionStatus = session::Session::cConnectionStatusConnected;
    return session::MeshLayerController::Startup(baseSetting);
}

void LanMatchMeshLayerController::MeshCleanup()
{
    session::MeshLayerController::MeshCleanup();
    m_matchmakeSession = 0;
}

void LanMatchMeshLayerController::Cleanup()
{
    if (LanFacade::GetInstance() != 0)
    {
        LanFacade::GetInstance()->StopLanSession();
        LanFacade::GetInstance()->Cleanup();
    }
    LanMatchMeshLayerController::MeshCleanup();
    session::MeshLayerController::Cleanup();
}

Result LanMatchMeshLayerController::MeshStartup(session::IMatchmakeSession* matchmakeSession,
                                                bool skipBandwidthCheck)
{
    m_matchmakeSession = matchmakeSession;
    return session::MeshLayerController::MeshStartup(matchmakeSession, skipBandwidthCheck);
}

Result LanMatchMeshLayerController::CheckConnectedStatus()
{
    if (m_connectionStatus == session::Session::cConnectionStatusDisconnectedNetwork)
        return cResultDisconnectedNetwork;
    if (m_connectionStatus == session::Session::cConnectionStatusDisconnectedSession)
        return cResultDisconnectedSession;
    return 0;
}

void LanMatchMeshLayerController::Trace(unsigned long long) const
{
}

unsigned int LanMatchMeshLayerController::GetMatchmakeSessionTotalStations() const
{
    return session::g_piaSession->GetStationNum();
}

unsigned int LanMatchMeshLayerController::GetMatchmakeSessionTotalParticipants() const
{
    unsigned short total = 0;
    common::ListNode* sentinel = session::g_piaSession->m_validStationIdList.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        transport::Station* station = transport::StationManager::s_pInstance->GetStation(
            *session::Session::StationIdList::ValueOf(node));
        unsigned char count;
        if (station != 0 && station->GetParticipantCount(&count) == 0)
            total += count;
    }
    return total;
}

bool LanMatchMeshLayerController::IsParticipated(unsigned int, unsigned long long principalId) const
{
    const transport::StationManager* manager = transport::StationManager::s_pInstance;
    transport::Station* const* begin = manager->m_stations.data;
    for (transport::Station* const* it = begin; it != begin + manager->m_stations.size; ++it)
    {
        unsigned long long stationPrincipalId;
        if ((*it)->GetPrincipalId(&stationPrincipalId) == 0 && stationPrincipalId == principalId)
            return true;
    }
    return false;
}

void LanMatchMeshLayerController::SetConnectionStatusDisconnected()
{
    m_connectionStatus = session::Session::cConnectionStatusDisconnectedNetwork;
}

}}}
