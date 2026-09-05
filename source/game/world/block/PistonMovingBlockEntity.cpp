#include "game/world/block/PistonMovingBlockEntity.hpp"
#include "game/core/Direction.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
static const Direction* gPistonNoClipDirection = NULL;
PistonMovingBlockEntity::PistonMovingBlockEntity(const BlockState* state, const Direction* direction, bool extending, bool source, CompoundTag* lastTick)
    : BlockEntity(), mMovedState(state), mDirection(direction), mExtending(extending), mSourcePiston(source), mLastTick(lastTick), mProgress(0.0f), mLastProgress(0.0f)
{
}
PistonMovingBlockEntity::PistonMovingBlockEntity() : BlockEntity(), mMovedState(NULL), mDirection(NULL), mExtending(false), mSourcePiston(false), mLastTick(NULL), mProgress(0.0f), mLastProgress(0.0f) {}
PistonMovingBlockEntity::~PistonMovingBlockEntity() { delete mLastTick; }
BlockEntity* PistonMovingBlockEntity::create() { return new PistonMovingBlockEntity(); }
void PistonMovingBlockEntity::stop()
{
    if (mLevel != NULL) { mLevel->removeBlockEntity(mPos); setRemoved(); if (mMovedState != NULL) { mLevel->setBlockAndUpdate(mPos, mMovedState); mLevel->neighborChanged(mPos, mMovedState->getBlock(), mPos); } }
    delete mLastTick; mLastTick = NULL;
}
void PistonMovingBlockEntity::finalTick() { if (mProgress < 1.0f) { mProgress = 1.0f; mLastProgress = 1.0f; stop(); } }
const Direction* PistonMovingBlockEntity::getDirection() { return mDirection; }
bool PistonMovingBlockEntity::isExtending() { return mExtending; }
void PistonMovingBlockEntity::NOCLIP_set(const Direction* direction) { gPistonNoClipDirection = direction; }
const Direction* PistonMovingBlockEntity::NOCLIP_get() { return gPistonNoClipDirection; }
const BlockState* PistonMovingBlockEntity::getBlockState() { return mMovedState; }
CompoundTag* PistonMovingBlockEntity::getUpdateTag() { CompoundTag* tag = new CompoundTag(); save(tag); return tag; }
int PistonMovingBlockEntity::getData() { return 0; }
bool PistonMovingBlockEntity::isSourcePiston() { return mSourcePiston; }
double PistonMovingBlockEntity::getProgress(float partialTick) { if (partialTick > 1.0f) partialTick = 1.0f; return mLastProgress + (mProgress - mLastProgress) * partialTick; }
double PistonMovingBlockEntity::getExtendedProgress(float partialTick) { return mExtending ? partialTick - 1.0f : 1.0f - partialTick; }
double PistonMovingBlockEntity::getXOff(float partialTick) { return mDirection->getNormal().x * getExtendedProgress(partialTick); }
double PistonMovingBlockEntity::getYOff(float partialTick) { return mDirection->getNormal().y * getExtendedProgress(partialTick); }
double PistonMovingBlockEntity::getZOff(float partialTick) { return mDirection->getNormal().z * getExtendedProgress(partialTick); }
const BlockState* PistonMovingBlockEntity::getCollisionRelatedBlockState() { return mMovedState; }
AABB* PistonMovingBlockEntity::getAABB(LevelSource* source, const BlockPos& pos, float partialTick)
{
    const AABB* shape = mMovedState->getBlock()->getShape(mMovedState, source, pos);
    AABB* result = new AABB(shape->mMinX, shape->mMinY, shape->mMinZ, shape->mMaxX, shape->mMaxY, shape->mMaxZ);
    AABB* moved = result->move(getXOff(partialTick), getYOff(partialTick), getZOff(partialTick)); result->set(moved); return result;
}
AABB* PistonMovingBlockEntity::getAABB(LevelSource* source, const BlockPos& pos) { return getAABB(source, pos, mProgress); }
void PistonMovingBlockEntity::moveByPositionAndProgress(AABB* box) { AABB* moved = box->move(getXOff(mProgress), getYOff(mProgress), getZOff(mProgress)); box->set(moved); }
void PistonMovingBlockEntity::getMinMaxPiecesAABB(std::vector<AABB*>* pieces)
{
    if (pieces->empty()) return; AABB* result = new AABB(*pieces->at(0)); for (std::size_t i = 1; i < pieces->size(); ++i) result->minmax(pieces->at(i)); pieces->clear(); pieces->push_back(result);
}
AABB* PistonMovingBlockEntity::getMovementArea(AABB* box, const Direction* direction, double distance) { return box->expand(direction->getNormal().x * distance, direction->getNormal().y * distance, direction->getNormal().z * distance); }
double PistonMovingBlockEntity::getDeltaX(AABB* box, const Direction* direction, AABB* other) { return direction->getAxisDirection() == Direction::AxisDirection::POSITIVE ? box->mMaxX - other->mMinX : other->mMaxX - box->mMinX; }
double PistonMovingBlockEntity::getDeltaY(AABB* box, const Direction* direction, AABB* other) { return direction->getAxisDirection() == Direction::AxisDirection::POSITIVE ? box->mMaxY - other->mMinY : other->mMaxY - box->mMinY; }
double PistonMovingBlockEntity::getDeltaZ(AABB* box, const Direction* direction, AABB* other) { return direction->getAxisDirection() == Direction::AxisDirection::POSITIVE ? box->mMaxZ - other->mMinZ : other->mMaxZ - box->mMinZ; }
double PistonMovingBlockEntity::getMovement(AABB* box, const Direction* direction, AABB* other)
{
    if (direction->getAxis() == Direction::Axis::X)
    {
        return getDeltaX(box, direction, other);
    }
    if (direction->getAxis() == Direction::Axis::Y)
    {
        return getDeltaY(box, direction, other);
    }
    return getDeltaZ(box, direction, other);
}
void PistonMovingBlockEntity::fixEntityWithinPistonBase(boost::shared_ptr<Entity>, const Direction*, double) {}
void PistonMovingBlockEntity::moveCollidedEntities(float) {}
void PistonMovingBlockEntity::tick() { mLastProgress = mProgress; if (mProgress >= 1.0f) { stop(); return; } float next = mProgress + 0.5f; moveCollidedEntities(next); mProgress += next; if (mProgress >= 1.0f) mProgress = 1.0f; }
void PistonMovingBlockEntity::addDataWalkers(DataFixerUpper*) {}
void PistonMovingBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    Block* block = Block::byId(tag->getInt(L"blockId"));
    mMovedState = block->getBlockState(tag->getInt(L"blockData"));
    mDirection = Direction::from3DDataValue(tag->getInt(L"facing"));
    mProgress = tag->getFloat(L"progress");
    mLastProgress = mProgress;
    mExtending = tag->getBoolean(L"extending");
    mSourcePiston = tag->getBoolean(L"source");
}

void PistonMovingBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putInt(L"blockId", Block::getId(mMovedState->getBlock()));
    tag->putInt(L"blockData", mMovedState->getBlock()->convertBlockStateToLegacyData(mMovedState));
    tag->putInt(L"facing", mDirection->get3DDataValue());
    tag->putFloat(L"progress", mProgress);
    tag->putBoolean(L"extending", mExtending);
    tag->putBoolean(L"source", mSourcePiston);
}

void PistonMovingBlockEntity::addCollissionAABBs(Level* level, const BlockPos& pos, const AABB*, std::vector<AABB*>* boxes, boost::shared_ptr<Entity>)
{
    const AABB* shape = mMovedState->getBlock()->getShape(mMovedState, level, pos);
    if (shape != NULL)
    {
        boxes->push_back(new AABB(*shape));
    }
}

boost::shared_ptr<BlockEntity> PistonMovingBlockEntity::clone()
{
    boost::shared_ptr<BlockEntity> result(new PistonMovingBlockEntity(mMovedState, mDirection, mExtending, mSourcePiston, mLastTick == NULL ? NULL : mLastTick->copy()));
    BlockEntity::clone(result);
    PistonMovingBlockEntity* copy = static_cast<PistonMovingBlockEntity*>(result.get());
    copy->mProgress = mProgress;
    copy->mLastProgress = mLastProgress;
    return result;
}
unsigned long long PistonMovingBlockEntity::GetType() { return 0x0120020500000000ULL; }
