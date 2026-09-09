#include "game/world/block/RailBlock.hpp"

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

static EnumProperty<BaseRailBlock::RailShape*>* gRailShapeProperty = NULL;

void RailBlock::blockStaticCtor()
{
    boost::unordered_set<BaseRailBlock::RailShape*> values;
    values.insert(gRailShapes[0]);
    values.insert(gRailShapes[1]);
    values.insert(gRailShapes[2]);
    values.insert(gRailShapes[3]);
    values.insert(gRailShapes[4]);
    values.insert(gRailShapes[5]);
    values.insert(gRailShapes[6]);
    values.insert(gRailShapes[7]);
    values.insert(gRailShapes[8]);
    values.insert(gRailShapes[9]);

    gRailShapeProperty = EnumProperty<BaseRailBlock::RailShape*>::create(
        L"shape", typeid(BaseRailBlock::RailShape*), values,
        Predicates<BaseRailBlock::RailShape*>::alwaysTrue);
}

RailBlock::RailBlock()
    : BaseRailBlock(false)
{
    DerivedInit();
    TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[0]);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(gRailShapeProperty, &shape));
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

const Property* RailBlock::getShapeProperty()
{
    return gRailShapeProperty;
}

const BlockState* RailBlock::getBlockState(int data)
{
    TypedBoxed<BaseRailBlock::RailShape*> shape(BaseRailBlock::RailShape::byData(data));
    return defaultBlockState()->setBoxedValue(gRailShapeProperty, &shape);
}

int RailBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(gRailShapeProperty)->getData();
}

unsigned int RailBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    unsigned int tooltip = Block::GetInteractTooltip(tooltipData);
    if (tooltipData.m_bIsInside
        && tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success)
    {
        return 0x4C36CD96U;
    }

    return tooltip;
}

const BlockState* RailBlock::rotate(const BlockState* state, Rotation* rotation)
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise180:
        switch (state->getValue(gRailShapeProperty)->getData())
        {
        case 2:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[3]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 3:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[2]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 4:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[5]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 5:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[4]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[8]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[9]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[6]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[7]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        }
    case Rotation::eTYPE_CounterClockwise90:
        switch (state->getValue(gRailShapeProperty)->getData())
        {
        case 0:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[1]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 1:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[0]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 2:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[4]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 3:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[5]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 4:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[3]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 5:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[2]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[9]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[6]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[7]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[8]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        }
    case Rotation::eTYPE_Clockwise90:
        switch (state->getValue(gRailShapeProperty)->getData())
        {
        case 0:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[1]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 1:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[0]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 2:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[5]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 3:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[4]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 4:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[2]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 5:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[3]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[7]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[8]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[9]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[6]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
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
        switch (state->getValue(gRailShapeProperty)->getData())
        {
        case 4:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[5]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 5:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[4]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[9]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[8]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[7]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[6]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        }
    }
    else if (mirror->getType() == Mirror::eTYPE_FrontBack)
    {
        switch (state->getValue(gRailShapeProperty)->getData())
        {
        case 2:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[3]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 3:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[2]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[7]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[6]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[9]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(gRailShapes[8]);
            return state->setBoxedValue(gRailShapeProperty, &shape);
        }
        }
    }

    return Block::mirror(state, mirror);
}

BlockStateDefinition* RailBlock::createBlockStateDefinition()
{
    static const Property* properties[] = { gRailShapeProperty };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* RailBlock::getTexture(const Direction*, const BlockState* state)
{
    if (state->getValue(gRailShapeProperty)->getData() > 5)
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
