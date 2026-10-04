#include "nn/pia/lan/LanMatchMeshLayerController.hpp"

namespace nn { namespace pia { namespace lan {

LanMatchMeshLayerController::LanMatchMeshLayerController()
{
}

LanMatchMeshLayerController::~LanMatchMeshLayerController()
{
}

Result LanMatchMeshLayerController::Startup(const session::Session::StartupSetting&)
{
    return 0;
}

void LanMatchMeshLayerController::MeshCleanup()
{
}

void LanMatchMeshLayerController::Cleanup()
{
}

void LanMatchMeshLayerController::MeshStartup(session::IMatchmakeSession*, bool)
{
}

uint64_t LanMatchMeshLayerController::CheckConnectedStatus()
{
    return 0;
}

void LanMatchMeshLayerController::Trace(unsigned long long) const
{
}

void LanMatchMeshLayerController::GetMatchmakeSessionTotalStations() const
{
}

int16_t LanMatchMeshLayerController::GetMatchmakeSessionTotalParticipants() const
{
    return 0;
}

bool LanMatchMeshLayerController::IsParticipated(unsigned int, unsigned long long) const
{
    return 0;
}

void LanMatchMeshLayerController::SetConnectionStatusDisconnected()
{
}

}}}
