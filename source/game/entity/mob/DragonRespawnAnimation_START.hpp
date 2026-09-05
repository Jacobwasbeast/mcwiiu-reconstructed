#ifndef GAME_ENTITY_MOB_DRAGONRESPAWNANIMATION_START_HPP
#define GAME_ENTITY_MOB_DRAGONRESPAWNANIMATION_START_HPP

#include <boost/shared_ptr.hpp>
#include <vector>

class BlockPos;
class EndDragonFight;
class EnderCrystal;
class ServerLevel;

class DragonRespawnAnimation_START
{
public:
    virtual void tick(ServerLevel* level, EndDragonFight* dragonFight,
        std::vector<boost::shared_ptr<EnderCrystal> >* crystals, int tick,
        const BlockPos& portalLocation);
};

#endif
