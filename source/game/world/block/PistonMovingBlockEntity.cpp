#include "game/world/block/PistonMovingBlockEntity.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <math.h>
#include "game/core/Direction.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/MoverType.hpp"
#include <boost/assert.hpp>
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/PistonBaseBlock.hpp"
#include "game/world/block/PistonHeadBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
static const Direction* gPistonNoClipDirection = NULL;
PistonMovingBlockEntity::PistonMovingBlockEntity(const BlockState* state, const Direction* direction, bool extending, bool source, CompoundTag* blockEntityTag)
    : BlockEntity(), mMovedState(state), mDirection(direction), mExtending(extending), mSourcePiston(source), mBlockEntityTag(blockEntityTag), mProgress(0.0f), mLastProgress(0.0f)
{
}
PistonMovingBlockEntity::PistonMovingBlockEntity()
    : BlockEntity(), mMovedState(NULL), mDirection(NULL), mExtending(false),
      mSourcePiston(false), mBlockEntityTag(NULL), mProgress(0.0f), mLastProgress(0.0f)
{
}

PistonMovingBlockEntity::~PistonMovingBlockEntity()
{
    delete mBlockEntityTag;
}

BlockEntity* PistonMovingBlockEntity::create()
{
    return new PistonMovingBlockEntity();
}
void PistonMovingBlockEntity::stop()
{
    mLevel->removeBlockEntity(mPos);
    setRemoved();
    if (mLevel->mIsClientSide && CGameNetworkManager::sInstance.IsHost())
        return;
    if (mLevel->getBlockState(mPos)->getBlock() != Blocks::piston_extension)
        return;

    mLevel->setBlock(mPos, mMovedState, Block::UPDATE_ALL | Block::UPDATE_NO_OBSERVERS, false);
    if (mBlockEntityTag != NULL)
    {
        boost::shared_ptr<BlockEntity> blockEntity = mLevel->getBlockEntity(mPos);
        if (blockEntity)
        {
            mBlockEntityTag->putInt(L"x", mPos.x);
            mBlockEntityTag->putInt(L"y", mPos.y);
            mBlockEntityTag->putInt(L"z", mPos.z);
            blockEntity->load(mBlockEntityTag);
            Block* block = mLevel->getBlock(mPos);
            if (Block::getId(block) == BlockId::CHEST || block == Blocks::trapped_chest)
                static_cast<ChestBlock*>(block)->recalcLockDir(mLevel, mPos, mLevel->getBlockState(mPos));
        }
        delete mBlockEntityTag;
        mBlockEntityTag = NULL;
    }
    mLevel->neighborChanged(mPos, mMovedState->getBlock(), mPos);
}
void PistonMovingBlockEntity::finalTick()
{
    if (mLastProgress < 1.0f && mLevel != NULL)
    {
        mLastProgress = 1.0f;
        mProgress = 1.0f;
        stop();
    }
}

const Direction* PistonMovingBlockEntity::getDirection()
{
    return mDirection;
}

bool PistonMovingBlockEntity::isExtending()
{
    return mExtending;
}

void PistonMovingBlockEntity::NOCLIP_set(const Direction* direction)
{
    gPistonNoClipDirection = direction;
}

const Direction* PistonMovingBlockEntity::NOCLIP_get()
{
    return gPistonNoClipDirection;
}

const BlockState* PistonMovingBlockEntity::getBlockState()
{
    return mMovedState;
}
CompoundTag* PistonMovingBlockEntity::getUpdateTag()
{
    CompoundTag* tag = new CompoundTag();
    save(tag);
    return tag;
}
int PistonMovingBlockEntity::getData()
{
    return 0;
}

bool PistonMovingBlockEntity::isSourcePiston()
{
    return mSourcePiston;
}
double PistonMovingBlockEntity::getProgress(float partialTick)
{
    if (partialTick > 1.0f)
    {
        partialTick = 1.0f;
    }
    return mLastProgress + (mProgress - mLastProgress) * partialTick;
}
double PistonMovingBlockEntity::getExtendedProgress(float partialTick)
{
    return mExtending ? partialTick - 1.0f : 1.0f - partialTick;
}
double PistonMovingBlockEntity::getXOff(float partialTick)
{
    return mDirection->getNormal().x * getExtendedProgress(partialTick);
}
double PistonMovingBlockEntity::getYOff(float partialTick)
{
    return mDirection->getNormal().y * getExtendedProgress(partialTick);
}
double PistonMovingBlockEntity::getZOff(float partialTick)
{
    return mDirection->getNormal().z * getExtendedProgress(partialTick);
}
const BlockState* PistonMovingBlockEntity::getCollisionRelatedBlockState()
{
    if (!isExtending() && isSourcePiston())
    {
        const BlockState* headState = Blocks::piston_head->defaultBlockState();
        TypedBoxed<PistonHeadBlock::PistonType*> pistonType(
            mMovedState->getBlock() == Blocks::sticky_piston ? pistonTypeSticky : pistonTypeNormal);
        headState = headState->setBoxedValue(
            static_cast<const Property*>(PistonHeadBlock::TYPE), &pistonType);
        TypedBoxed<const Direction*> facing(mMovedState->getValue(PistonBaseBlock::FACING));
        headState = headState->setBoxedValue(
            static_cast<const Property*>(PistonHeadBlock::FACING), &facing);
        return headState;
    }
    return mMovedState;
}
AABB* PistonMovingBlockEntity::getAABB(LevelSource* source, const BlockPos& pos, float partialTick)
{
    const AABB* shape = getCollisionRelatedBlockState()->getShape(source, pos);
    AABB* result = new AABB(*shape);
    return result->move(getXOff(partialTick), getYOff(partialTick), getZOff(partialTick));
}
AABB* PistonMovingBlockEntity::getAABB(LevelSource* source, const BlockPos& pos)
{
    AABB* current = getAABB(source, pos, mProgress);
    AABB* previous = getAABB(source, pos, mLastProgress);
    current->minmax(previous);
    return current;
}
AABB* PistonMovingBlockEntity::moveByPositionAndProgress(AABB* box)
{
    double progress = getExtendedProgress(mProgress);
    return box->move(mPos.x + progress * mDirection->getStepX(), mPos.y + progress * mDirection->getStepY(),
                     mPos.z + progress * mDirection->getStepZ());
}
AABB* PistonMovingBlockEntity::getMinMaxPiecesAABB(std::vector<AABB*>* pieces)
{
    double minX = 0.0;
    double minY = 0.0;
    double minZ = 0.0;
    double maxX = 1.0;
    double maxY = 1.0;
    double maxZ = 1.0;
    for (std::vector<AABB*>::iterator it = pieces->begin(); it != pieces->end(); ++it)
    {
        AABB* piece = *it;
        minX = std::min(piece->mMinX, minX);
        minY = std::min(piece->mMinY, minY);
        minZ = std::min(piece->mMinZ, minZ);
        maxX = std::max(piece->mMaxX, maxX);
        maxY = std::max(piece->mMaxY, maxY);
        maxZ = std::max(piece->mMaxZ, maxZ);
    }
    return AABB::newTemp(minX, minY, minZ, maxX, maxY, maxZ);
}
AABB* PistonMovingBlockEntity::getMovementArea(AABB* box, const Direction* direction, double distance)
{
    double signedDistance = distance * static_cast<double>(direction->getAxisDirection()->getStep());
    double lo = signedDistance < 0.0 ? signedDistance : 0.0;
    double hi = signedDistance < 0.0 ? 0.0 : signedDistance;
    switch (direction->get3DDataValue())
    {
    case 0:
        return AABB::newTemp(box->mMinX, box->mMinY + lo, box->mMinZ, box->mMaxX, box->mMinY + hi, box->mMaxZ);
    case 1:
        return AABB::newTemp(box->mMinX, box->mMaxY + lo, box->mMinZ, box->mMaxX, box->mMaxY + hi, box->mMaxZ);
    case 2:
        return AABB::newTemp(box->mMinX, box->mMinY, box->mMinZ + lo, box->mMaxX, box->mMaxY, box->mMinZ + hi);
    case 3:
        return AABB::newTemp(box->mMinX, box->mMinY, box->mMaxZ + lo, box->mMaxX, box->mMaxY, box->mMaxZ + hi);
    case 4:
        return AABB::newTemp(box->mMinX + lo, box->mMinY, box->mMinZ, box->mMinX + hi, box->mMaxY, box->mMaxZ);
    default:
        return AABB::newTemp(box->mMaxX + lo, box->mMinY, box->mMinZ, box->mMaxX + hi, box->mMaxY, box->mMaxZ);
    }
}
double PistonMovingBlockEntity::getDeltaX(AABB* box, const Direction* direction, AABB* other)
{
    return direction->getAxisDirection() == Direction::AxisDirection::POSITIVE
        ? box->mMaxX - other->mMinX : other->mMaxX - box->mMinX;
}
double PistonMovingBlockEntity::getDeltaY(AABB* box, const Direction* direction, AABB* other)
{
    return direction->getAxisDirection() == Direction::AxisDirection::POSITIVE
        ? box->mMaxY - other->mMinY : other->mMaxY - box->mMinY;
}
double PistonMovingBlockEntity::getDeltaZ(AABB* box, const Direction* direction, AABB* other)
{
    return direction->getAxisDirection() == Direction::AxisDirection::POSITIVE
        ? box->mMaxZ - other->mMinZ : other->mMaxZ - box->mMinZ;
}
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
void PistonMovingBlockEntity::fixEntityWithinPistonBase(boost::shared_ptr<Entity> entity, const Direction* direction,
                                                        double distance)
{
    AABB* entityBox = entity->mBoundingBox;
    AABB* pistonBox = Block::FULL_BLOCK_SHAPE->move(mPos);
    if (!pistonBox->intersects(entityBox->mMinX, entityBox->mMinY, entityBox->mMinZ, entityBox->mMaxX,
                               entityBox->mMaxY, entityBox->mMaxZ))
    {
        return;
    }

    const Direction* opposite = direction->getOpposite();
    double movement = getMovement(pistonBox, opposite, entityBox) + 0.01;
    double overlapMovement = getMovement(pistonBox, opposite, entityBox->intersect(pistonBox)) + 0.01;
    if (fabs(movement - overlapMovement) < 0.01)
    {
        movement = std::min(movement, distance) + 0.01;
        NOCLIP_set(direction);
        entity->move(eMoverType_Piston, movement * opposite->getStepX(), movement * opposite->getStepY(),
                     movement * opposite->getStepZ(), false);
        NOCLIP_set(NULL);
    }
}
void PistonMovingBlockEntity::moveCollidedEntities(float progress)
{
    const Direction* direction = mExtending ? mDirection : mDirection->getOpposite();
    double distance = progress - mProgress;
    std::vector<AABB*> pieces;
    getCollisionRelatedBlockState()->addCollissionAABBs(mLevel, BlockPos::ZERO, AABB::newTemp(BlockPos::ZERO),
                                                         &pieces, boost::shared_ptr<Entity>(), true);
    if (pieces.empty())
        return;

    AABB* bounds = moveByPositionAndProgress(getMinMaxPiecesAABB(&pieces));
    std::vector<boost::shared_ptr<Entity> >* entities =
        mLevel->getEntities(boost::shared_ptr<Entity>(), getMovementArea(bounds, direction, distance)->minmax(bounds),
                            NULL);
    if (entities->empty())
        return;

    bool slime = mMovedState->getBlock() == Blocks::slime;
    for (unsigned int i = 0; i < entities->size(); ++i)
    {
        boost::shared_ptr<Entity> entity = entities->at(i);
        if (entity->getPistonPushReaction() == 3)
            continue;

        if (slime)
        {
            switch (direction->getAxis()->getId())
            {
            case 0:
                entity->mXd = direction->getStepX();
                break;
            case 1:
                entity->mYd = direction->getStepY();
                break;
            case 2:
                entity->mZd = direction->getStepZ();
                break;
            }
        }

        double movement = 0.0;
        AABB* entityBox = entity->mBoundingBox;
        for (unsigned int piece = 0; piece < pieces.size(); ++piece)
        {
            AABB* area = getMovementArea(moveByPositionAndProgress(pieces.at(piece)), direction, distance);
            if (area->intersects(entityBox->mMinX, entityBox->mMinY, entityBox->mMinZ, entityBox->mMaxX,
                                 entityBox->mMaxY, entityBox->mMaxZ))
            {
                movement = std::max(movement, getMovement(area, direction, entityBox));
                if (movement >= distance)
                    break;
            }
        }

        if (movement > 0.0)
        {
            movement = std::min(movement, distance) + 0.01;
            NOCLIP_set(direction);
            int moves = (entity->GetType() & EntityType::MINECART) == EntityType::MINECART ? 2 : 1;
            for (; moves > 0; --moves)
            {
                entity->move(eMoverType_Piston, movement * direction->getStepX(), movement * direction->getStepY(),
                             movement * direction->getStepZ(), false);
            }
            NOCLIP_set(NULL);
            if (!mExtending && mSourcePiston)
                fixEntityWithinPistonBase(entity, direction, distance);
        }
    }
}
void PistonMovingBlockEntity::tick()
{
    mLastProgress = mProgress;
    if (mProgress >= 1.0f)
    {
        stop();
        return;
    }

    const float nextProgress = mProgress + 0.5f;
    moveCollidedEntities(nextProgress);
    mProgress += nextProgress;
    if (mProgress >= 1.0f)
    {
        mProgress = 1.0f;
    }
}
void PistonMovingBlockEntity::addDataWalkers(DataFixerUpper*)
{
}
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

CompoundTag* PistonMovingBlockEntity::save(CompoundTag* tag)
{
    tag = BlockEntity::save(tag);
    tag->putInt(L"blockId", Block::getId(mMovedState->getBlock()));
    tag->putInt(L"blockData", mMovedState->getBlock()->convertBlockStateToLegacyData(mMovedState));
    tag->putInt(L"facing", mDirection->get3DDataValue());
    tag->putFloat(L"progress", mProgress);
    tag->putBoolean(L"extending", mExtending);
    tag->putBoolean(L"source", mSourcePiston);
    return tag;
}

void PistonMovingBlockEntity::addCollissionAABBs(Level* level, const BlockPos& pos, const AABB* entityBox,
                                                 std::vector<AABB*>* boxes, boost::shared_ptr<Entity> entity)
{
    if (!mExtending && mSourcePiston)
    {
        TypedBoxed<bool> extended(true);
        mMovedState->setBoxedValue(PistonBaseBlock::EXTENDED, &extended)
            ->addCollissionAABBs(level, pos, entityBox, boxes, entity, false);
    }

    const Direction* noClip = NOCLIP_get();
    if (mProgress < 1.0 && noClip == (mExtending ? mDirection : mDirection->getOpposite()))
        return;

    unsigned int firstNew = boxes->size();
    const BlockState* state;
    float xOff;
    float yOff;
    float zOff;
    if (isSourcePiston())
    {
        TypedBoxed<const Direction*> facing(mDirection);
        TypedBoxed<bool> shortArm(mExtending != (1.0f - mProgress < 0.25f));
        state = Blocks::piston_head->defaultBlockState()
                    ->setBoxedValue(PistonHeadBlock::FACING, &facing)
                    ->setBoxedValue(PistonHeadBlock::SHORT, &shortArm);
    }
    else
    {
        state = mMovedState;
    }
    double progress = getExtendedProgress(mProgress);
    xOff = static_cast<float>(mDirection->getStepX() * progress);
    yOff = static_cast<float>(mDirection->getStepY() * progress);
    zOff = static_cast<float>(mDirection->getStepZ() * progress);

    state->addCollissionAABBs(level, pos, entityBox->move(-xOff, -yOff, -zOff), boxes, entity, true);
    for (unsigned int i = firstNew; i < boxes->size(); ++i)
        (*boxes)[i] = boxes->at(i)->move(xOff, yOff, zOff);
}

boost::shared_ptr<BlockEntity> PistonMovingBlockEntity::clone()
{
    boost::shared_ptr<BlockEntity> result(new PistonMovingBlockEntity());
    BlockEntity::clone(result);
    PistonMovingBlockEntity* copy = static_cast<PistonMovingBlockEntity*>(result.get());
    copy->mMovedState = mMovedState;
    copy->mDirection = mDirection;
    copy->mExtending = mExtending;
    copy->mSourcePiston = mSourcePiston;
    copy->mProgress = mProgress;
    copy->mLastProgress = mLastProgress;
    return result;
}
unsigned long long PistonMovingBlockEntity::GetType()
{
    return BlockEntityType::PISTON_MOVING;
}
