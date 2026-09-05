#ifndef GAME_WORLD_BLOCK_MOBSPAWNERBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_MOBSPAWNERBLOCKENTITY_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/core/SpawnData.hpp"
#include "game/entity/mob/BaseMobSpawner.hpp"
#include "game/world/block/BlockEntity.hpp"

class CompoundTag;
class DataFixerUpper;
class Level;
class Packet;

class MobSpawnerBlockEntity : public BlockEntity
{
public:
    class BlockEntityMobSpawner : public BaseMobSpawner
    {
    public:
        explicit BlockEntityMobSpawner(MobSpawnerBlockEntity* blockEntity);
        virtual ~BlockEntityMobSpawner();
        virtual void broadcastEvent(int event);
        virtual Level* getLevel();
        virtual BlockPos getPos();
        virtual void setNextSpawnData(SpawnData* data);
    private:
        MobSpawnerBlockEntity* mBlockEntity;
    };

    MobSpawnerBlockEntity();
    virtual ~MobSpawnerBlockEntity();
    static BlockEntity* create();
    BlockEntityMobSpawner* getSpawner();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void load(CompoundTag* tag);
    virtual void save(CompoundTag* tag);
    virtual void tick();
    virtual boost::shared_ptr<Packet> getUpdatePacket();
    virtual CompoundTag* getUpdateTag();
    virtual bool triggerEvent(int id, int param);
    virtual bool onlyOpCanSetNbt();
    virtual boost::shared_ptr<BlockEntity> clone();
    void setEntityId(const std::wstring& entityId);
    virtual unsigned long long GetType();
private:
    BlockEntityMobSpawner* mSpawner;
};

#endif
