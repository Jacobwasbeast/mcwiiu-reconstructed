#include "game/entity/mob/DragonRespawnAnimation_PREPARING_TO_SUMMON_PILLARS.hpp"

#include "game/entity/mob/EndDragonFight.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/block/BlockPos.hpp"

void DragonRespawnAnimation_PREPARING_TO_SUMMON_PILLARS::tick(
    ServerLevel* level, EndDragonFight* dragonFight,
    std::vector<boost::shared_ptr<EnderCrystal> >* crystals, int tick,
    const BlockPos& portalLocation)
{
    if (tick < 100)
    {
        if (tick == 0 || (tick > 49 && tick < 53) || tick > 94)
        {
            BlockPos beamTarget(0, 128, 0);
            level->levelEvent(3001, beamTarget, 0);
        }
    }
    else
    {
        dragonFight->setRespawnStage(DragonRespawnAnimation::SUMMONING_PILLARS);
    }
}
