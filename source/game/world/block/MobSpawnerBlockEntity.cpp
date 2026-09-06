#include "game/world/block/MobSpawnerBlockEntity.hpp"

#include "game/core/UniqueTypeWalker.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Blocks.hpp"

MobSpawnerBlockEntity::MobSpawnerBlockEntity()
    : BlockEntity(), mSpawner(new BlockEntityMobSpawner(this))
{
}

MobSpawnerBlockEntity::~MobSpawnerBlockEntity()
{
    delete mSpawner;
}

BlockEntity* MobSpawnerBlockEntity::create()
{
    return new MobSpawnerBlockEntity();
}

MobSpawnerBlockEntity::BlockEntityMobSpawner* MobSpawnerBlockEntity::getSpawner()
{
    return mSpawner;
}

void MobSpawnerBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_BlockEntity,
                     (DataWalker*)new UniqueTypeWalker(BlockEntity::getKey(0x01200201ULL)));
}

void MobSpawnerBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mSpawner->load(tag);
}

void MobSpawnerBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    mSpawner->save(tag);
}

void MobSpawnerBlockEntity::tick()
{
    mSpawner->tick();
    BlockEntity::tick();
}

boost::shared_ptr<Packet> MobSpawnerBlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<Packet>(new ClientboundBlockEntityDataPacket(
        getBlockPos(), 1, mLevel->mDimension->getType()->getId(), getUpdateTag()));
}

CompoundTag* MobSpawnerBlockEntity::getUpdateTag()
{
    CompoundTag* tag = new CompoundTag();
    save(tag);
    tag->remove(L"SpawnPotentials");
    return tag;
}

bool MobSpawnerBlockEntity::triggerEvent(int id, int param)
{
    if (mSpawner->onEventTriggered(id))
    {
        return true;
    }
    return BlockEntity::triggerEvent(id, param);
}

bool MobSpawnerBlockEntity::onlyOpCanSetNbt()
{
    return true;
}

boost::shared_ptr<BlockEntity> MobSpawnerBlockEntity::clone()
{
    boost::shared_ptr<MobSpawnerBlockEntity> result(new MobSpawnerBlockEntity());
    BlockEntity::clone(result);
    return result;
}

void MobSpawnerBlockEntity::setEntityId(const std::wstring& entityId)
{
    ResourceLocation location(entityId);
    mSpawner->setEntityId(location);
}

unsigned long long MobSpawnerBlockEntity::GetType()
{
    return 0x0120020100000000ULL;
}

MobSpawnerBlockEntity::BlockEntityMobSpawner::BlockEntityMobSpawner(MobSpawnerBlockEntity* blockEntity)
    : BaseMobSpawner(), mBlockEntity(blockEntity)
{
}

MobSpawnerBlockEntity::BlockEntityMobSpawner::~BlockEntityMobSpawner()
{
}

void MobSpawnerBlockEntity::BlockEntityMobSpawner::broadcastEvent(int event)
{
    mBlockEntity->mLevel->blockEvent(mBlockEntity->mPos, Blocks::mob_spawner, event, 0);
}

Level* MobSpawnerBlockEntity::BlockEntityMobSpawner::getLevel()
{
    return mBlockEntity->mLevel;
}

BlockPos MobSpawnerBlockEntity::BlockEntityMobSpawner::getPos()
{
    return mBlockEntity->mPos;
}

void MobSpawnerBlockEntity::BlockEntityMobSpawner::setNextSpawnData(SpawnData* data)
{
    BaseMobSpawner::setNextSpawnData(data);
    if (getLevel() != NULL)
    {
        BlockPos pos = getPos();
        const BlockState* state = getLevel()->getBlockState(pos);
        getLevel()->sendBlockUpdated(pos, state, state, 4, true);
    }
}
