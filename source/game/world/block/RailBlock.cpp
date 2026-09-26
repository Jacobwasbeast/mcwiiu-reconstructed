#include "game/world/block/RailBlock.hpp"
#include "game/core/StringIds.hpp"

#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<BaseRailBlock::RailShape*>* RailBlock::SHAPE = NULL;

void RailBlock::blockStaticCtor()
{
    boost::unordered_set<BaseRailBlock::RailShape*> values;
    values.insert(RailShape::NORTH_SOUTH);
    values.insert(RailShape::EAST_WEST);
    values.insert(RailShape::ASCENDING_EAST);
    values.insert(RailShape::ASCENDING_WEST);
    values.insert(RailShape::ASCENDING_NORTH);
    values.insert(RailShape::ASCENDING_SOUTH);
    values.insert(RailShape::SOUTH_EAST);
    values.insert(RailShape::SOUTH_WEST);
    values.insert(RailShape::NORTH_WEST);
    values.insert(RailShape::NORTH_EAST);

    RailBlock::SHAPE = EnumProperty<BaseRailBlock::RailShape*>::create(
        L"shape", typeid(BaseRailBlock::RailShape*), values,
        Predicates<BaseRailBlock::RailShape*>::alwaysTrue);
}

RailBlock::RailBlock()
    : BaseRailBlock(false)
{
    DerivedInit();
    TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_SOUTH);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(RailBlock::SHAPE, &shape));
}

RailBlock::~RailBlock()
{
}

void RailBlock::updateState(const BlockState* state, Level* level, const BlockPos& pos, Block* block)
{
    if (block->defaultBlockState()->isSignalSource())
    {
        Rail rail(level, pos, state);
        if (rail.countPotentialConnections() == 3)
        {
            updateDir(level, pos, state, false);
        }
    }
}

const AbstractProperty<BaseRailBlock::RailShape*>* RailBlock::getShapeProperty()
{
    return RailBlock::SHAPE;
}

const BlockState* RailBlock::getBlockState(int data)
{
    TypedBoxed<BaseRailBlock::RailShape*> shape(BaseRailBlock::RailShape::byData(data));
    return defaultBlockState()->setBoxedValue(RailBlock::SHAPE, &shape);
}

int RailBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(RailBlock::SHAPE)->getData();
}

unsigned int RailBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    unsigned int tooltip = Block::GetInteractTooltip(tooltipData);
    if (tooltipData.m_bIsInside
        && tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success)
    {
        return IDS_TOOLTIPS_PLACE;
    }

    return tooltip;
}

const BlockState* RailBlock::rotate(const BlockState* state, Rotation* rotation)
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise180:
        switch (state->getValue(RailBlock::SHAPE)->getData())
        {
        case 2:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_WEST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 3:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_EAST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 4:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_SOUTH);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 5:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_NORTH);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_WEST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_EAST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_EAST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_WEST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        }
    case Rotation::eTYPE_CounterClockwise90:
        switch (state->getValue(RailBlock::SHAPE)->getData())
        {
        case 0:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::EAST_WEST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 1:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_SOUTH);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 2:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_NORTH);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 3:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_SOUTH);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 4:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_WEST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 5:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_EAST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_EAST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_EAST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_WEST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_WEST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        }
    case Rotation::eTYPE_Clockwise90:
        switch (state->getValue(RailBlock::SHAPE)->getData())
        {
        case 0:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::EAST_WEST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 1:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_SOUTH);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 2:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_SOUTH);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 3:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_NORTH);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 4:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_EAST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 5:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_WEST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_WEST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_WEST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_EAST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_EAST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        }
    default:
        break;
    }

    return state;
}

const BlockState* RailBlock::mirror(const BlockState* state, Mirror* mirror)
{
    if (mirror->getType() == Mirror::eTYPE_LeftRight)
    {
        switch (state->getValue(RailBlock::SHAPE)->getData())
        {
        case 4:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_SOUTH);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 5:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_NORTH);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_EAST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_WEST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_WEST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_EAST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        }
    }
    else if (mirror->getType() == Mirror::eTYPE_FrontBack)
    {
        switch (state->getValue(RailBlock::SHAPE)->getData())
        {
        case 2:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_WEST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 3:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_EAST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_WEST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_EAST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_EAST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_WEST);
            return state->setBoxedValue(RailBlock::SHAPE, &shape);
        }
        }
    }

    return Block::mirror(state, mirror);
}

BlockStateDefinition* RailBlock::createBlockStateDefinition()
{
    static const Property* properties[] = { RailBlock::SHAPE };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* RailBlock::getTexture(const Direction*, const BlockState* state)
{
    if (state->getValue(RailBlock::SHAPE)->getData() > 5)
    {
        return mTurnedTexture;
    }

    return mIcon;
}

void RailBlock::registerIcons(IconRegister* iconRegister)
{
    Block::registerIcons(iconRegister);
    mTurnedTexture = iconRegister->registerIcon(getIconName() + L"_turned");
}
