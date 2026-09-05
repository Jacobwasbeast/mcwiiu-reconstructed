#ifndef GAME_ENTITY_MOB_DRAGONRESPAWNANIMATION_SUMMONING_DRAGON_HPP
#define GAME_ENTITY_MOB_DRAGONRESPAWNANIMATION_SUMMONING_DRAGON_HPP

#include <boost/shared_ptr.hpp>
#include <vector>

class BlockPos;
class EndDragonFight;
class EnderCrystal;
class ServerLevel;

class DragonRespawnAnimation_SUMMONING_DRAGON
{
public:
    virtual void tick(ServerLevel* level, EndDragonFight* dragonFight,
        std::vector<boost::shared_ptr<EnderCrystal> >* crystals, int tick,
        const BlockPos& portalLocation);
};

#endif
