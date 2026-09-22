#include "nn/pia/lan/LanLeaveWithHostMigrationJob.hpp"
#include "nn/pia/session/Mesh.hpp"

namespace nn { namespace pia { namespace lan {

LanLeaveWithHostMigrationJob::LanLeaveWithHostMigrationJob() : session::LeaveWithHostMigrationJob() {}
LanLeaveWithHostMigrationJob::~LanLeaveWithHostMigrationJob() {}
void LanLeaveWithHostMigrationJob::Trace(unsigned long long) const {}
int LanLeaveWithHostMigrationJob::DecideNextHost()
{
    const StationIndex local = session::g_piaMesh->GetLocalStationIndex();
    if (local < 0x20 && local == session::g_piaMesh->GetHostStationIndex())
    {
        for (StationIndex candidate = 0; candidate < 0x20; ++candidate)
            if (candidate != local && session::g_piaMesh->IsValidStationIndex(candidate))
                return candidate;
    }
    return 0xfd;
}

}}}
