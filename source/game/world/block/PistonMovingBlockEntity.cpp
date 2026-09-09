#include "game/world/block/PistonMovingBlockEntity.hpp"
#include "game/core/Direction.hpp"
#include "game/data/CompoundTag.hpp"
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
static const Direction* gPistonNoClipDirection = NULL;
PistonMovingBlockEntity::PistonMovingBlockEntity(const BlockState* state, const Direction* direction, bool extending, bool source, CompoundTag* lastTick)
    : BlockEntity(), mMovedState(state), mDirection(direction), mExtending(extending), mSourcePiston(source), mLastTick(lastTick), mProgress(0.0f), mLastProgress(0.0f)
{
}
PistonMovingBlockEntity::PistonMovingBlockEntity()
    : BlockEntity(), mMovedState(NULL), mDirection(NULL), mExtending(false),
      mSourcePiston(false), mLastTick(NULL), mProgress(0.0f), mLastProgress(0.0f)
{
}

PistonMovingBlockEntity::~PistonMovingBlockEntity()
{
    delete mLastTick;
}

BlockEntity* PistonMovingBlockEntity::create()
{
    return new PistonMovingBlockEntity();
}
void PistonMovingBlockEntity::stop()
{
    if (mLevel != NULL)
    {
        mLevel->removeBlockEntity(mPos);
        setRemoved();
        if (mMovedState != NULL)
        {
            mLevel->setBlockAndUpdate(mPos, mMovedState);
            mLevel->neighborChanged(mPos, mMovedState->getBlock(), mPos);
        }
    }
    delete mLastTick;
    mLastTick = NULL;
}
void PistonMovingBlockEntity::finalTick()
{
    if (mProgress < 1.0f)
    {
        mProgress = 1.0f;
        mLastProgress = 1.0f;
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
void PistonMovingBlockEntity::moveByPositionAndProgress(AABB* box)
{
    AABB* moved = box->move(getXOff(mProgress), getYOff(mProgress), getZOff(mProgress));
    box->set(moved);
}
AABB* PistonMovingBlockEntity::getMinMaxPiecesAABB(std::vector<AABB*>* pieces)
{
    if (pieces->empty())
    {
        return NULL;
    }
    AABB* result = AABB::newTemp(pieces->at(0));
    for (std::size_t i = 1; i < pieces->size(); ++i)
    {
        result->minmax(pieces->at(i));
    }
    return result;
}
AABB* PistonMovingBlockEntity::getMovementArea(AABB* box, const Direction* direction, double distance)
{
    Vec3i normal = direction->getNormal();
    return box->expand(normal.x * distance, normal.y * distance, normal.z * distance);
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
    BOOST_ASSERT(entity);
    BOOST_ASSERT(entity->mBoundingBox);

    AABB* pistonBox = AABB::newTemp(mPos);
    if (!entity->mBoundingBox->intersects(pistonBox->mMinX, pistonBox->mMinY, pistonBox->mMinZ,
                                          pistonBox->mMaxX, pistonBox->mMaxY, pistonBox->mMaxZ))
    {
        return;
    }

    const Direction* opposite = direction->getOpposite();
    double movement = getMovement(entity->mBoundingBox, opposite, pistonBox) + 0.01;
    AABB* overlap = entity->mBoundingBox->intersect(pistonBox);
    double overlapMovement = getMovement(entity->mBoundingBox, opposite, overlap) + 0.01;
    if (movement - overlapMovement < 0.01)
    {
        if (movement < distance)
        {
            distance = movement;
        }
        entity->move(eMoverType_Piston,
                     distance * opposite->getStepX(),
                     distance * opposite->getStepY(),
                     distance * opposite->getStepZ(), true);
    }
}
void PistonMovingBlockEntity::moveCollidedEntities(float progress)
{
    const Direction* direction = mExtending ? mDirection : mDirection->getOpposite();
    double distance = static_cast<double>(progress - mProgress);
    std::vector<AABB*> pieces;
    mMovedState->addCollissionAABBs(mLevel, BlockPos::ZERO, NULL, &pieces,
                                    boost::shared_ptr<Entity>(), true);
    if (pieces.empty())
    {
        return;
    }

    AABB* moved = getMinMaxPiecesAABB(&pieces);
    moveByPositionAndProgress(moved);
    AABB* movementArea = getMovementArea(moved, direction, distance);
    movementArea->minmax(moved);
    std::vector<boost::shared_ptr<Entity> > entityStorage;
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntities(
        boost::shared_ptr<Entity>(), movementArea, &entityStorage);
    if (entities == NULL)
    {
        return;
    }

    for (std::size_t i = 0; i < entities->size(); ++i)
    {
        boost::shared_ptr<Entity> entity = entities->at(i);
        BOOST_ASSERT(entity);
        if (entity->getPistonPushReaction() == PISTON_PUSH_IGNORE)
        {
            continue;
        }

        double movement = 0.0;
        for (std::size_t piece = 0; piece < pieces.size(); ++piece)
        {
            AABB pieceBox(*pieces[piece]);
            AABB* movedPiece = pieceBox.move(getXOff(mProgress), getYOff(mProgress), getZOff(mProgress));
            AABB* area = getMovementArea(movedPiece, direction, distance);
            area->minmax(movedPiece);
            if (entity->mBoundingBox->intersects(area->mMinX, area->mMinY, area->mMinZ,
                                                  area->mMaxX, area->mMaxY, area->mMaxZ))
            {
                double candidate = getMovement(area, direction, entity->mBoundingBox);
                if (candidate > movement)
                {
                    movement = candidate;
                }
            }
        }

        if (movement > 0.0)
        {
            NOCLIP_set(direction);
            entity->move(eMoverType_Piston,
                         movement * direction->getStepX(),
                         movement * direction->getStepY(),
                         movement * direction->getStepZ(), true);
            NOCLIP_set(NULL);
            if (!mExtending && mSourcePiston)
            {
                fixEntityWithinPistonBase(entity, direction, distance);
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
    std::vector<AABB*> pieces;
    const BlockState* state = getCollisionRelatedBlockState();
    state->addCollissionAABBs(level, pos, entityBox, &pieces, entity, true);
    for (std::size_t i = 0; i < pieces.size(); ++i)
    {
        boxes->push_back(pieces[i]->move(getXOff(mProgress), getYOff(mProgress), getZOff(mProgress)));
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
unsigned long long PistonMovingBlockEntity::GetType()
{
    return 0x0000000001200205ULL;
}
