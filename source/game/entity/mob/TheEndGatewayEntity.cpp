#include "game/entity/mob/TheEndGatewayEntity.hpp"

#include <boost/shared_ptr.hpp>
#include <vector>

#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/util/NbtUtils.hpp"
#include "game/util/Random.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/generation/EndGatewayFeature.hpp"

TheEndGatewayEntity::TheEndGatewayEntity()
    : BlockEntity(), mAge(0), mCooldown(0), mExitPortal(NULL), mExactTeleport(false),
      mReserved1(0), mReserved2(0), mReserved3(0), mUnknown(NULL)
{
}

TheEndGatewayEntity::~TheEndGatewayEntity()
{
    delete mExitPortal;
}

BlockEntity* TheEndGatewayEntity::create()
{
    return new TheEndGatewayEntity();
}

bool TheEndGatewayEntity::isCoolingDown()
{
    return mCooldown > 0;
}

BlockPos TheEndGatewayEntity::findTallestBlock(Level* level, const BlockPos& origin, int radius, bool skipAir)
{
    BlockPos* result = NULL;
    int minimum = -radius;
    int maximum = radius;
    for (int x = minimum; x < maximum; ++x)
    {
        for (int z = minimum; z < maximum; ++z)
        {
            for (int y = 255; y >= 0; --y)
            {
                BlockPos candidate = origin.offset(x, y, z);
                const BlockState* state = level->getBlockState(candidate);
                if (state != NULL && (!skipAir || !level->isEmptyBlock(candidate)))
                {
                    delete result;
                    result = new BlockPos(candidate);
                    break;
                }
            }
        }
    }
    BlockPos answer = result == NULL ? origin : *result;
    delete result;
    return answer;
}

void TheEndGatewayEntity::SetExitPortal(BlockPos* position)
{
    delete mExitPortal;
    mExitPortal = position;
}

BlockPos* TheEndGatewayEntity::findValidSpawnInChunk(LevelChunk* chunk)
{
    if (chunk == NULL)
        return NULL;
    ChunkPos chunkPosition = chunk->getPos();
    BlockPos first(chunkPosition.x * 16, 30, chunkPosition.z * 16);
    BlockPos last(chunkPosition.x * 16 + 15, chunk->getHighestSectionPosition() + 15, chunkPosition.z * 16 + 15);
    BlockPos* best = NULL;
    double bestDistance = 0.0;
    for (int x = first.x; x <= last.x; ++x)
    {
        for (int z = first.z; z <= last.z; ++z)
        {
            for (int y = last.y; y >= first.y; --y)
            {
                BlockPos candidate(x, y, z);
                const BlockState* state = chunk->getBlockState(candidate);
                if (state != NULL && state->getBlock() == Blocks::end_stone)
                {
                    if (chunk->getBlockState(candidate.above()) == NULL || !chunk->getLevel()->isEmptyBlock(candidate.above()))
                        continue;
                    if (chunk->getBlockState(candidate.above(2)) == NULL || !chunk->getLevel()->isEmptyBlock(candidate.above(2)))
                        continue;
                    double distance = candidate.distSqrToCenter(0.0, 0.0, 0.0);
                    if (best == NULL || distance < bestDistance)
                    {
                        delete best;
                        best = new BlockPos(candidate);
                        bestDistance = distance;
                    }
                }
            }
        }
    }
    return best;
}

void TheEndGatewayEntity::createExitPortal(const BlockPos& position)
{
    if (mLevel == NULL)
        return;
    Random random;
    EndGatewayFeature feature(mExactTeleport ? 1 : 0);
    feature.place(mLevel, random, position);
}

void TheEndGatewayEntity::findExitPortal()
{
    if (mLevel == NULL)
        return;
    BlockPos origin = getBlockPos();
    LevelChunk* chunk = mLevel->getChunkAt(origin);
    SetExitPortal(findValidSpawnInChunk(chunk));
    if (mExitPortal != NULL)
        createExitPortal(*mExitPortal);
    setChanged();
}

BlockPos TheEndGatewayEntity::findExitPosition()
{
    if (mLevel == NULL || mExitPortal == NULL)
        return BlockPos();
    return findTallestBlock(mLevel, *mExitPortal, 5, false).above();
}

void TheEndGatewayEntity::triggerCooldown()
{
    if (mLevel != NULL && !mLevel->mIsClientSide)
    {
        mCooldown = 40;
        mLevel->setBlockAndUpdate(getBlockPos(), getBlock()->defaultBlockState());
        setChanged();
    }
}

void TheEndGatewayEntity::teleportEntity(boost::shared_ptr<Entity> entity)
{
    if (entity.get() == NULL || mLevel == NULL || mLevel->mIsClientSide || isCoolingDown())
        return;
    if (mExitPortal == NULL)
        findExitPortal();
    if (mExitPortal == NULL)
        return;
    BlockPos target = *mExitPortal;
    if (!mExactTeleport)
        target = target.above();
    entity->stopRidingBeforeTeleport();
    entity->teleportTo(target.x + 0.5, target.y + 1.0, target.z + 0.5);
    triggerCooldown();
}

CompoundTag* TheEndGatewayEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putLong(L"Age", mAge);
    if (mExitPortal != NULL)
        tag->put(L"ExitPortal", NbtUtils::writeBlockPos(*mExitPortal));
    if (mExactTeleport)
        tag->putBoolean(L"ExactTeleport", mExactTeleport);
    return tag;
}

void TheEndGatewayEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mAge = tag->getLong(L"Age");
    if (tag->contains(L"ExitPortal", 10))
        SetExitPortal(new BlockPos(NbtUtils::readBlockPos(tag->getCompound(L"ExitPortal"))));
    mExactTeleport = tag->getBoolean(L"ExactTeleport");
}

double TheEndGatewayEntity::getViewDistance()
{
    return 65536.0;
}

bool TheEndGatewayEntity::isSpawning()
{
    return mAge < 200;
}

void TheEndGatewayEntity::tick()
{
    bool spawning = isSpawning();
    bool cooling = isCoolingDown();
    ++mAge;
    if (cooling)
        --mCooldown;
    else if (!mLevel->mIsClientSide)
    {
        std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(Entity), AABB::newTemp(getBlockPos()));
        if (entities != NULL)
        {
            for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
                teleportEntity(*it);
            delete entities;
        }
    }
    if (spawning != isSpawning() || cooling != isCoolingDown())
        setChanged();
}

float TheEndGatewayEntity::getSpawnPercent(float partialTick)
{
    return Mth::clamp((float)(((double)mAge + partialTick) / 200.0), 0.0F, 1.0F);
}

double TheEndGatewayEntity::getCooldownPercent(float partialTick)
{
    return 1.0 - Mth::clamp((float)(((double)mCooldown - partialTick) / 40.0), 0.0F, 1.0F);
}

boost::shared_ptr<Packet> TheEndGatewayEntity::getUpdatePacket()
{
    return BlockEntity::getUpdatePacket();
}

CompoundTag* TheEndGatewayEntity::getUpdateTag()
{
    CompoundTag* tag = new CompoundTag();
    save(tag);
    return tag;
}

bool TheEndGatewayEntity::triggerEvent(int id, int param)
{
    if (param == 1)
    {
        mCooldown = 40;
        return true;
    }
    return BlockEntity::triggerEvent(id, param);
}

void TheEndGatewayEntity::setChanged()
{
    BlockEntity::setChanged();
    if (mLevel != NULL && !mLevel->mIsClientSide)
        mLevel->updateObservingBlocksAt(mPos, getBlock());
}

bool TheEndGatewayEntity::shouldRenderFace(const Direction* direction)
{
    return getBlock()->shouldRenderFace(getBlock()->defaultBlockState(), mLevel, mPos, direction);
}

int TheEndGatewayEntity::getParticleAmount()
{
    int amount = 0;
    for (std::vector<const Direction*>::iterator it = Direction::VALUES.begin();
         it != Direction::VALUES.end(); ++it)
    {
        amount += shouldRenderFace(*it) ? 1 : 0;
    }
    return amount;
}

boost::shared_ptr<BlockEntity> TheEndGatewayEntity::clone()
{
    boost::shared_ptr<TheEndGatewayEntity> copy(new TheEndGatewayEntity());
    copy->mAge = mAge;
    copy->mCooldown = mCooldown;
    copy->mExactTeleport = mExactTeleport;
    if (mExitPortal != NULL)
        copy->mExitPortal = new BlockPos(*mExitPortal);
    copy->BlockEntity::operator=(*this);
    return copy;
}

unsigned long long TheEndGatewayEntity::GetType()
{
    return 0x1300201ULL;
}
