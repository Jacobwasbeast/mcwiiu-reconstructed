#include "game/world/block/RailBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"

static BaseRailBlock::RailShape* gRailShapes[10] =
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
static EnumProperty<BaseRailBlock::RailShape*>* gRailShapeProperty = NULL;

class RailShapePredicate : public Predicate<BaseRailBlock::RailShape*>
{
public:
    virtual bool apply(BaseRailBlock::RailShape*) const { return true; }
};

static RailShapePredicate gRailShapePredicate;

static const BlockState* railState(const BlockState* state, const Property* property,
                                   BaseRailBlock::RailShape* shape)
{
    TypedBoxed<BaseRailBlock::RailShape*> value(shape);
    return state->setBoxedValue(property, &value);
}

void RailBlock::blockStaticCtor()
{
    static const wchar_t* names[10] =
        { L"north_south", L"east_west", L"ascending_east", L"ascending_west",
          L"ascending_north", L"ascending_south", L"south_east", L"south_west",
          L"north_west", L"north_east" };
    boost::unordered_set<BaseRailBlock::RailShape*> values;
    for (int i = 0; i < 10; ++i)
    {
        gRailShapes[i] = new BaseRailBlock::RailShape(i, names[i]);
        values.insert(gRailShapes[i]);
    }
    gRailShapeProperty = EnumProperty<BaseRailBlock::RailShape*>::create(
        L"shape", typeid(BaseRailBlock::RailShape*), values, &gRailShapePredicate);
}

RailBlock::RailBlock()
    : BaseRailBlock(false), mTurnedTexture(NULL)
{
    DerivedInit();
    TypedBoxed<BaseRailBlock::RailShape*> value(gRailShapes[0]);
    registerDefaultState(defaultBlockState()->setBoxedValue(gRailShapeProperty, &value));
}

RailBlock::~RailBlock()
{
}

void RailBlock::updateState(const BlockState* state, Level* level, const BlockPos& pos, Block* block)
{
    if (defaultBlockState()->isSignalSource())
    {
        BaseRailBlock::Rail rail(level, pos, state);
        if (rail.countPotentialConnections() == 3)
            level->setBlock(pos, state, 0, false);
    }
}

const Property* RailBlock::getShapeProperty()
{
    return gRailShapeProperty;
}

const BlockState* RailBlock::getBlockState(int data)
{
    return railState(defaultBlockState(), gRailShapeProperty,
                     BaseRailBlock::RailShape::byData(data));
}

int RailBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(gRailShapeProperty)->getData();
}

unsigned int RailBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    unsigned int result = Block::GetInteractTooltip(tooltipData);
    if (tooltipData.m_bIsInside && tooltipData.m_pBlockState == NULL)
        return 0x4c36cd96;
    return result;
}

const BlockState* RailBlock::rotate(const BlockState* state, Rotation* rotation)
{
    int shape = convertBlockStateToLegacyData(state);
    int rotated = shape;
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise90:
        { static const int map[10] = { 1, 0, 3, 2, 5, 4, 8, 9, 7, 6 }; rotated = map[shape]; }
        break;
    case Rotation::eTYPE_Clockwise180:
        { static const int map[10] = { 0, 1, 5, 4, 3, 2, 9, 8, 6, 7 }; rotated = map[shape]; }
        break;
    case Rotation::eTYPE_CounterClockwise90:
        { static const int map[10] = { 1, 0, 4, 5, 2, 3, 9, 8, 6, 7 }; rotated = map[shape]; }
        break;
    default:
        break;
    }
    return railState(state, gRailShapeProperty, gRailShapes[rotated]);
}

const BlockState* RailBlock::mirror(const BlockState* state, Mirror* mirror)
{
    int shape = convertBlockStateToLegacyData(state);
    int mirrored = shape;
    if (mirror->getType() == Mirror::eTYPE_LeftRight)
    {
        static const int map[10] = { 0, 1, 4, 5, 2, 3, 6, 9, 8, 7 };
        mirrored = map[shape];
    }
    else if (mirror->getType() == Mirror::eTYPE_FrontBack)
    {
        static const int map[10] = { 0, 1, 5, 4, 3, 2, 9, 8, 7, 6 };
        mirrored = map[shape];
    }
    return railState(state, gRailShapeProperty, gRailShapes[mirrored]);
}

BlockStateDefinition* RailBlock::createBlockStateDefinition()
{
    const Property* properties[1] = { gRailShapeProperty };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* RailBlock::getTexture(const Direction*, const BlockState* state)
{
    return convertBlockStateToLegacyData(state) > 5 ? mTurnedTexture : mIcon;
}

void RailBlock::registerIcons(IconRegister* iconRegister)
{
    Block::registerIcons(iconRegister);
    mTurnedTexture = iconRegister->registerIcon(getIconName() + L"_turned");
}
