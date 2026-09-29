#include "game/entity/mob/DragonRespawnAnimation_SUMMONING_PILLARS.hpp"

#include <typeinfo>

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EndDragonFight.hpp"
#include "game/core/EnderCrystal.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/biome/TheEndBiomeDecorator.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Iterator.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/generation/SpikeFeature.hpp"

void DragonRespawnAnimation_SUMMONING_PILLARS::tick(ServerLevel* level,
    EndDragonFight* dragonFight,
    std::vector<boost::shared_ptr<EnderCrystal> >* crystals, int tick,
    const BlockPos& portalLocation)
{
    if (tick % 40 == 0 || tick % 40 == 39)
    {
        arrayWithLength<SpikeFeature::EndSpike> spikes =
            TheEndBiomeDecorator::getSpikesForLevel(level);

        if (tick / 40 < static_cast<int>(spikes.length))
        {
            SpikeFeature::EndSpike& spike = spikes.data[tick / 40];

            if (tick % 40 == 0)
            {
                std::vector<boost::shared_ptr<EnderCrystal> >::iterator it =
                    crystals->begin();
                while (it != crystals->end())
                {
                    it->get()->setBeamTarget(new BlockPos(static_cast<int>(spike.getCenterX()),
                        static_cast<int>(spike.getHeight()) + 1,
                        static_cast<int>(spike.getCenterZ())));
                    ++it;
                }
            }
            else
            {
                BlockPos from(static_cast<int>(spike.getCenterX()) - 10,
                    static_cast<int>(spike.getHeight()) - 10,
                    static_cast<int>(spike.getCenterZ()) - 10);
                BlockPos to(static_cast<int>(spike.getCenterX()) + 10,
                    static_cast<int>(spike.getHeight()) + 10,
                    static_cast<int>(spike.getCenterZ()) + 10);
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

                level->explode(boost::shared_ptr<Entity>(),
                    static_cast<double>(static_cast<float>(spike.getCenterX()) + 0.5f),
                    static_cast<double>(spike.getHeight()),
                    static_cast<double>(static_cast<float>(spike.getCenterZ()) + 0.5f),
                    5.0f, true, true, false, false, false, -1.0f);

                SpikeFeature feature;
                SpikeFeature::EndSpike featureSpike(spike);
                feature.setSpike(&featureSpike);
                feature.setCrystalInvulnerable(true);

                BlockPos crystalBeamTarget(0, 128, 0);
                feature.setCrystalBeamTarget(crystalBeamTarget);

                Random random;
                BlockPos placement(static_cast<int>(spike.getCenterX()), 45,
                    static_cast<int>(spike.getCenterZ()));
                feature.place(level, random, placement);
            }
        }
        else if (tick % 40 == 0)
        {
            dragonFight->setRespawnStage(DragonRespawnAnimation::SUMMONING_DRAGON);
        }
        delete[] spikes.data;
    }
}
