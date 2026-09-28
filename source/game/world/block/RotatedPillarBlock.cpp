#include "game/world/block/RotatedPillarBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/Boxed.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<Direction::Axis*>* gRotatedPillarBlockAxisProperty = NULL;

void RotatedPillarBlock::blockStaticCtor()
{
    boost::unordered_set<Direction::Axis*> values;
    values.insert(Direction::Axis::X);
    values.insert(Direction::Axis::Y);
    values.insert(Direction::Axis::Z);
    gRotatedPillarBlockAxisProperty = EnumProperty<Direction::Axis*>::create(
        L"axis", typeid(Direction::Axis*), values, Predicates<Direction::Axis*>::alwaysTrue);
}

RotatedPillarBlock::RotatedPillarBlock(Material* material)
    : Block(material, material->getColor())
{
}

RotatedPillarBlock::RotatedPillarBlock(Material* material, const MaterialColor* materialColor)
    : Block(material, materialColor)
{
}

const BlockState* RotatedPillarBlock::rotate(const BlockState* state, Rotation* rotation)
{
    if (rotation->getType() == Rotation::eTYPE_Clockwise90 ||
        rotation->getType() == Rotation::eTYPE_CounterClockwise90)
    {
        int axis = state->getValue(gRotatedPillarBlockAxisProperty)->getId();
        if (axis == Direction::Axis::eAxis_X)
        {
            TypedBoxed<Direction::Axis*> value(Direction::Axis::Z);
            return state->setBoxedValue(gRotatedPillarBlockAxisProperty, &value);
        }
        if (axis == Direction::Axis::eAxis_Z)
        {
            TypedBoxed<Direction::Axis*> value(Direction::Axis::X);
            return state->setBoxedValue(gRotatedPillarBlockAxisProperty, &value);
        }
    }

    return state;
}

const BlockState* RotatedPillarBlock::getBlockState(int data)
{
    Direction::Axis* axis = Direction::Axis::Y;
    if ((data & 0xC) == 4)
    {
        axis = Direction::Axis::X;
    }
    else if ((data & 0xC) == 8)
    {
        axis = Direction::Axis::Z;
    }

    TypedBoxed<Direction::Axis*> value(axis);
    return defaultBlockState()->setBoxedValue(gRotatedPillarBlockAxisProperty, &value);
}

int RotatedPillarBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    Direction::Axis* axis = state->getValue(gRotatedPillarBlockAxisProperty);
    if (axis == Direction::Axis::X)
    {
        return 4;
    }
    if (axis == Direction::Axis::Z)
    {
        return 8;
    }
    return 0;
}

BlockStateDefinition* RotatedPillarBlock::createBlockStateDefinition()
{
    static const Property* properties[] = { gRotatedPillarBlockAxisProperty };
    return new BlockStateDefinition(this, properties, 1);
}

not_null_ptr<ItemInstance> RotatedPillarBlock::getSilkTouchItemInstance(const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this)));
}

const BlockState* RotatedPillarBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction, float hitX, float hitY, float hitZ, int auxValue, boost::shared_ptr<LivingEntity> placer)
{
    const BlockState* state = Block::getStateForPlacement(level, pos, direction, hitX, hitY, hitZ, auxValue, placer);
    TypedBoxed<Direction::Axis*> value(direction->getAxis());
    return state->setBoxedValue(gRotatedPillarBlockAxisProperty, &value);
}

TextureAtlasSprite* RotatedPillarBlock::getTexture(const Direction* direction, const BlockState* state)
{
    Direction::Axis* axis = getAxis(state);
    if (axis == Direction::Axis::Y
        && (direction == Direction::UP || direction == Direction::DOWN))
    {
        return getTopTexture(state);
    }
    if (axis == Direction::Axis::X
        && (direction == Direction::EAST || direction == Direction::WEST))
    {
        return getTopTexture(state);
    }
    if (axis == Direction::Axis::Z
        && (direction == Direction::NORTH || direction == Direction::SOUTH))
    {
        return getTopTexture(state);
    }

    return getTypeTexture(state);
}

TextureAtlasSprite* RotatedPillarBlock::getTypeTexture(const BlockState*)
{
    return mIcon;
}

TextureAtlasSprite* RotatedPillarBlock::getTopTexture(const BlockState*)
{
    return mTopTexture;
}

EBlockRenderShape RotatedPillarBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_TREE;
}
