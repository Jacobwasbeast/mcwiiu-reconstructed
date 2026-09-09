#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_MESH_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_MESH_HPP

#include "nn/pia.h"

namespace nn { namespace pia { namespace session {

class KickoutManageJob;

class Mesh
{
public:
    unsigned int GetHostMigrationMode() const;
    bool IsRecentPlayRecordEnabled() const;
    bool IsBandwidthCheckProtocolSetupParamValid(int measurementTime,
                                                 unsigned int ipPacketSize,
                                                 int measurementCount) const;
    Result CheckPlayerInfo(const transport::Station::PlayerInfo& info) const;
    Result CheckPlayerInfoSetting(const transport::Station::PlayerInfo* infos,
                                  unsigned char count) const;
    void SetSyncClockPulseInterval(int interval);
    void SetSkipCleanupReport(bool skip);
    Result CheckConnectionError() const;
    unsigned int GetDisconnectReason() const;
    KickoutManageJob* GetKickoutManageJobPtr();

    KickoutManageJob* m_kickoutManageJob;
    unsigned int m_meshState;
    unsigned int m_disconnectReason;
    unsigned int m_hostMigrationMode;
    unsigned char m_recentPlayRecordEnabled;
    transport::ProtocolId m_syncClockProtocolId;
    unsigned char m_skipCleanupReport;
};

extern Mesh* g_piaMesh;

}}}

#endif
