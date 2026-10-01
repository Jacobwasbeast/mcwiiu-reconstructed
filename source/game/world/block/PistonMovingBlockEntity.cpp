#include "game/world/block/PistonMovingBlockEntity.hpp"
#include "platform/wiiu/TLSStorageCafe.hpp"
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
static unsigned int gPistonNoClipTlsIndex = TlsAlloc();
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
    TlsSetValue(gPistonNoClipTlsIndex, const_cast<Direction*>(direction));
}

const Direction* PistonMovingBlockEntity::NOCLIP_get()
{
    return static_cast<const Direction*>(TlsGetValue(gPistonNoClipTlsIndex));
}

const BlockState* PistonMovingBlockEntity::getBlockState()
{
    return mMovedState;
}
CompoundTag* PistonMovingBlockEntity::getUpdateTag()
{
    return save(new CompoundTag());
}
int PistonMovingBlockEntity::getData()
{
    return 0;
}

bool PistonMovingBlockEntity::isSourcePiston()
{
    return mSourcePiston;
}
float PistonMovingBlockEntity::getProgress(float partialTick)
{
    if (partialTick > 1.0f)
    {
        partialTick = 1.0f;
    }
    return mLastProgress + (mProgress - mLastProgress) * partialTick;
}
float PistonMovingBlockEntity::getExtendedProgress(float partialTick)
{
    return mExtending ? partialTick - 1.0f : 1.0f - partialTick;
}
float PistonMovingBlockEntity::getXOff(float partialTick)
{
    return mDirection->getStepX() * getExtendedProgress(getProgress(partialTick));
}
float PistonMovingBlockEntity::getYOff(float partialTick)
{
    return mDirection->getStepY() * getExtendedProgress(getProgress(partialTick));
}
float PistonMovingBlockEntity::getZOff(float partialTick)
{
    return mDirection->getStepZ() * getExtendedProgress(getProgress(partialTick));
}
const BlockState* PistonMovingBlockEntity::getCollisionRelatedBlockState()
{
    if (!isExtending() && isSourcePiston())
    {
        const BlockState* headState = Blocks::piston_head->defaultBlockState();
        headState = headState->setValue(
            PistonHeadBlock::TYPE,
            mMovedState->getBlock() == Blocks::sticky_piston ? pistonTypeSticky : pistonTypeNormal);
        headState = headState->setValue(PistonHeadBlock::FACING, mMovedState->getValue(PistonBaseBlock::FACING));
        return headState;
    }
    return mMovedState;
}
AABB* PistonMovingBlockEntity::getAABB(LevelSource* source, const BlockPos& pos, float partialTick)
{
    float progress = getExtendedProgress(partialTick);
    const BlockState* state = getCollisionRelatedBlockState();
    const Direction* direction = mDirection;
    const AABB* shape = state->getShape(source, pos);
    return shape->move(progress * direction->getStepX(), progress * direction->getStepY(),
                       progress * direction->getStepZ());
}
AABB* PistonMovingBlockEntity::getAABB(LevelSource* source, const BlockPos& pos)
{
    AABB* previous = getAABB(source, pos, mLastProgress);
    AABB* current = getAABB(source, pos, mProgress);
    return current->minmax(previous);
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
    case 2:
        return AABB::newTemp(box->mMinX, box->mMinY, box->mMinZ + lo, box->mMaxX, box->mMaxY, box->mMinZ + hi);
    case 3:
        return AABB::newTemp(box->mMinX, box->mMinY, box->mMaxZ + lo, box->mMaxX, box->mMaxY, box->mMaxZ + hi);
    case 4:
        return AABB::newTemp(box->mMinX + lo, box->mMinY, box->mMinZ, box->mMinX + hi, box->mMaxY, box->mMaxZ);
    case 5:
        return AABB::newTemp(box->mMaxX + lo, box->mMinY, box->mMinZ, box->mMaxX + hi, box->mMaxY, box->mMaxZ);
    default:
        return AABB::newTemp(box->mMinX, box->mMaxY + lo, box->mMinZ, box->mMaxX, box->mMaxY + hi, box->mMaxZ);
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
    switch (direction->getAxis()->getId())
    {
    case 0:
        return getDeltaX(box, direction, other);
    case 2:
        return getDeltaZ(box, direction, other);
    default:
        return getDeltaY(box, direction, other);
    }
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
    if (!pieces.empty())
    {
        AABB* bounds = moveByPositionAndProgress(getMinMaxPiecesAABB(&pieces));
        std::vector<boost::shared_ptr<Entity> >* entities =
            mLevel->getEntities(boost::shared_ptr<Entity>(), getMovementArea(bounds, direction, distance)->minmax(bounds),
                                NULL);
        if (!entities->empty())
        {
            bool slime = mMovedState->getBlock() == Blocks::slime;
            for (unsigned int i = 0; i < entities->size(); ++i)
            {
                boost::shared_ptr<Entity> entity = entities->at(i);
                if (entity->getPistonPushReaction() == 3)
                    continue;

                if (slime)
                {
                    int axis = direction->getAxis()->getId();
                    if (axis == 0)
                    {
                        entity->mXd = direction->getStepX();
                    }
                    else if (axis == 1)
                    {
                        entity->mYd = direction->getStepY();
                    }
                    else if (axis == 2)
                    {
                        entity->mZd = direction->getStepZ();
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
                        double areaMovement = getMovement(area, direction, entityBox);
                        if (movement < areaMovement)
                            movement = areaMovement;
                        if (movement >= distance)
                            break;
                    }
                }

                if (movement > 0.0)
                {
                    double amount = movement < distance ? movement : distance;
                    amount += 0.01;
                    NOCLIP_set(direction);
                    int moves = (entity->GetType() & EntityType::MINECART) == EntityType::MINECART ? 2 : 1;
                    do
                    {
                        entity->move(eMoverType_Piston, amount * direction->getStepX(),
                                     amount * direction->getStepY(), amount * direction->getStepZ(), false);
                        --moves;
                    } while (moves != 0);
                    NOCLIP_set(NULL);
                    if (!mExtending && mSourcePiston)
                        fixEntityWithinPistonBase(entity, direction, distance);
                }
            }
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
    mMovedState = Block::byId(tag->getInt(L"blockId"))->getBlockState(tag->getInt(L"blockData"));
    mDirection = Direction::from3DDataValue(tag->getInt(L"facing"));
    mProgress = mLastProgress = tag->getFloat(L"progress");
    mExtending = tag->getBoolean(L"extending");
    mSourcePiston = tag->getBoolean(L"source");
}

CompoundTag* PistonMovingBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putInt(L"blockId", Block::getId(mMovedState->getBlock()));
    tag->putInt(L"blockData", mMovedState->getBlock()->convertBlockStateToLegacyData(mMovedState));
    tag->putInt(L"facing", mDirection->get3DDataValue());
    tag->putFloat(L"progress", mLastProgress);
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
    boost::shared_ptr<PistonMovingBlockEntity> result(new PistonMovingBlockEntity());
    BlockEntity::clone(result);
    result->mMovedState = mMovedState;
    result->mDirection = mDirection;
    result->mExtending = mExtending;
    result->mSourcePiston = mSourcePiston;
    result->mProgress = mProgress;
    result->mLastProgress = mLastProgress;
    return result;
}
unsigned long long PistonMovingBlockEntity::GetType()
{
    return BlockEntityType::PISTON_MOVING;
}
