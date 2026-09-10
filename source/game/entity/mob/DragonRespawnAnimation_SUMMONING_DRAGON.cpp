#include "game/entity/mob/DragonRespawnAnimation_SUMMONING_DRAGON.hpp"

#include "game/entity/mob/EndDragonFight.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/block/BlockPos.hpp"

void DragonRespawnAnimation_SUMMONING_DRAGON::tick(ServerLevel* level,
    EndDragonFight* dragonFight,
    std::vector<boost::shared_ptr<EnderCrystal> >* crystals, int tick,
    const BlockPos& portalLocation)
{
    if (tick < 100)
    {
        if (tick < 80)
        {
            if (tick == 0)
            {
                BlockPos beamTarget(0, 128, 0);
                std::vector<boost::shared_ptr<EnderCrystal> >::iterator it = crystals->begin();
                while (it != crystals->end())
                {
                    it->get()->setBeamTarget(&beamTarget);
                    ++it;
                }
            }
            else if (tick < 5)
            {
                BlockPos eventPosition(0, 128, 0);
                level->levelEvent(3001, eventPosition, 0);
            }
        }
        else
        {
            BlockPos eventPosition(0, 128, 0);
            level->levelEvent(3001, eventPosition, 0);
        }
        return;
    }

    dragonFight->setRespawnStage(DragonRespawnAnimation::END);
    dragonFight->resetSpikeCrystals();

    std::vector<boost::shared_ptr<EnderCrystal> >::iterator it = crystals->begin();
    while (it != crystals->end())
    {
        EnderCrystal* crystal = it->get();
        crystal->setBeamTarget(NULL);

        crystal->remove();
        ++it;
    }
}
