#include "game/world/block/DetectorRailBlock.hpp"

#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/Minecart.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BaseCommandBlock.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/MinecartCommandBlock.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<BaseRailBlock::RailShape*>* DetectorRailBlock::SHAPE = NULL;
BooleanProperty* DetectorRailBlock::POWERED = static_cast<BooleanProperty*>(BooleanProperty::create(L"powered"));

void DetectorRailBlock::blockStaticCtor()
{
    boost::unordered_set<BaseRailBlock::RailShape*> values;
    for (int i = 0; i < 10; ++i)
        values.insert(BaseRailBlock::RailShape::byData(i));
    DetectorRailBlock::SHAPE = EnumProperty<BaseRailBlock::RailShape*>::create(
        L"shape", typeid(BaseRailBlock::RailShape*), values,
        new Predicates<BaseRailBlock::RailShape*>::FunctionPredicate(railpredicate));
}

DetectorRailBlock::DetectorRailBlock()
    : BaseRailBlock(true), mIconOff(NULL), mIconOn(NULL)
{
    DerivedInit();
    setTicking(true);
    TypedBoxed<bool> poweredValue(false);
    TypedBoxed<BaseRailBlock::RailShape*> shapeValue(BaseRailBlock::RailShape::NORTH_SOUTH);
    const BlockState* state = mStateDefinition->any()->setBoxedValue(DetectorRailBlock::POWERED, &poweredValue);
    registerDefaultState(state->setBoxedValue(DetectorRailBlock::SHAPE, &shapeValue));
}

DetectorRailBlock::~DetectorRailBlock()
{
}

int DetectorRailBlock::getTickDelay(Level*) { return 20; }

bool DetectorRailBlock::isSignalSource(const BlockState*) { return true; }

void DetectorRailBlock::entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                                     const boost::shared_ptr<Entity>&)
{
    if (!level->mIsClientSide && !state->getValue(DetectorRailBlock::POWERED))
        checkPressed(level, pos, state);
}

void DetectorRailBlock::randomTick(Level*, const BlockPos&, const BlockState*, Random*)
{
}

void DetectorRailBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!level->mIsClientSide && state->getValue(DetectorRailBlock::POWERED))
        checkPressed(level, pos, state);
}

int DetectorRailBlock::getSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction*)
{
    return state->getValue(DetectorRailBlock::POWERED) ? 15 : 0;
}

AABB* DetectorRailBlock::getSearchBB(const BlockPos& pos)
{
    return AABB::newTemp(pos.x + 0.2f, pos.y, pos.z + 0.2f,
                         pos.x + 1 - 0.2f, pos.y + 1 - 0.2f, pos.z + 1 - 0.2f);
}

std::vector<boost::shared_ptr<Entity> >* DetectorRailBlock::getInteractingMinecartOfType(
    Level* level, const BlockPos& pos, const std::type_info& type,
    const Predicate<boost::shared_ptr<Entity> >* predicate)
{
    const AABB* box = getSearchBB(pos);
    if (predicate == NULL)
        return level->getEntitiesOfClass(type, box);
    return level->getEntitiesOfClass(type, box, predicate, false);
}

void DetectorRailBlock::checkPressed(Level* level, const BlockPos& pos, const BlockState* state)
{
    bool powered = state->getValue(DetectorRailBlock::POWERED);
    std::vector<boost::shared_ptr<Entity> >* minecarts =
        getInteractingMinecartOfType(level, pos, typeid(Minecart), NULL);
    bool occupied = !minecarts->empty();
    if (occupied && !powered)
    {
        TypedBoxed<bool> poweredValue(true);
        level->setBlock(pos, state->setBoxedValue(DetectorRailBlock::POWERED, &poweredValue), Block::UPDATE_ALL, false);
        level->updateNeighborsAt(pos, this, false);
        level->updateNeighborsAt(pos.below(), this, false);
        level->setBlocksDirty(pos, pos);
    }
    if (!occupied && powered)
    {
        TypedBoxed<bool> poweredValue(false);
        level->setBlock(pos, state->setBoxedValue(DetectorRailBlock::POWERED, &poweredValue), Block::UPDATE_ALL, false);
        level->updateNeighborsAt(pos, this, false);
        level->updateNeighborsAt(pos.below(), this, false);
        level->setBlocksDirty(pos, pos);
    }
    if (occupied)
        level->addToTickNextTick(pos, this, getTickDelay(level));
    level->updateNeighbourForOutputSignal(pos, this);
    delete minecarts;
}

void DetectorRailBlock::updatePowerToConnected(Level* level, const BlockPos& pos,
                                               const BlockState* state, bool)
{
    BaseRailBlock::Rail rail(level, pos, state);
    std::vector<BlockPos>* connections = rail.getConnections();
    for (std::vector<BlockPos>::const_iterator it = connections->begin(); it != connections->end(); ++it)
    {
        const BlockState* neighbor = level->getBlockState(*it);
        if (neighbor != NULL)
            neighbor->neighborChanged(level, *it, neighbor->getBlock(), pos);
    }
}

int DetectorRailBlock::getDirectSignal(const BlockState* state, LevelSource*, const BlockPos&,
                                       const Direction* direction)
{
    return state->getValue(DetectorRailBlock::POWERED) && direction == Direction::UP ? 15 : 0;
}

void DetectorRailBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    BaseRailBlock::onPlace(level, pos, state);
    checkPressed(level, pos, state);
}

const AbstractProperty<BaseRailBlock::RailShape*>* DetectorRailBlock::getShapeProperty()
{
    return DetectorRailBlock::SHAPE;
}

bool DetectorRailBlock::hasAnalogOutputSignal(const BlockState*) { return true; }

int DetectorRailBlock::getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos)
{
    const BlockState* railState = level->getBlockState(pos);
    if (!railState->getValue(DetectorRailBlock::POWERED))
        return 0;
    std::vector<boost::shared_ptr<Entity> >* commandCarts =
        getInteractingMinecartOfType(level, pos, typeid(MinecartCommandBlock), NULL);
    if (!commandCarts->empty())
    {
        boost::shared_ptr<MinecartCommandBlock> commandCart =
            boost::static_pointer_cast<MinecartCommandBlock>(commandCarts->at(0));
        int successCount = commandCart->getCommandBlock()->getSuccessCount();
        delete commandCarts;
        return successCount;
    }
    delete commandCarts;
    return state->getValue(DetectorRailBlock::POWERED) ? 15 : 0;
}

const BlockState* DetectorRailBlock::getBlockState(int data)
{
    TypedBoxed<BaseRailBlock::RailShape*> shapeValue(BaseRailBlock::RailShape::byData(data & 7));
    TypedBoxed<bool> poweredValue((data & 8) != 0);
    const BlockState* state = defaultBlockState()->setBoxedValue(DetectorRailBlock::SHAPE, &shapeValue);
    return state->setBoxedValue(DetectorRailBlock::POWERED, &poweredValue);
}

int DetectorRailBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(DetectorRailBlock::SHAPE)->getData();
    if (state->getValue(DetectorRailBlock::POWERED))
        data |= 8;
    return data;
}

const BlockState* DetectorRailBlock::rotate(const BlockState* state, Rotation* rotation)
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise180:
        switch (state->getValue(DetectorRailBlock::SHAPE)->getData())
        {
        case 2:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_WEST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 3:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_EAST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 4:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_SOUTH);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 5:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_NORTH);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_WEST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_EAST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_EAST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_WEST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        }
    case Rotation::eTYPE_CounterClockwise90:
        switch (state->getValue(DetectorRailBlock::SHAPE)->getData())
        {
        case 0:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::EAST_WEST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 1:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_SOUTH);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 2:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_NORTH);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 3:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_SOUTH);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 4:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_WEST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 5:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_EAST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_EAST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_EAST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_WEST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_WEST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        }
    case Rotation::eTYPE_Clockwise90:
        switch (state->getValue(DetectorRailBlock::SHAPE)->getData())
        {
        case 0:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::EAST_WEST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 1:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_SOUTH);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 2:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_SOUTH);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 3:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_NORTH);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 4:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_EAST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 5:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_WEST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_WEST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_WEST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_EAST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_EAST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        }
    default:
        break;
    }

    return state;
}

const BlockState* DetectorRailBlock::mirror(const BlockState* state, Mirror* mirror)
{
    if (mirror->getType() == Mirror::eTYPE_LeftRight)
    {
        switch (state->getValue(DetectorRailBlock::SHAPE)->getData())
        {
        case 4:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_SOUTH);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 5:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_NORTH);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_EAST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_WEST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_WEST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_EAST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        }
    }
    else if (mirror->getType() == Mirror::eTYPE_FrontBack)
    {
        switch (state->getValue(DetectorRailBlock::SHAPE)->getData())
        {
        case 2:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_WEST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 3:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_EAST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_WEST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_EAST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_EAST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_WEST);
            return state->setBoxedValue(DetectorRailBlock::SHAPE, &shape);
        }
        }
    }

    return Block::mirror(state, mirror);
}

BlockStateDefinition* DetectorRailBlock::createBlockStateDefinition()
{
    const Property* properties[2] = { DetectorRailBlock::SHAPE, DetectorRailBlock::POWERED };
    return new BlockStateDefinition(this, properties, 2);
}

TextureAtlasSprite* DetectorRailBlock::getTexture(const Direction*, const BlockState* state)
{
    return state->getValue(DetectorRailBlock::POWERED) ? mIconOn : mIconOff;
}

void DetectorRailBlock::registerIcons(IconRegister* iconRegister)
{
    mIconOff = iconRegister->registerIcon(L"detectorRail");
    mIconOn = iconRegister->registerIcon(L"detectorRail_on");
}
