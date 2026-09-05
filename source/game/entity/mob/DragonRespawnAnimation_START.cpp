#include "game/entity/mob/DragonRespawnAnimation_START.hpp"

#include "game/entity/mob/EndDragonFight.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/block/BlockPos.hpp"

class DragonRespawnAnimation;
extern DragonRespawnAnimation* g_preparingToSummonPillars;

void DragonRespawnAnimation_START::tick(ServerLevel* level,
    EndDragonFight* dragonFight,
    std::vector<boost::shared_ptr<EnderCrystal> >* crystals, int tick,
    const BlockPos& portalLocation)
{
    BlockPos beamTarget(0, 128, 0);

    std::vector<boost::shared_ptr<EnderCrystal> >::iterator it = crystals->begin();
    while (it != crystals->end())
    {
        it->get()->setBeamTarget(&beamTarget);
        ++it;
    }

    dragonFight->setRespawnStage(g_preparingToSummonPillars);
}
