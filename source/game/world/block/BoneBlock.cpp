#include "game/world/block/BoneBlock.hpp"

#include "game/client/audio/SoundType.hpp"
#include "game/core/IconRegister.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

BoneBlock::BoneBlock()
    : RotatedPillarBlock(Material::STONE, MaterialColor::SNOW)
{
    DerivedInit();

    BlockStateDefinition* definition = mStateDefinition;
    TypedBoxed<Direction::Axis*> value(Direction::Axis::Y);
    registerDefaultState(definition->any()->setBoxedValue(definition->getProperties()[0], &value));
    setDestroyTime(2.0f);
    setSoundType(SoundType::STONE);
}

BoneBlock::~BoneBlock()
{
}

TextureAtlasSprite* BoneBlock::getTypeTexture(const BlockState*)
{
    return mIcon;
}

void BoneBlock::registerIcons(IconRegister* iconRegister)
{
    mTopTexture = iconRegister->registerIcon(getIconName() + L"_top");
    mIcon = iconRegister->registerIcon(getIconName() + L"_side");
}

EBlockRenderShape BoneBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_TREE;
}

Direction::Axis* BoneBlock::getAxis(const BlockState* state)
{
    const std::vector<const Property*>& properties = state->getProperties();
    return state->getValue(static_cast<const AbstractProperty<Direction::Axis*>*>(properties[0]));
}
