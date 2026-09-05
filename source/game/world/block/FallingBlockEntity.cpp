#include "game/world/block/FallingBlockEntity.hpp"

#include <boost/shared_ptr.hpp>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/Tag.hpp"
#include "game/world/block/EntityDataSerializer.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"

const EntityDataAccessor<BlockPos>* FallingBlockEntity::DATA_START_POS = NULL;
static const unsigned long long ENTITY_TYPE_FALLING_BLOCK = 0x0000000008000009ULL;

class EntityDataSerializers
{
public:
    static EntityDataSerializer<BlockPos>* BLOCK_POS;
};

FallingBlockEntity::FallingBlockEntity(Level* level, double x, double y, double z, const BlockState* blockState)
    : Entity(level, true), mBlockState(blockState), mTime(0), mDropItem(true), mHurtsEntities(false),
      mFallHurtMax(40), mFallHurtAmount(2.0f), mBlockEntityData(NULL)
{
    _init();
    setSize(0.98f, 0.98f);
    setPos(x, y + (1.0f - mBbHeight) * 0.5f, z);
    mXo = mX;
    mYo = mY;
    mZo = mZ;
    mXOld = mX;
    mYOld = mY;
    mZOld = mZ;
    setStartPos(BlockPos(this));
}

FallingBlockEntity::FallingBlockEntity(Level* level)
    : Entity(level, true), mBlockState(NULL), mTime(0), mDropItem(true), mHurtsEntities(false),
      mFallHurtMax(40), mFallHurtAmount(2.0f), mBlockEntityData(NULL)
{
    _init();
}

FallingBlockEntity::~FallingBlockEntity()
{
    if (mBlockEntityData != NULL)
    {
        delete mBlockEntityData;
        mBlockEntityData = NULL;
    }
}

void FallingBlockEntity::staticCtor()
{
    DATA_START_POS = SynchedEntityData::defineId(ENTITY_TYPE_FALLING_BLOCK, ENTITY_TYPE_ENTITY, EntityDataSerializers::BLOCK_POS);
}

void FallingBlockEntity::_init()
{
    fjDerivedCtorCalls();
    mDropItem = true;
    mHurtsEntities = false;
    mBlockEntityData = NULL;
    mTime = 0;
    mFallHurtMax = 40;
    mFallHurtAmount = 2.0f;
}

void FallingBlockEntity::setStartPos(const BlockPos& pos)
{
    if (DATA_START_POS != NULL)
    {
        getEntityData()->set(DATA_START_POS, pos, true);
    }
}

void FallingBlockEntity::defineSynchedData()
{
    getEntityData()->createDateItem(DATA_START_POS, BlockPos::ZERO);
}

bool FallingBlockEntity::isAttackable()
{
    return false;
}

bool FallingBlockEntity::makeStepSound()
{
    return false;
}

bool FallingBlockEntity::isPickable()
{
    return !mRemoved;
}

void FallingBlockEntity::tick()
{
    if (mRemoved)
    {
        return;
    }

    mXo = mX;
    mYo = mY;
    mZo = mZ;
    ++mTime;
    mYd -= 0.04;
    move(eMoverType_Self, mXd, mYd, mZd, true);
    if (mOnGround)
    {
        mYd *= -0.5;
        mXd *= 0.7;
        mZd *= 0.7;
    }
}

void FallingBlockEntity::causeFallDamage(float fallDistance, float damageMultiplier)
{
    if (!mHurtsEntities || fallDistance <= 1.0f)
    {
        return;
    }
}

void FallingBlockEntity::addDataWalkers(DataFixerUpper*)
{
}

void FallingBlockEntity::addAdditonalSaveData(CompoundTag* tag)
{
    Block* block = mBlockState == NULL ? NULL : mBlockState->getBlock();
    tag->putString(std::wstring(L"Block"), block == NULL ? std::wstring(L"") : block->getName());
    tag->putByte(std::wstring(L"Data"), block == NULL ? 0 : (unsigned char)block->convertBlockStateToLegacyData(mBlockState));
    tag->putInt(std::wstring(L"Time"), mTime);
    tag->putBoolean(std::wstring(L"DropItem"), mDropItem);
    tag->putBoolean(std::wstring(L"HurtEntities"), mHurtsEntities);
    tag->putFloat(std::wstring(L"FallHurtAmount"), mFallHurtAmount);
    tag->putInt(std::wstring(L"FallHurtMax"), mFallHurtMax);
    if (mBlockEntityData != NULL)
    {
        tag->put(std::wstring(L"TileEntityData"), mBlockEntityData);
    }
}

void FallingBlockEntity::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag->contains(std::wstring(L"Block"), 8))
    {
        Block* block = Block::byString(tag->getString(std::wstring(L"Block")));
        mBlockState = block == NULL ? NULL : block->getBlockState((int)tag->getByte(std::wstring(L"Data")));
    }
    else if (tag->contains(std::wstring(L"TileID"), 99))
    {
        Block* block = Block::byId(tag->getInt(std::wstring(L"TileID")));
        mBlockState = block == NULL ? NULL : block->getBlockState((int)tag->getByte(std::wstring(L"Tile")));
    }

    mTime = tag->getInt(std::wstring(L"Time"));
    if (tag->contains(std::wstring(L"HurtEntities"), 99))
    {
        mHurtsEntities = tag->getBoolean(std::wstring(L"HurtEntities"));
        mFallHurtAmount = tag->getFloat(std::wstring(L"FallHurtAmount"));
        mFallHurtMax = tag->getInt(std::wstring(L"FallHurtMax"));
    }
    if (tag->contains(std::wstring(L"DropItem"), 99))
    {
        mDropItem = tag->getBoolean(std::wstring(L"DropItem"));
    }
    if (tag->contains(std::wstring(L"TileEntityData"), 10))
    {
        mBlockEntityData = tag->getCompound(std::wstring(L"TileEntityData"));
    }
    if (mBlockState == NULL)
    {
        mBlockState = Block::byId(0)->defaultBlockState();
    }
}

Level* FallingBlockEntity::getLevel()
{
    return mLevel;
}

void FallingBlockEntity::setHurtsEntities(bool hurtsEntities)
{
    mHurtsEntities = hurtsEntities;
}

const BlockState* FallingBlockEntity::getBlock()
{
    return mBlockState;
}

bool FallingBlockEntity::displayFireAnimation()
{
    return false;
}

bool FallingBlockEntity::onlyOpCanSetNbt()
{
    return true;
}

void FallingBlockEntity::create(Level* level)
{
    new FallingBlockEntity(level);
}

unsigned long long FallingBlockEntity::GetType()
{
    return 0x0800000900000000ULL;
}
