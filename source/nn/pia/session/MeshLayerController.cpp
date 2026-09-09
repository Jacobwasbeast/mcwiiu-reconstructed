#include "nn/pia/session/MeshLayerController.hpp"

namespace nn { namespace pia { namespace session {

void MeshLayerController::Trace(unsigned long long) const
{
}

unsigned int MeshLayerController::GetMatchmakeSessionTotalStations() const
{
    return 0;
}

unsigned int MeshLayerController::GetMatchmakeSessionTotalParticipants() const
{
    return 0;
}

bool MeshLayerController::IsParticipated(unsigned int, unsigned long long) const
{
    return true;
}

bool MeshLayerController::IsJoinedUnknownSession(unsigned int, unsigned int)
{
    return false;
}

const transport::Station::PlayerInfo* MeshLayerController::GetPlayerInfo(unsigned char index) const
{
    return &m_playerInfo[index];
}

unsigned char MeshLayerController::GetPlayerInfoSize() const
{
    return m_playerInfoSize;
}

bool MeshLayerController::IsPlayersCountedAsParticipants() const
{
    return m_playersCountedAsParticipants != 0;
}

bool MeshLayerController::IsRegisteredPlayerInfo() const
{
    return m_registeredPlayerInfo != 0;
}

void MeshLayerController::SetLeaveFlag()
{
}

bool MeshLayerController::IsHostMigrationEnabled() const
{
    return m_hostMigrationEnabled != 0;
}

bool MeshLayerController::IsStartedUp() const
{
    return m_startedUp != 0;
}

}}}
