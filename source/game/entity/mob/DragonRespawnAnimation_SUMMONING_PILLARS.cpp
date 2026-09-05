#include "game/entity/mob/DragonRespawnAnimation_SUMMONING_PILLARS.hpp"

#include <typeinfo>

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EndDragonFight.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Iterator.hpp"
#include "game/world/block/MutableBlockPos.hpp"

class DragonRespawnAnimation;
extern DragonRespawnAnimation* g_summoningDragon;

void DragonRespawnAnimation_SUMMONING_PILLARS::tick(ServerLevel* level,
    EndDragonFight* dragonFight,
    std::vector<boost::shared_ptr<EnderCrystal> >* crystals, int tick,
    const BlockPos& portalLocation)
{
    if (tick % 40 == 0 || tick % 40 == 39)
    {
        std::vector<SpikeFeature::EndSpike> spikes =
            TheEndBiomeDecorator::getSpikesForLevel(level);

        if (tick / 40 < static_cast<int>(spikes.size()))
        {
            SpikeFeature::EndSpike& spike = spikes[tick / 40];

            if (tick % 40 == 0)
            {
                BlockPos beamTarget(spike.getCenterX(), spike.getHeight() + 1,
                    spike.getCenterZ());

                std::vector<boost::shared_ptr<EnderCrystal> >::iterator it =
                    crystals->begin();
                while (it != crystals->end())
                {
                    it->get()->setBeamTarget(&beamTarget);
                    ++it;
                }
            }
            else
            {
                BlockPos from(spike.getCenterX() - 10,
                    spike.getHeight() - 10, spike.getCenterZ() - 10);
                BlockPos to(spike.getCenterX() + 10,
                    spike.getHeight() + 10, spike.getCenterZ() + 10);
                AABB* box = AABB::newTemp(from, to);

                std::vector<boost::shared_ptr<Entity> >* entities =
                    level->getEntitiesOfClass(typeid(Entity), box);
                std::vector<boost::shared_ptr<Entity> >::iterator entity =
                    entities->begin();
                while (entity != entities->end())
                {
                    entity->get()->setInvulnerable(false);
                    ++entity;
                }
                delete entities;

                Iterator<MutableBlockPos*>* positions =
                    BlockPos::mutablesBetween(from, to);
                while (positions->hasNext())
                {
                    level->removeBlock(*positions->next());
                }
                delete positions;

                SpikeFeature feature;
                SpikeFeature::EndSpike featureSpike(spike);
                feature.setSpike(&featureSpike);
                feature.setCrystalInvulnerable(true);

                BlockPos crystalBeamTarget(0, 128, 0);
                feature.setCrystalBeamTarget(crystalBeamTarget);

                Random random;
                BlockPos placement(spike.getCenterX(), 45,
                    spike.getCenterZ());
                feature.place(level, random, placement);
            }
        }
        else if (tick % 40 == 0)
        {
            dragonFight->setRespawnStage(g_summoningDragon);
        }
    }
}
