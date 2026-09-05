#ifndef GAME_CORE_CAMERA_HPP
#define GAME_CORE_CAMERA_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/BlockPos.hpp"

class Block;
class Entity;
class Level;
class LivingEntity;
class Player;
class Vec3;

class Camera
{
public:
    static void prepare(boost::shared_ptr<Player> player, bool secondPass);
    static Vec3* getCameraPos(boost::shared_ptr<Entity> entity, double partialTick);
    static BlockPos getCameraBlockPos(boost::shared_ptr<LivingEntity> entity, double partialTick);
    static Block* getBlockAt(Level* level, boost::shared_ptr<Entity> entity, float partialTick);
    static void UpdatePlayerOffsets(boost::shared_ptr<Player> player);
};

#endif
