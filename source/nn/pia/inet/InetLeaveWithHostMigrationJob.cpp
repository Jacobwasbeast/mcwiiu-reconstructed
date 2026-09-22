#include "nn/pia/inet/InetLeaveWithHostMigrationJob.hpp"

#include "nn/pia/session/Mesh.hpp"

namespace nn { namespace pia { namespace inet {

InetLeaveWithHostMigrationJob::InetLeaveWithHostMigrationJob() : session::LeaveWithHostMigrationJob() {}
InetLeaveWithHostMigrationJob::~InetLeaveWithHostMigrationJob() {}
void InetLeaveWithHostMigrationJob::Trace(unsigned long long) const {}
int InetLeaveWithHostMigrationJob::DecideNextHost()
{
    session::Mesh* mesh = session::g_piaMesh;
    const StationIndex local = mesh->GetLocalStationIndex();
    if (local < 32 && local == mesh->GetHostStationIndex())
    {
        for (StationIndex candidate = 0; candidate < 32; ++candidate)
            if (candidate != local && mesh->IsValidStationIndex(candidate))
                return candidate;
    }
    return -3;
}

}}}
