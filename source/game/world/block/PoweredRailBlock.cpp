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
    BaseRailBlock::RailShape* directionShape = shape;
    bool checkBelow = true;
    switch (shape->getData())
    {
    case 0: next = forward ? pos.south() : pos.north(); break;
    case 1: next = forward ? pos.west() : pos.east(); break;
    case 2:
        directionShape = gPoweredRailShapes[1];
        if (forward)
            next = pos.west();
        else
        {
            next = pos.east().above();
            checkBelow = false;
        }
        break;
    case 3:
        directionShape = gPoweredRailShapes[1];
        if (forward)
        {
            next = pos.west().above();
            checkBelow = false;
        }
        else
            next = pos.east();
        break;
    case 4:
        directionShape = gPoweredRailShapes[0];
        if (forward)
            next = pos.south();
        else
        {
            next = pos.north().above();
            checkBelow = false;
        }
        break;
    case 5:
        directionShape = gPoweredRailShapes[0];
        if (forward)
        {
            next = pos.south().above();
            checkBelow = false;
        }
        else
            next = pos.north();
        break;
    case 6:
    case 7:
    case 8:
    case 9:
        if (!isSameRailWithPower(level, next, forward, distance, directionShape))
        {
            next = pos.east().below();
            directionShape = gPoweredRailShapes[1];
            return isSameRailWithPower(level, next, forward, distance, directionShape);
        }
        return true;
    default:
        break;
    }

    if (isSameRailWithPower(level, next, forward, distance, directionShape))
        return true;
    return checkBelow &&
        isSameRailWithPower(level, next.below(), forward, distance, directionShape);
}

bool PoweredRailBlock::isSameRailWithPower(Level* level, const BlockPos& pos, bool forward,
                                           int distance, BaseRailBlock::RailShape* shape)
{
    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() != this)
        return false;
    BaseRailBlock::RailShape* otherShape = state->getValue(gPoweredRailShapeProperty);
    if (shape == gPoweredRailShapes[1] &&
        (otherShape == gPoweredRailShapes[0] || otherShape == gPoweredRailShapes[4] ||
         otherShape == gPoweredRailShapes[5]))
        return false;
    if (shape == gPoweredRailShapes[0] &&
        (otherShape == gPoweredRailShapes[1] || otherShape == gPoweredRailShapes[2] ||
         otherShape == gPoweredRailShapes[3]))
        return false;
    if (state->getValue(gPoweredRailProperty))
        return true;
    if (level->hasNeighborSignal(pos))
        return true;
    return findPoweredRailSignal(level, pos, state, forward, distance + 1);
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
    Rotation::eTYPE type = rotation->getType();
    if (type == Rotation::eTYPE_None)
        return state;
    int data = state->getValue(gPoweredRailShapeProperty)->getData();
    if (type == Rotation::eTYPE_Clockwise180)
    {
        switch (data)
        {
        case 2: data = 3; break;
        case 3: data = 2; break;
        case 4: data = 5; break;
        case 5: data = 4; break;
        case 6: data = 8; break;
        case 7: data = 9; break;
        case 8: data = 6; break;
        case 9: data = 7; break;
        default: data = (data == 0) ? 1 : 0; break;
        }
    }
    else
    {
        switch (data)
        {
        case 0: data = 1; break;
        case 1: data = 0; break;
        case 2: data = 4; break;
        case 3: data = 5; break;
        case 4: data = 3; break;
        case 5: data = 2; break;
        case 6: data = 9; break;
        case 7: data = 6; break;
        case 8: data = 7; break;
        case 9: data = 8; break;
        default: break;
        }
    }
    TypedBoxed<BaseRailBlock::RailShape*> shapeValue(gPoweredRailShapes[data]);
    return state->setBoxedValue(gPoweredRailShapeProperty, &shapeValue);
}

const BlockState* PoweredRailBlock::mirror(const BlockState* state, Mirror* mirror)
{
    int data = state->getValue(gPoweredRailShapeProperty)->getData();
    if (mirror->getType() == Mirror::eTYPE_LeftRight)
    {
        switch (data)
        {
        case 4: data = 5; break;
        case 5: data = 4; break;
        case 6: data = 9; break;
        case 7: data = 8; break;
        case 8: data = 7; break;
        case 9: data = 6; break;
        default: return Block::mirror(state, mirror);
        }
    }
    else if (mirror->getType() == Mirror::eTYPE_FrontBack)
    {
        switch (data)
        {
        case 2: data = 3; break;
        case 3: data = 2; break;
        case 6: data = 7; break;
        case 7: data = 6; break;
        case 8: data = 9; break;
        case 9: data = 8; break;
        default: return Block::mirror(state, mirror);
        }
    }
    else
    {
        return Block::mirror(state, mirror);
    }
    TypedBoxed<BaseRailBlock::RailShape*> shapeValue(gPoweredRailShapes[data]);
    return state->setBoxedValue(gPoweredRailShapeProperty, &shapeValue);
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
