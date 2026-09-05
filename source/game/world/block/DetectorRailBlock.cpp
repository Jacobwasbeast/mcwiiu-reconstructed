#include "game/world/block/DetectorRailBlock.hpp"

#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/world/block/TypedBoxed.hpp"

static EnumProperty<BaseRailBlock::RailShape*>* gDetectorRailShapeProperty = NULL;
static Property* gDetectorRailPoweredProperty = NULL;

class DetectorRailShapePredicate : public Predicate<BaseRailBlock::RailShape*>
{
public:
    virtual bool apply(BaseRailBlock::RailShape*) const { return true; }
};

static DetectorRailShapePredicate gDetectorRailShapePredicate;

static const BlockState* detectorRailState(const BlockState* state, const Property* shapeProperty,
                                           const Property* poweredProperty,
                                           BaseRailBlock::RailShape* shape, bool powered)
{
    TypedBoxed<BaseRailBlock::RailShape*> shapeValue(shape);
    const BlockState* result = state->setBoxedValue(shapeProperty, &shapeValue);
    TypedBoxed<bool> poweredValue(powered);
    return result->setBoxedValue(poweredProperty, &poweredValue);
}

void DetectorRailBlock::blockStaticCtor()
{
    static const wchar_t* names[10] =
        { L"north_south", L"east_west", L"ascending_east", L"ascending_west",
          L"ascending_north", L"ascending_south", L"south_east", L"south_west",
          L"north_west", L"north_east" };
    boost::unordered_set<BaseRailBlock::RailShape*> values;
    for (int i = 0; i < 10; ++i)
        values.insert(new BaseRailBlock::RailShape(i, names[i]));
    gDetectorRailShapeProperty = EnumProperty<BaseRailBlock::RailShape*>::create(
        L"shape", typeid(BaseRailBlock::RailShape*), values, &gDetectorRailShapePredicate);
    gDetectorRailPoweredProperty = BooleanProperty::create(L"powered");
}

DetectorRailBlock::DetectorRailBlock()
    : BaseRailBlock(true), mIconOff(NULL), mIconOn(NULL)
{
    DerivedInit();
    setTicking(true);
    const BlockState* state = defaultBlockState();
    state = detectorRailState(state, gDetectorRailShapeProperty, gDetectorRailPoweredProperty,
                              BaseRailBlock::RailShape::byData(0), false);
    registerDefaultState(state);
}

DetectorRailBlock::~DetectorRailBlock()
{
}

int DetectorRailBlock::getTickDelay(Level*) { return 20; }

bool DetectorRailBlock::isSignalSource(const BlockState*) { return true; }

void DetectorRailBlock::entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                                     const boost::shared_ptr<Entity>& entity)
{
    if (!level->mIsClientSide && entity && entity->isPassenger())
        checkPressed(level, pos, state);
}

void DetectorRailBlock::randomTick(Level*, const BlockPos&, const BlockState*, Random*)
{
}

void DetectorRailBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!level->mIsClientSide && state->getValue(static_cast<const AbstractProperty<bool>*>(gDetectorRailPoweredProperty)))
        checkPressed(level, pos, state);
}

int DetectorRailBlock::getSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction*)
{
    return state->getValue(static_cast<const AbstractProperty<bool>*>(gDetectorRailPoweredProperty)) ? 15 : 0;
}

AABB* DetectorRailBlock::getSearchBB(const BlockPos& pos)
{
    return AABB::newTemp(pos.x + 0.2, pos.y, pos.z + 0.2,
                         pos.x + 0.8, pos.y + 0.8, pos.z + 0.8);
}

std::vector<boost::shared_ptr<Entity> >* DetectorRailBlock::getInteractingMinecartOfType(
    Level* level, const BlockPos& pos, const std::type_info& type,
    const Predicate<boost::shared_ptr<Entity> >* predicate)
{
    return level->getEntitiesOfClass(type, getSearchBB(pos), predicate, false);
}

void DetectorRailBlock::checkPressed(Level* level, const BlockPos& pos, const BlockState* state)
{
    std::vector<boost::shared_ptr<Entity> >* entities =
        getInteractingMinecartOfType(level, pos, typeid(Entity), NULL);
    bool powered = entities != NULL && !entities->empty();
    bool oldPowered = state->getValue(static_cast<const AbstractProperty<bool>*>(gDetectorRailPoweredProperty));
    if (powered != oldPowered)
    {
        const BlockState* next = detectorRailState(state, gDetectorRailShapeProperty,
                                                    gDetectorRailPoweredProperty,
                                                    state->getValue(gDetectorRailShapeProperty), powered);
        level->setBlock(pos, next, 3, false);
        level->updateNeighborsAt(pos, this, false);
        level->updateNeighborsAt(pos.below(), this, false);
        level->setBlocksDirty(pos, pos);
    }
    delete entities;
}

void DetectorRailBlock::updatePowerToConnected(Level* level, const BlockPos& pos,
                                               const BlockState* state, bool powered)
{
    BaseRailBlock::Rail rail(level, pos, state);
    std::vector<BlockPos>* connections = rail.getConnections();
    for (std::vector<BlockPos>::const_iterator it = connections->begin(); it != connections->end(); ++it)
    {
        BlockState* neighbor = const_cast<BlockState*>(level->getBlockState(*it));
        if (neighbor != NULL)
            neighbor->getBlock()->getDirectSignal(neighbor, level, *it, NULL);
    }
}

int DetectorRailBlock::getDirectSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction* direction)
{
    return state->getValue(static_cast<const AbstractProperty<bool>*>(gDetectorRailPoweredProperty)) &&
           direction == Direction::DOWN ? 15 : 0;
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

int DetectorRailBlock::getAnalogOutputSignal(const BlockState* state, Level*, const BlockPos&)
{
    return state->getValue(static_cast<const AbstractProperty<bool>*>(gDetectorRailPoweredProperty)) ? 15 : 0;
}

const BlockState* DetectorRailBlock::getBlockState(int data)
{
    return detectorRailState(defaultBlockState(), gDetectorRailShapeProperty, gDetectorRailPoweredProperty,
                             BaseRailBlock::RailShape::byData(data & 7), (data & 8) != 0);
}

int DetectorRailBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(gDetectorRailShapeProperty)->getData();
    if (state->getValue(static_cast<const AbstractProperty<bool>*>(gDetectorRailPoweredProperty)))
        data |= 8;
    return data;
}

const BlockState* DetectorRailBlock::rotate(const BlockState* state, Rotation* rotation)
{
    static const int clockwise90[10] = { 1, 0, 3, 2, 5, 4, 8, 9, 7, 6 };
    static const int clockwise180[10] = { 0, 1, 5, 4, 3, 2, 9, 8, 6, 7 };
    static const int counterClockwise90[10] = { 1, 0, 4, 5, 2, 3, 9, 8, 6, 7 };
    int data = convertBlockStateToLegacyData(state);
    int shape = data & 7;
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise90: shape = clockwise90[shape]; break;
    case Rotation::eTYPE_Clockwise180: shape = clockwise180[shape]; break;
    case Rotation::eTYPE_CounterClockwise90: shape = counterClockwise90[shape]; break;
    default: break;
    }
    return detectorRailState(state, gDetectorRailShapeProperty, gDetectorRailPoweredProperty,
                             BaseRailBlock::RailShape::byData(shape), (data & 8) != 0);
}

const BlockState* DetectorRailBlock::mirror(const BlockState* state, Mirror* mirror)
{
    static const int leftRight[10] = { 0, 1, 4, 5, 2, 3, 6, 9, 8, 7 };
    static const int frontBack[10] = { 0, 1, 5, 4, 3, 2, 9, 8, 7, 6 };
    int data = convertBlockStateToLegacyData(state);
    int shape = data & 7;
    if (mirror->getType() == Mirror::eTYPE_LeftRight) shape = leftRight[shape];
    else if (mirror->getType() == Mirror::eTYPE_FrontBack) shape = frontBack[shape];
    return detectorRailState(state, gDetectorRailShapeProperty, gDetectorRailPoweredProperty,
                             BaseRailBlock::RailShape::byData(shape), (data & 8) != 0);
}

BlockStateDefinition* DetectorRailBlock::createBlockStateDefinition()
{
    const Property* properties[2] = { gDetectorRailShapeProperty, gDetectorRailPoweredProperty };
    return new BlockStateDefinition(this, properties, 2);
}

TextureAtlasSprite* DetectorRailBlock::getTexture(const Direction*, const BlockState* state)
{
    return state->getValue(static_cast<const AbstractProperty<bool>*>(gDetectorRailPoweredProperty)) ? mIconOn : mIconOff;
}

void DetectorRailBlock::registerIcons(IconRegister* iconRegister)
{
    mIconOff = iconRegister->registerIcon(L"detectorRail");
    mIconOn = iconRegister->registerIcon(L"detectorRail_on");
}
