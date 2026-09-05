#include "game/world/block/GlazedTerracottaBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/world/block/TypedBoxed.hpp"

GlazedTerracottaBlock::GlazedTerracottaBlock(const DyeColor* dyeColor)
    : HorizontalDirectionalBlock(Material::CLAY, MaterialColor::getDyeMaterial(dyeColor))
{
    DerivedInit();
    setDestroyTime(0.075f);
    setSoundType(SoundType::STONE);
}

GlazedTerracottaBlock::~GlazedTerracottaBlock()
{
}

const BlockState* GlazedTerracottaBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(HorizontalDirectionalBlock::FACING)));
    return state->setBoxedValue(HorizontalDirectionalBlock::FACING, &facing);
}

const BlockState* GlazedTerracottaBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(HorizontalDirectionalBlock::FACING)));
}

const BlockState* GlazedTerracottaBlock::getStateForPlacement(Level*, const BlockPos&, const Direction*,
                                                               float, float, float, int,
                                                               boost::shared_ptr<LivingEntity> placer)
{
    const BlockState* state = defaultBlockState();
    const Direction* facing = placer->getDirection()->getOpposite();
    TypedBoxed<const Direction*> boxedFacing(facing);
    return state->setBoxedValue(HorizontalDirectionalBlock::FACING, &boxedFacing);
}

int GlazedTerracottaBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(HorizontalDirectionalBlock::FACING)->get2DDataValue();
}

const BlockState* GlazedTerracottaBlock::getBlockState(int data)
{
    TypedBoxed<const Direction*> facing(Direction::from2DDataValue(data));
    return defaultBlockState()->setBoxedValue(HorizontalDirectionalBlock::FACING, &facing);
}

EPistonPushReaction GlazedTerracottaBlock::getPistonPushReaction(const BlockState*)
{
    return static_cast<EPistonPushReaction>(4);
}

EBlockRenderShape GlazedTerracottaBlock::getRenderShape(const BlockState*)
{
    return static_cast<EBlockRenderShape>(47);
}

BlockStateDefinition* GlazedTerracottaBlock::createBlockStateDefinition()
{
    static const Property* properties[1] = { NULL };
    if (properties[0] == NULL)
        properties[0] = HorizontalDirectionalBlock::FACING;
    return new BlockStateDefinition(this, properties, 1);
}
