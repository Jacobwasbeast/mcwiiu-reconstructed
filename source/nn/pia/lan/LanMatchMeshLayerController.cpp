#include "nn/pia/lan/LanMatchMeshLayerController.hpp"

namespace nn { namespace pia { namespace lan {

LanMatchMeshLayerController::LanMatchMeshLayerController()
{
}

LanMatchMeshLayerController::~LanMatchMeshLayerController()
{
}

uint64_t LanMatchMeshLayerController::Startup(uint32_t a1, int32_t a2)
{
    return 0;
}

void LanMatchMeshLayerController::MeshCleanup()
{
}

void LanMatchMeshLayerController::Cleanup()
{
}

void LanMatchMeshLayerController::MeshStartup(void* a1, bool a2)
{
}

uint64_t LanMatchMeshLayerController::CheckConnectedStatus()
{
    return 0;
}

void LanMatchMeshLayerController::Trace(uint64_t a1)
{
}

void LanMatchMeshLayerController::GetMatchmakeSessionTotalStations()
{
}

int16_t LanMatchMeshLayerController::GetMatchmakeSessionTotalParticipants()
{
    return 0;
}

uint32_t LanMatchMeshLayerController::IsParticipated(uint32_t a1, int32_t a2, int32_t a3)
{
    return 0;
}

void LanMatchMeshLayerController::SetConnectionStatusDisconnected()
{
}

}}}
