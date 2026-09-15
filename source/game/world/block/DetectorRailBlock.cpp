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

static EnumProperty<BaseRailBlock::RailShape*>* gDetectorRailShapeProperty = NULL;
static const AbstractProperty<bool>* gDetectorRailPoweredProperty =
    static_cast<const AbstractProperty<bool>*>(BooleanProperty::create(L"powered"));

void DetectorRailBlock::blockStaticCtor()
{
    boost::unordered_set<BaseRailBlock::RailShape*> values;
    for (int i = 0; i < 10; ++i)
        values.insert(BaseRailBlock::RailShape::byData(i));
    gDetectorRailShapeProperty = EnumProperty<BaseRailBlock::RailShape*>::create(
        L"shape", typeid(BaseRailBlock::RailShape*), values,
        new Predicates<BaseRailBlock::RailShape*>::FunctionPredicate(railpredicate));
}

DetectorRailBlock::DetectorRailBlock()
    : BaseRailBlock(true), mIconOff(NULL), mIconOn(NULL)
{
    DerivedInit();
    setTicking(true);
    TypedBoxed<bool> poweredValue(false);
    TypedBoxed<BaseRailBlock::RailShape*> shapeValue(BaseRailBlock::RailShape::byData(0));
    const BlockState* state = mStateDefinition->any()->setBoxedValue(gDetectorRailPoweredProperty, &poweredValue);
    registerDefaultState(state->setBoxedValue(gDetectorRailShapeProperty, &shapeValue));
}

DetectorRailBlock::~DetectorRailBlock()
{
}

int DetectorRailBlock::getTickDelay(Level*) { return 20; }

bool DetectorRailBlock::isSignalSource(const BlockState*) { return true; }

void DetectorRailBlock::entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                                     const boost::shared_ptr<Entity>&)
{
    if (!level->mIsClientSide && !state->getValue(gDetectorRailPoweredProperty))
        checkPressed(level, pos, state);
}

void DetectorRailBlock::randomTick(Level*, const BlockPos&, const BlockState*, Random*)
{
}

void DetectorRailBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!level->mIsClientSide && state->getValue(gDetectorRailPoweredProperty))
        checkPressed(level, pos, state);
}

int DetectorRailBlock::getSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction*)
{
    return state->getValue(gDetectorRailPoweredProperty) ? 15 : 0;
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
    bool powered = state->getValue(gDetectorRailPoweredProperty);
    std::vector<boost::shared_ptr<Entity> >* minecarts =
        getInteractingMinecartOfType(level, pos, typeid(Minecart), NULL);
    bool occupied = !minecarts->empty();
    if (occupied && !powered)
    {
        TypedBoxed<bool> poweredValue(true);
        level->setBlock(pos, state->setBoxedValue(gDetectorRailPoweredProperty, &poweredValue), 3, false);
        level->updateNeighborsAt(pos, this, false);
        level->updateNeighborsAt(pos.below(), this, false);
        level->setBlocksDirty(pos, pos);
    }
    if (!occupied && powered)
    {
        TypedBoxed<bool> poweredValue(false);
        level->setBlock(pos, state->setBoxedValue(gDetectorRailPoweredProperty, &poweredValue), 3, false);
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
    return state->getValue(gDetectorRailPoweredProperty) && direction == Direction::UP ? 15 : 0;
}

void DetectorRailBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    BaseRailBlock::onPlace(level, pos, state);
    checkPressed(level, pos, state);
}

const Property* DetectorRailBlock::getShapeProperty()
{
    return gDetectorRailShapeProperty;
}

bool DetectorRailBlock::hasAnalogOutputSignal(const BlockState*) { return true; }

int DetectorRailBlock::getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos)
{
    const BlockState* railState = level->getBlockState(pos);
    if (!railState->getValue(gDetectorRailPoweredProperty))
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
    return state->getValue(gDetectorRailPoweredProperty) ? 15 : 0;
}

const BlockState* DetectorRailBlock::getBlockState(int data)
{
    TypedBoxed<BaseRailBlock::RailShape*> shapeValue(BaseRailBlock::RailShape::byData(data & 7));
    TypedBoxed<bool> poweredValue((data & 8) != 0);
    const BlockState* state = defaultBlockState()->setBoxedValue(gDetectorRailShapeProperty, &shapeValue);
    return state->setBoxedValue(gDetectorRailPoweredProperty, &poweredValue);
}

int DetectorRailBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(gDetectorRailShapeProperty)->getData();
    if (state->getValue(gDetectorRailPoweredProperty))
        data |= 8;
    return data;
}

const BlockState* DetectorRailBlock::rotate(const BlockState* state, Rotation* rotation)
{
    static const int clockwise90[10] = { 1, 0, 5, 4, 2, 3, 7, 8, 9, 6 };
    static const int clockwise180[10] = { 0, 1, 3, 2, 5, 4, 8, 9, 6, 7 };
    static const int counterClockwise90[10] = { 1, 0, 4, 5, 3, 2, 9, 6, 7, 8 };
    const int* mapping = NULL;
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise90: mapping = clockwise90; break;
    case Rotation::eTYPE_Clockwise180: mapping = clockwise180; break;
    case Rotation::eTYPE_CounterClockwise90: mapping = counterClockwise90; break;
    default: return state;
    }
    int shape = state->getValue(gDetectorRailShapeProperty)->getData();
    if (mapping[shape] == shape)
        return state;
    TypedBoxed<BaseRailBlock::RailShape*> shapeValue(BaseRailBlock::RailShape::byData(mapping[shape]));
    return state->setBoxedValue(gDetectorRailShapeProperty, &shapeValue);
}

const BlockState* DetectorRailBlock::mirror(const BlockState* state, Mirror* mirror)
{
    int shape = state->getValue(gDetectorRailShapeProperty)->getData();
    int mirrored = -1;
    if (mirror->getType() == Mirror::eTYPE_LeftRight)
    {
        static const int leftRight[6] = { 5, 4, 9, 8, 7, 6 };
        if (shape >= 4)
            mirrored = leftRight[shape - 4];
    }
    else if (mirror->getType() == Mirror::eTYPE_FrontBack)
    {
        switch (shape)
        {
        case 2: mirrored = 3; break;
        case 3: mirrored = 2; break;
        case 6: mirrored = 7; break;
        case 7: mirrored = 6; break;
        case 8: mirrored = 9; break;
        case 9: mirrored = 8; break;
        default: break;
        }
    }
    if (mirrored < 0)
        return Block::mirror(state, mirror);
    TypedBoxed<BaseRailBlock::RailShape*> shapeValue(BaseRailBlock::RailShape::byData(mirrored));
    return state->setBoxedValue(gDetectorRailShapeProperty, &shapeValue);
}

BlockStateDefinition* DetectorRailBlock::createBlockStateDefinition()
{
    const Property* properties[2] = { gDetectorRailShapeProperty, gDetectorRailPoweredProperty };
    return new BlockStateDefinition(this, properties, 2);
}

TextureAtlasSprite* DetectorRailBlock::getTexture(const Direction*, const BlockState* state)
{
    return state->getValue(gDetectorRailPoweredProperty) ? mIconOn : mIconOff;
}

void DetectorRailBlock::registerIcons(IconRegister* iconRegister)
{
    mIconOff = iconRegister->registerIcon(L"detectorRail");
    mIconOn = iconRegister->registerIcon(L"detectorRail_on");
}
