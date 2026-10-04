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
    std::wstring name(L"shape");
    Predicate<BaseRailBlock::RailShape*>* filter = Predicates<BaseRailBlock::RailShape*>::alwaysTrue;
    EnumProperty<BaseRailBlock::RailShape*>* shape;
    {
        boost::unordered_set<BaseRailBlock::RailShape*> values;
        for (int i = 0; i < 10; ++i)
            values.insert(RailShape::VALUES[i]);
        shape = EnumProperty<BaseRailBlock::RailShape*>::create(
            name, typeid(BaseRailBlock::RailShape), values, filter);
    }
    RailBlock::SHAPE = shape;
}

RailBlock::RailBlock()
    : BaseRailBlock(false)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(RailBlock::SHAPE, RailShape::NORTH_SOUTH));
}

void RailBlock::updateState(const BlockState* state, Level* level, const BlockPos& pos, Block* block)
{
    if (block->defaultBlockState()->isSignalSource())
    {
        if (Rail(level, pos, state).countPotentialConnections() == 3)
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
    return defaultBlockState()->setValue(RailBlock::SHAPE, BaseRailBlock::RailShape::byData(data));
}

int RailBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(RailBlock::SHAPE)->getData();
}

unsigned int RailBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    unsigned int tooltip = Block::GetInteractTooltip(tooltipData);
    if (tooltipData.m_bCanPlace
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
    BaseRailBlock::RailShape* shape = state->getValue(RailBlock::SHAPE);
    switch (mirror->getType())
    {
    case Mirror::eTYPE_LeftRight:
        switch (shape->getData())
        {
        case 4:
            return state->setValue(RailBlock::SHAPE, RailShape::ASCENDING_SOUTH);
        case 5:
            return state->setValue(RailBlock::SHAPE, RailShape::ASCENDING_NORTH);
        case 6:
            return state->setValue(RailBlock::SHAPE, RailShape::NORTH_EAST);
        case 7:
            return state->setValue(RailBlock::SHAPE, RailShape::NORTH_WEST);
        case 8:
            return state->setValue(RailBlock::SHAPE, RailShape::SOUTH_WEST);
        case 9:
            return state->setValue(RailBlock::SHAPE, RailShape::SOUTH_EAST);
        }
        break;
    case Mirror::eTYPE_FrontBack:
        switch (shape->getData())
        {
        case 2:
            return state->setValue(RailBlock::SHAPE, RailShape::ASCENDING_WEST);
        case 3:
            return state->setValue(RailBlock::SHAPE, RailShape::ASCENDING_EAST);
        case 6:
            return state->setValue(RailBlock::SHAPE, RailShape::SOUTH_WEST);
        case 7:
            return state->setValue(RailBlock::SHAPE, RailShape::SOUTH_EAST);
        case 8:
            return state->setValue(RailBlock::SHAPE, RailShape::NORTH_EAST);
        case 9:
            return state->setValue(RailBlock::SHAPE, RailShape::NORTH_WEST);
        }
        break;
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
    if (state->getValue(RailBlock::SHAPE)->getData() < 6)
    {
        return mIcon;
    }

    return mTurnedTexture;
}

void RailBlock::registerIcons(IconRegister* iconRegister)
{
    Block::registerIcons(iconRegister);
    mTurnedTexture = iconRegister->registerIcon(getIconName() + L"_turned");
}
