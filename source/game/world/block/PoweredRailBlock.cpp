#include "game/world/block/PoweredRailBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"

static BaseRailBlock::RailShape* gPoweredRailShapes[10] =
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
static EnumProperty<BaseRailBlock::RailShape*>* gPoweredRailShapeProperty = NULL;
static BooleanProperty* gPoweredRailProperty = NULL;

class PoweredRailShapePredicate : public Predicate<BaseRailBlock::RailShape*>
{
public:
    virtual bool apply(BaseRailBlock::RailShape*) const { return true; }
};

static PoweredRailShapePredicate gPoweredRailShapePredicate;

static const BlockState* poweredRailState(const BlockState* state, const Property* property,
                                          BaseRailBlock::RailShape* shape, bool powered)
{
    TypedBoxed<BaseRailBlock::RailShape*> shapeValue(shape);
    const BlockState* result = state->setBoxedValue(property, &shapeValue);
    TypedBoxed<bool> poweredValue(powered);
    return result->setBoxedValue(gPoweredRailProperty, &poweredValue);
}

void PoweredRailBlock::blockStaticCtor()
{
    static const wchar_t* names[10] =
        { L"north_south", L"east_west", L"ascending_east", L"ascending_west",
          L"ascending_north", L"ascending_south", L"south_east", L"south_west",
          L"north_west", L"north_east" };
    boost::unordered_set<BaseRailBlock::RailShape*> values;
    for (int i = 0; i < 10; ++i)
    {
        gPoweredRailShapes[i] = new BaseRailBlock::RailShape(i, names[i]);
        values.insert(gPoweredRailShapes[i]);
    }
    gPoweredRailShapeProperty = EnumProperty<BaseRailBlock::RailShape*>::create(
        L"shape", typeid(BaseRailBlock::RailShape*), values, &gPoweredRailShapePredicate);
    gPoweredRailProperty = static_cast<BooleanProperty*>(BooleanProperty::create(L"powered"));
}

PoweredRailBlock::PoweredRailBlock()
    : BaseRailBlock(true), mPoweredTexture(NULL)
{
    DerivedInit();
    TypedBoxed<BaseRailBlock::RailShape*> shapeValue(gPoweredRailShapes[0]);
    TypedBoxed<bool> poweredValue(false);
    const BlockState* state = defaultBlockState()->setBoxedValue(gPoweredRailShapeProperty, &shapeValue);
    registerDefaultState(state->setBoxedValue(gPoweredRailProperty, &poweredValue));
}

PoweredRailBlock::~PoweredRailBlock()
{
}

bool PoweredRailBlock::findPoweredRailSignal(Level* level, const BlockPos& pos,
                                              const BlockState* state, bool forward, int distance)
{
    if (distance > 7)
        return false;

    BaseRailBlock::RailShape* shape = state->getValue(gPoweredRailShapeProperty);
    BlockPos next = pos;
    bool checkSide = true;
    switch (shape->getData())
    {
    case 0: next = forward ? pos.south() : pos.north(); break;
    case 1: next = forward ? pos.east() : pos.west(); break;
    case 2: next = forward ? pos.east().above() : pos.west(); break;
    case 3: next = forward ? pos.east() : pos.west().above(); break;
    case 4:
        if (forward) next = pos.south();
        else { next = pos.north().above(); checkSide = false; }
        break;
    case 5:
        if (forward) { next = pos.south().above(); checkSide = false; }
        else next = pos.north();
        break;
    default: next = forward ? pos.south() : pos.north(); break;
    }

    if (isSameRailWithPower(level, next, forward, distance + 1,
                            forward ? shape : shape))
        return true;
    if (checkSide)
    {
        BlockPos side = next.west();
        if (isSameRailWithPower(level, side, forward, distance + 1, shape))
            return true;
    }
    return false;
}

bool PoweredRailBlock::isSameRailWithPower(Level* level, const BlockPos& pos, bool forward,
                                           int distance, BaseRailBlock::RailShape* shape)
{
    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() != this)
        return false;
    BaseRailBlock::RailShape* otherShape = state->getValue(gPoweredRailShapeProperty);
    if (shape != NULL && otherShape != shape)
    {
        int data = otherShape->getData();
        if (shape == gPoweredRailShapes[0] && data != 0 && data != 2 && data != 3 && data != 4 && data != 5)
            return false;
        if (shape == gPoweredRailShapes[1] && data != 1 && data != 2 && data != 3 && data != 6 && data != 7 && data != 8 && data != 9)
            return false;
    }
    if (state->getValue(gPoweredRailProperty))
        return true;
    if (level->hasNeighborSignal(pos))
        return true;
    return findPoweredRailSignal(level, pos, state, forward, distance);
}

void PoweredRailBlock::updateState(const BlockState* state, Level* level, const BlockPos& pos, Block*)
{
    bool powered = level->hasNeighborSignal(pos) ||
        isSameRailWithPower(level, pos, false, 0, NULL) ||
        isSameRailWithPower(level, pos, true, 0, NULL);
    if (powered != state->getValue(gPoweredRailProperty))
    {
        TypedBoxed<bool> poweredValue(powered);
        const BlockState* updated = state->setBoxedValue(gPoweredRailProperty, &poweredValue);
        level->setBlock(pos, updated, 3, false);
        level->updateNeighborsAt(pos.below(), this, false);
        if (state->getValue(gPoweredRailShapeProperty)->isAscending())
            level->updateNeighborsAt(pos.above(), this, false);
    }
}

const Property* PoweredRailBlock::getShapeProperty()
{
    return gPoweredRailShapeProperty;
}

const BlockState* PoweredRailBlock::getBlockState(int data)
{
    return poweredRailState(defaultBlockState(), gPoweredRailShapeProperty,
                            BaseRailBlock::RailShape::byData(data & 7), (data & 8) != 0);
}

int PoweredRailBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(gPoweredRailShapeProperty)->getData();
    if (state->getValue(gPoweredRailProperty))
        data |= 8;
    return data;
}

const BlockState* PoweredRailBlock::rotate(const BlockState* state, Rotation* rotation)
{
    int shape = convertBlockStateToLegacyData(state);
    int data = shape & 7;
    static const int clockwise90[10] = { 1, 0, 3, 2, 5, 4, 8, 9, 7, 6 };
    static const int clockwise180[10] = { 0, 1, 5, 4, 3, 2, 9, 8, 6, 7 };
    static const int counterClockwise90[10] = { 1, 0, 4, 5, 2, 3, 9, 8, 6, 7 };
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise90: data = clockwise90[data]; break;
    case Rotation::eTYPE_Clockwise180: data = clockwise180[data]; break;
    case Rotation::eTYPE_CounterClockwise90: data = counterClockwise90[data]; break;
    default: break;
    }
    return poweredRailState(state, gPoweredRailShapeProperty, gPoweredRailShapes[data], (shape & 8) != 0);
}

const BlockState* PoweredRailBlock::mirror(const BlockState* state, Mirror* mirror)
{
    int shape = convertBlockStateToLegacyData(state);
    int data = shape & 7;
    static const int leftRight[10] = { 0, 1, 4, 5, 2, 3, 6, 9, 8, 7 };
    static const int frontBack[10] = { 0, 1, 5, 4, 3, 2, 9, 8, 7, 6 };
    if (mirror->getType() == Mirror::eTYPE_LeftRight)
        data = leftRight[data];
    else if (mirror->getType() == Mirror::eTYPE_FrontBack)
        data = frontBack[data];
    return poweredRailState(state, gPoweredRailShapeProperty, gPoweredRailShapes[data], (shape & 8) != 0);
}

BlockStateDefinition* PoweredRailBlock::createBlockStateDefinition()
{
    const Property* properties[2] = { gPoweredRailShapeProperty, gPoweredRailProperty };
    return new BlockStateDefinition(this, properties, 2);
}

TextureAtlasSprite* PoweredRailBlock::getTexture(const Direction*, const BlockState* state)
{
    return state->getValue(gPoweredRailProperty) ? mPoweredTexture : mIcon;
}

void PoweredRailBlock::registerIcons(IconRegister* iconRegister)
{
    Block::registerIcons(iconRegister);
    mPoweredTexture = iconRegister->registerIcon(getIconName() + L"_powered");
}
