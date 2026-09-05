#ifndef GAME_ENTITY_MOB_THEENDGATEWAYENTITY_HPP
#define GAME_ENTITY_MOB_THEENDGATEWAYENTITY_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/BlockEntity.hpp"

class Block;
class BlockPos;
class CompoundTag;
class Direction;
class Entity;
class Level;
class LevelChunk;
class Packet;
class Vec3;

class TheEndGatewayEntity : public BlockEntity
{
public:
    TheEndGatewayEntity();
    virtual ~TheEndGatewayEntity();

    static TheEndGatewayEntity* create();
    bool isCoolingDown();
    static BlockPos findTallestBlock(Level* level, const BlockPos& origin, int radius, bool skipAir);
    void SetExitPortal(BlockPos* position);
    static BlockPos* findValidSpawnInChunk(LevelChunk* chunk);
    void createExitPortal(const BlockPos& position);
    void findExitPortal();
    BlockPos findExitPosition();
    void triggerCooldown();
    void teleportEntity(boost::shared_ptr<Entity> entity);

    virtual void load(CompoundTag* tag);
    virtual void save(CompoundTag* tag);
    virtual double getViewDistance();
    virtual void tick();
    bool isSpawning();
    float getSpawnPercent(float partialTick);
    double getCooldownPercent(float partialTick);
    virtual boost::shared_ptr<Packet> getUpdatePacket();
    virtual CompoundTag* getUpdateTag();
    virtual bool triggerEvent(int id, int param);
    virtual void setChanged();
    virtual bool shouldRenderFace(const Direction* direction);
    int getParticleAmount();
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual unsigned long long GetType();

    static LevelChunk* getChunk(Level* level, Vec3* position);
    static void setExactPosition(const BlockPos& position);

private:
    long long mAge;
    int mCooldown;
    BlockPos* mExitPortal;
    bool mExactTeleport;
    unsigned char mReserved1;
    unsigned char mReserved2;
    unsigned char mReserved3;
    void* mUnknown;
};

#endif
