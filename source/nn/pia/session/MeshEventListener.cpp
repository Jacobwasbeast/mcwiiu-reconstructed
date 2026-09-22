#include "nn/pia/session/MeshEventListener.hpp"

#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/transport/StationManager.hpp"

namespace nn { namespace pia { namespace session {

MeshEventListener::MeshEventListener()
{
}

MeshEventListener::~MeshEventListener()
{
}

unsigned long long MeshEventListener::RemoveStationId(unsigned long long stationId)
{
    if (stationId == 0 || transport::g_piaStationManager == 0)
        return 0;

    transport::Station* station =
        transport::g_piaStationManager->GetStationByStationIdWithoutConvert(stationId);
    if (station == 0)
        return 0;

    station->CleanupJobs();
    station->Cleanup();
    transport::g_piaStationManager->DestroyStation(station);
    return stationId;
}

unsigned long long MeshEventListener::RemoveStationIndex(StationIndex stationIndex)
{
    if (g_piaMesh != 0)
        g_piaMesh->UnfixDisconnectedId(stationIndex);
    if (transport::g_piaStationManager == 0)
        return 0;

    transport::Station* station = transport::g_piaStationManager->GetStation(stationIndex);
    if (station == 0)
        return 0;
    station->CleanupJobs();
    station->Cleanup();
    transport::g_piaStationManager->DestroyStation(station);
    return static_cast<unsigned long long>(stationIndex);
}

void MeshEventListener::NoticeEvent(const MeshEvent& event)
{
    if (event.type == Mesh::cEventTypeLeave)
        RemoveStationId(event.stationId);
    else
        RemoveStationIndex(event.stationIndex);
}

}}}
