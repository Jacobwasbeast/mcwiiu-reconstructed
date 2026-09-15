#include "game/world/block/BaseRailBlock.hpp"

#include <typeinfo>

#include "game/core/Direction.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/Material.hpp"

static const AABB* gRailFlatAABB = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.125, 1.0);
static const AABB* gRailAscendingAABB = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.5, 1.0);

BaseRailBlock::RailShape* gRailShapes[10] =
{
    new BaseRailBlock::RailShape(0, L"north_south"),
    new BaseRailBlock::RailShape(1, L"east_west"),
    new BaseRailBlock::RailShape(2, L"ascending_east"),
    new BaseRailBlock::RailShape(3, L"ascending_west"),
    new BaseRailBlock::RailShape(4, L"ascending_north"),
    new BaseRailBlock::RailShape(5, L"ascending_south"),
    new BaseRailBlock::RailShape(6, L"south_east"),
    new BaseRailBlock::RailShape(7, L"south_west"),
    new BaseRailBlock::RailShape(8, L"north_west"),
    new BaseRailBlock::RailShape(9, L"north_east")
};

bool railfilter(BaseRailBlock::RailShape* shape)
{
    return shape != gRailShapes[9] && shape != gRailShapes[8] &&
           shape != gRailShapes[6] && shape != gRailShapes[7];
}

bool railpredicate(BaseRailBlock::RailShape* shape)
{
    return shape != gRailShapes[9] && shape != gRailShapes[8] &&
           shape != gRailShapes[6] && shape != gRailShapes[7];
}

static const AABB* gRailClipAABB = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.125, 1.0);

BaseRailBlock::RailShape::RailShape(int data, const std::wstring& name)
    : mData(data), mName(name)
{
}

bool BaseRailBlock::RailShape::isAscending()
{
    return this == gRailShapes[2] || this == gRailShapes[3] ||
           this == gRailShapes[4] || this == gRailShapes[5];
}

int BaseRailBlock::RailShape::getData()
{
    return mData;
}

std::wstring BaseRailBlock::RailShape::toString() const
{
    return mName;
}

BaseRailBlock::RailShape* BaseRailBlock::RailShape::byData(int data)
{
    if (data < 0 || data > 9)
        data = 0;
    return gRailShapes[data];
}

std::wstring BaseRailBlock::RailShape::getSerializedName() const
{
    return mName;
}

BaseRailBlock::BaseRailBlock(bool isPowered)
    : Block(Material::DECORATION), mIsPowered(isPowered)
{
    mBlockType = EBLOCK_TYPE_RAIL;
    setThickness(0.07f);
}

BaseRailBlock::~BaseRailBlock()
{
}

bool BaseRailBlock::isRail(const Block* block)
{
    return block != NULL && const_cast<Block*>(block)->instanceof(EBLOCK_TYPE_RAIL);
}

bool BaseRailBlock::isRail(Level* level, const BlockPos& pos)
{
    return level != NULL && isRail(level->getBlockState(pos));
}

bool BaseRailBlock::isRail(const BlockState* state)
{
    return state != NULL && isRail(state->getBlock());
}

const AABB* BaseRailBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return gRailClipAABB;
}

bool BaseRailBlock::isSolidRender(const BlockState*) const
{
    return false;
}

const AABB* BaseRailBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    if (state->getBlock() == this)
    {
        RailShape* shape = static_cast<TypedBoxed<RailShape*>*>(
            state->getBoxedValue(getShapeProperty()))->getValue();
        if (shape != NULL && shape->isAscending())
        {
            return gRailAscendingAABB;
        }
    }
    return gRailFlatAABB;
}

EBlockFaceShape BaseRailBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

bool BaseRailBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool BaseRailBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return level->getBlockState(pos.below())->isTopSolidBlocking()
        || Block::isExceptionForTopSolidBlocking(level->getBlockState(pos.below())->getBlock());
}

void BaseRailBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!level->mIsClientSide && mIsPowered)
        updateDir(level, pos, state, true);
}

void BaseRailBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                    Block* block, const BlockPos& fromPos)
{
    if (level->mIsClientSide)
    {
        return;
    }

    RailShape* shape = static_cast<TypedBoxed<RailShape*>*>(
        state->getBoxedValue(getShapeProperty()))->getValue();
    bool unsupported = false;
    if (!level->getBlockState(pos.below())->isTopSolidBlocking()
        && !Block::isExceptionForTopSolidBlocking(level->getBlockState(pos.below())->getBlock()))
    {
        unsupported = true;
    }

    if (shape == gRailShapes[2]
        && !level->getBlockState(pos.east())->isTopSolidBlocking()
        && !Block::isExceptionForTopSolidBlocking(level->getBlockState(pos.east())->getBlock()))
    {
        unsupported = true;
    }
    else if (shape == gRailShapes[3]
        && !level->getBlockState(pos.west())->isTopSolidBlocking()
        && !Block::isExceptionForTopSolidBlocking(level->getBlockState(pos.west())->getBlock()))
    {
        unsupported = true;
    }
    else if (shape == gRailShapes[4]
        && !level->getBlockState(pos.north())->isTopSolidBlocking()
        && !Block::isExceptionForTopSolidBlocking(level->getBlockState(pos.north())->getBlock()))
    {
        unsupported = true;
    }
    else if (shape == gRailShapes[5]
        && !level->getBlockState(pos.south())->isTopSolidBlocking()
        && !Block::isExceptionForTopSolidBlocking(level->getBlockState(pos.south())->getBlock()))
    {
        unsupported = true;
    }

    if (unsupported && !level->isEmptyBlock(pos))
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
        return;
    }
    updateState(state, level, pos, block);
}

void BaseRailBlock::updateState(const BlockState*, Level*, const BlockPos&, Block*)
{
}

const BlockState* BaseRailBlock::updateDir(Level* level, const BlockPos& pos, const BlockState* state, bool forceUpdate)
{
    Rail* rail = new Rail(level, pos, state);
    rail->place(false, forceUpdate);
    const BlockState* result = rail->getState();
    delete rail;
    return result;
}

EPistonPushReaction BaseRailBlock::getPistonPushReaction(const BlockState*)
{
    return PISTON_PUSH_NORMAL;
}

int BaseRailBlock::getRenderLayer()
{
    return 0;
}

void BaseRailBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onRemove(level, pos, state);
    TypedBoxed<RailShape*>* boxed = static_cast<TypedBoxed<RailShape*>*>(state->getBoxedValue(getShapeProperty()));
    RailShape* shape = boxed->getValue();
    if (shape != NULL && shape->isAscending())
        level->updateNeighborsAt(pos.above(), this, false);
    if (mIsPowered)
    {
        level->updateNeighborsAt(pos, this, false);
        level->updateNeighborsAt(pos.below(), this, false);
    }
}

EBlockRenderShape BaseRailBlock::getRenderShape(const BlockState*)
{
    return static_cast<EBlockRenderShape>(9);
}

BaseRailBlock::Rail::Rail(Level* level, const BlockPos& pos, const BlockState* state)
    : mLevel(level), mPos(pos), mBlock(state->getBlock()), mState(state), mPowered(false)
{
    TypedBoxed<RailShape*>* boxed = static_cast<TypedBoxed<RailShape*>*>(state->getBoxedValue(
        static_cast<BaseRailBlock*>(mBlock)->getShapeProperty()));
    mPowered = static_cast<BaseRailBlock*>(mBlock)->mIsPowered;
    updateConnections(boxed->getValue());
}

const BlockState* BaseRailBlock::Rail::getState()
{
    return mState;
}

void BaseRailBlock::Rail::updateConnections(RailShape* shape)
{
    mConnections.clear();
    if (shape == NULL)
        return;
    switch (shape->getData())
    {
    case 0: mConnections.push_back(mPos.north()); mConnections.push_back(mPos.south()); break;
    case 1: mConnections.push_back(mPos.west()); mConnections.push_back(mPos.east()); break;
    case 2: mConnections.push_back(mPos.west()); mConnections.push_back(mPos.east().above()); break;
    case 3: mConnections.push_back(mPos.west().above()); mConnections.push_back(mPos.east()); break;
    case 4: mConnections.push_back(mPos.north().above()); mConnections.push_back(mPos.south()); break;
    case 5: mConnections.push_back(mPos.north()); mConnections.push_back(mPos.south().above()); break;
    case 6: mConnections.push_back(mPos.east()); mConnections.push_back(mPos.south()); break;
    case 7: mConnections.push_back(mPos.west()); mConnections.push_back(mPos.south()); break;
    case 8: mConnections.push_back(mPos.west()); mConnections.push_back(mPos.north()); break;
    case 9: mConnections.push_back(mPos.east()); mConnections.push_back(mPos.north()); break;
    }
}

BaseRailBlock::Rail* BaseRailBlock::Rail::getRail(const BlockPos& pos)
{
    const BlockState* state = mLevel->getBlockState(pos);
    if (!BaseRailBlock::isRail(state))
    {
        state = mLevel->getBlockState(pos.above());
        if (!BaseRailBlock::isRail(state))
        {
            state = mLevel->getBlockState(pos.below());
            if (!BaseRailBlock::isRail(state))
                return NULL;
            return new Rail(mLevel, pos.below(), state);
        }
        return new Rail(mLevel, pos.above(), state);
    }
    return new Rail(mLevel, pos, state);
}

bool BaseRailBlock::Rail::hasConnection(const BlockPos& pos)
{
    for (std::vector<BlockPos>::const_iterator it = mConnections.begin(); it != mConnections.end(); ++it)
        if (it->x == pos.x && it->y == pos.y && it->z == pos.z)
            return true;
    return false;
}

bool BaseRailBlock::Rail::connectsTo(Rail* rail)
{
    return rail != NULL && hasConnection(rail->mPos);
}

bool BaseRailBlock::Rail::canConnectTo(Rail* rail)
{
    return connectsTo(rail) || mConnections.size() < 2;
}

void BaseRailBlock::Rail::removeSoftConnections()
{
    for (std::vector<BlockPos>::iterator it = mConnections.begin(); it != mConnections.end(); )
    {
        Rail* rail = getRail(*it);
        if (rail == NULL || !rail->connectsTo(this))
            it = mConnections.erase(it);
        else
            ++it;
        delete rail;
    }
}

bool BaseRailBlock::Rail::hasNeighborRail(const BlockPos& pos)
{
    Rail* rail = getRail(pos);
    if (rail == NULL)
        return false;
    rail->removeSoftConnections();
    bool result = rail->canConnectTo(this);
    delete rail;
    return result;
}

void BaseRailBlock::Rail::connectTo(Rail* rail)
{
    mConnections.push_back(rail->mPos);

    BlockPos north = mPos.north();
    BlockPos south = mPos.south();
    BlockPos west = mPos.west();
    BlockPos east = mPos.east();

    bool hasNorth = hasConnection(north);
    bool hasSouth = hasConnection(south);
    bool hasWest = hasConnection(west);
    bool hasEast = hasConnection(east);

    RailShape* shape = NULL;
    if (hasNorth || hasSouth)
        shape = gRailShapes[0];
    if (hasWest || hasEast)
        shape = gRailShapes[1];

    if (!mPowered)
    {
        if (!hasSouth)
        {
            if (hasNorth)
            {
                if (hasWest)
                {
                    shape = gRailShapes[8];
                    if (hasEast)
                        shape = gRailShapes[1];
                }
                else if (hasEast)
                {
                    shape = gRailShapes[9];
                }
            }
        }
        else if (!hasWest)
        {
            if (hasEast)
                shape = hasNorth ? gRailShapes[1] : gRailShapes[6];
            else
                shape = gRailShapes[0];
        }
        else
        {
            shape = gRailShapes[7];
            if (hasNorth || hasEast)
                shape = gRailShapes[1];
        }
    }

    if (shape == gRailShapes[0])
    {
        if (BaseRailBlock::isRail(mLevel, north.above()))
            shape = gRailShapes[4];
        if (BaseRailBlock::isRail(mLevel, south.above()))
            shape = gRailShapes[5];
    }
    if (shape == gRailShapes[1])
    {
        if (BaseRailBlock::isRail(mLevel, east.above()))
            shape = gRailShapes[2];
        if (BaseRailBlock::isRail(mLevel, west.above()))
            shape = gRailShapes[3];
    }
    if (shape == NULL)
        shape = gRailShapes[0];

    const Property* shapeProperty = static_cast<BaseRailBlock*>(mBlock)->getShapeProperty();
    mState = mState->setBoxedValue(shapeProperty, new TypedBoxed<RailShape*>(shape));
    mLevel->setBlock(mPos, mState, 3, false);
}

BaseRailBlock::Rail* BaseRailBlock::Rail::place(bool powered, bool forceUpdate)
{
    bool north = hasNeighborRail(mPos.north());
    bool south = hasNeighborRail(mPos.south());
    bool west = hasNeighborRail(mPos.west());
    bool east = hasNeighborRail(mPos.east());
    int data = 0;
    if (west && east && !north && !south) data = 1;
    else if (west && east) data = west && !east ? 3 : 1;
    else if (north && south) data = 0;
    else if (north && east) data = 9;
    else if (north && west) data = 8;
    else if (south && east) data = 6;
    else if (south && west) data = 7;
    else if (north) data = 0;
    else if (south) data = 0;
    else if (west || east) data = 1;
    RailShape* shape = RailShape::byData(data);
    updateConnections(shape);
    mState = mState->setBoxedValue(static_cast<BaseRailBlock*>(mBlock)->getShapeProperty(),
                                   new TypedBoxed<RailShape*>(shape));
    if (forceUpdate)
        mLevel->setBlockAndUpdate(mPos, mState);
    return this;
}

std::vector<BlockPos>* BaseRailBlock::Rail::getConnections()
{
    return &mConnections;
}

bool BaseRailBlock::Rail::hasRail(const BlockPos& pos)
{
    return BaseRailBlock::isRail(mLevel, pos) || BaseRailBlock::isRail(mLevel, pos.above()) ||
           BaseRailBlock::isRail(mLevel, pos.below());
}

int BaseRailBlock::Rail::countPotentialConnections()
{
    int count = 0;
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        if (hasRail(mPos.relative(directions.data[i])))
            ++count;
    }
    return count;
}
