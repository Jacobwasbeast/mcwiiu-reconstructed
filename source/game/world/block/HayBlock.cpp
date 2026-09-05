#include "game/world/block/HayBlock.hpp"

#include "game/core/IconRegister.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

HayBlock::HayBlock()
    : RotatedPillarBlock(Material::GRASS, MaterialColor::GRASS)
{
    DerivedInit();

    BlockStateDefinition* definition = mStateDefinition;
    TypedBoxed<Direction::Axis*> value(Direction::Axis::Y);
    registerDefaultState(definition->any()->setBoxedValue(definition->getProperties()[0], &value));
}

HayBlock::~HayBlock()
{
}

void HayBlock::fallOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, float fallDistance)
{
    entity->causeFallDamage(fallDistance, 0.2f);
}

TextureAtlasSprite* HayBlock::getTypeTexture(const BlockState*)
{
    return mIcon;
}

void HayBlock::registerIcons(IconRegister* iconRegister)
{
    mTopTexture = iconRegister->registerIcon(getIconName() + L"_top");
    mIcon = iconRegister->registerIcon(getIconName() + L"_side");
}

EBlockRenderShape HayBlock::getRenderShape(const BlockState*)
{
    return static_cast<EBlockRenderShape>(31);
}

Direction::Axis* HayBlock::getAxis(const BlockState* state)
{
    const std::vector<const Property*>& properties = state->getProperties();
    return state->getValue(static_cast<const AbstractProperty<Direction::Axis*>*>(properties[0]));
}
