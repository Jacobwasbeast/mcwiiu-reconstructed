#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_MESH_LAYER_CONTROLLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_MESH_LAYER_CONTROLLER_HPP

#include "nn/pia/common/Job.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/transport/Station.hpp"

namespace nn { namespace pia { namespace common { class CryptoSetting; }}}

namespace nn { namespace pia { namespace session {

class IMatchmakeSession;

class MeshLayerController
{
public:
    MeshLayerController();
    virtual ~MeshLayerController();
    virtual Result Startup(const Session::StartupSetting& setting);
    virtual void Cleanup();
    virtual Result MeshStartup(IMatchmakeSession* matchmakeSession, bool skipBandwidthCheck);
    virtual void MeshCleanup();
    virtual Result CheckConnectedStatus() = 0;
    virtual void Trace(unsigned long long value) const;
    virtual void SetBufferSessionId(unsigned int sessionId);
    virtual void SetCurrentSessionId(unsigned int sessionId);
    virtual unsigned int GetMatchmakeSessionTotalStations() const;
    virtual unsigned int GetMatchmakeSessionTotalParticipants() const;
    virtual bool IsParticipated(unsigned int sessionId, unsigned long long stationId) const;
    virtual common::Job* GetJointSessionJobPtr();
    virtual bool IsJoinedUnknownSession(unsigned int sessionId, unsigned int bufferSessionId);
    virtual const transport::Station::PlayerInfo* GetPlayerInfo(unsigned char index) const;
    virtual unsigned char GetPlayerInfoSize() const;
    virtual bool IsPlayersCountedAsParticipants() const;
    virtual bool IsRegisteredPlayerInfo() const;
    virtual void SetLeaveFlag();
    virtual bool IsAllStationJoined(unsigned int sessionId);
    virtual bool IsHostMigrationEnabled() const;
    virtual bool IsStartedUp() const;

    void UpdateCryptoSetting(const common::CryptoSetting& cryptoSetting);

    transport::Station::PlayerInfo m_playerInfo[1];
    unsigned char m_playerInfoSize;
    unsigned char m_registeredPlayerInfo;
    unsigned char m_playersCountedAsParticipants;
    unsigned int m_field84;
    unsigned int m_field88;
    unsigned char m_hostMigrationEnabled;
    int m_bandwidthCheckMeasurementTime;
    unsigned int m_bandwidthCheckIpPacketSize;
    unsigned char m_skipBandwidthCheck;
    int m_bandwidthCheckMeasurementCount;
    unsigned int m_payloadSize;
    transport::Station::IdentificationToken m_identificationToken;
    unsigned char m_recentPlayRecordEnabled;
    unsigned char m_startedUp;
    unsigned int m_fielde0;
    unsigned int m_fielde4;
};

}}}

#endif
