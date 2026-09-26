#include "game/world/block/Predicates.hpp"
#include "game/world/block/StainedGlassPaneBlock.hpp"

#include <typeinfo>
#include <vector>
#include <boost/unordered_set.hpp>

#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BeaconBlock.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<const DyeColor*>* StainedGlassPaneBlock::COLOR = NULL;
static TextureAtlasSprite* gStainedGlassPaneIcons[16];
static TextureAtlasSprite* gStainedGlassPaneEdges[16];

void StainedGlassPaneBlock::blockStaticCtor()
{
    boost::unordered_set<const DyeColor*> colors;
    for (int data = 0; data < 16; ++data)
    {
        colors.insert(DyeColor::byBlockData(data));
    }

    COLOR = EnumProperty<const DyeColor*>::create(L"color", typeid(DyeColor), colors, Predicates<const DyeColor*>::alwaysTrue);
}

StainedGlassPaneBlock::StainedGlassPaneBlock()
    : ThinFenceBlock(L"glass", L"glass_pane_top", Material::GLASS, false)
{
    ClearDerivedInit();
    DerivedInit();
    TypedBoxed<bool> north(false);
    TypedBoxed<bool> east(false);
    TypedBoxed<bool> south(false);
    TypedBoxed<bool> west(false);
    TypedBoxed<const DyeColor*> color(DyeColor::WHITE);
    const BlockState* state = mStateDefinition->any();
    state = state->setBoxedValue(THIN_FENCE_NORTH, &north);
    state = state->setBoxedValue(THIN_FENCE_EAST, &east);
    state = state->setBoxedValue(THIN_FENCE_SOUTH, &south);
    state = state->setBoxedValue(THIN_FENCE_WEST, &west);
    registerDefaultState(state->setBoxedValue(COLOR, &color));
}

StainedGlassPaneBlock::~StainedGlassPaneBlock()
{
}

int StainedGlassPaneBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(COLOR)->getBlockData();
}

const MaterialColor* StainedGlassPaneBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    return MaterialColor::getDyeMaterial(state->getValue(COLOR));
}

int StainedGlassPaneBlock::getRenderLayer()
{
    return 1;
}

const BlockState* StainedGlassPaneBlock::getBlockState(int data)
{
    TypedBoxed<const DyeColor*> color(DyeColor::byBlockData(data));
    return defaultBlockState()->setBoxedValue(COLOR, &color);
}

int StainedGlassPaneBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(COLOR)->getBlockData();
}

const BlockState* StainedGlassPaneBlock::rotate(const BlockState* state, Rotation* rotation)
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise180:
    {
        TypedBoxed<bool> north(state->getValue(THIN_FENCE_SOUTH));
        TypedBoxed<bool> east(state->getValue(THIN_FENCE_WEST));
        TypedBoxed<bool> south(state->getValue(THIN_FENCE_NORTH));
        TypedBoxed<bool> west(state->getValue(THIN_FENCE_EAST));
        return state->setBoxedValue(THIN_FENCE_NORTH, &north)
            ->setBoxedValue(THIN_FENCE_EAST, &east)
            ->setBoxedValue(THIN_FENCE_SOUTH, &south)
            ->setBoxedValue(THIN_FENCE_WEST, &west);
    }
    case Rotation::eTYPE_CounterClockwise90:
    {
        TypedBoxed<bool> north(state->getValue(THIN_FENCE_EAST));
        TypedBoxed<bool> east(state->getValue(THIN_FENCE_SOUTH));
        TypedBoxed<bool> south(state->getValue(THIN_FENCE_WEST));
        TypedBoxed<bool> west(state->getValue(THIN_FENCE_NORTH));
        return state->setBoxedValue(THIN_FENCE_NORTH, &north)
            ->setBoxedValue(THIN_FENCE_EAST, &east)
            ->setBoxedValue(THIN_FENCE_SOUTH, &south)
            ->setBoxedValue(THIN_FENCE_WEST, &west);
    }
    case Rotation::eTYPE_Clockwise90:
    {
        TypedBoxed<bool> north(state->getValue(THIN_FENCE_WEST));
        TypedBoxed<bool> east(state->getValue(THIN_FENCE_NORTH));
        TypedBoxed<bool> south(state->getValue(THIN_FENCE_EAST));
        TypedBoxed<bool> west(state->getValue(THIN_FENCE_SOUTH));
        return state->setBoxedValue(THIN_FENCE_NORTH, &north)
            ->setBoxedValue(THIN_FENCE_EAST, &east)
            ->setBoxedValue(THIN_FENCE_SOUTH, &south)
            ->setBoxedValue(THIN_FENCE_WEST, &west);
    }
    default:
        return state;
    }
}

const BlockState* StainedGlassPaneBlock::mirror(const BlockState* state, Mirror* mirror)
{
    switch (mirror->getType())
    {
    case Mirror::eTYPE_LeftRight:
    {
        TypedBoxed<bool> north(state->getValue(THIN_FENCE_SOUTH));
        TypedBoxed<bool> south(state->getValue(THIN_FENCE_NORTH));
        return state->setBoxedValue(THIN_FENCE_NORTH, &north)->setBoxedValue(THIN_FENCE_SOUTH, &south);
    }
    case Mirror::eTYPE_FrontBack:
    {
        TypedBoxed<bool> east(state->getValue(THIN_FENCE_WEST));
        TypedBoxed<bool> west(state->getValue(THIN_FENCE_EAST));
        return state->setBoxedValue(THIN_FENCE_EAST, &east)->setBoxedValue(THIN_FENCE_WEST, &west);
    }
    default:
        return ThinFenceBlock::mirror(state, mirror);
    }
}

BlockStateDefinition* StainedGlassPaneBlock::createBlockStateDefinition()
{
    const Property* properties[5] = { THIN_FENCE_NORTH, THIN_FENCE_EAST,
        THIN_FENCE_SOUTH, THIN_FENCE_WEST, COLOR };
    return new BlockStateDefinition(this, properties, 5);
}

void StainedGlassPaneBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    if (level->mIsClientSide)
        return;
    BeaconBlock::updateValidityWithinBeam(level, pos);
}

void StainedGlassPaneBlock::onRemove(Level* level, const BlockPos& pos, const BlockState*)
{
    if (level->mIsClientSide)
        return;
    BeaconBlock::updateValidityWithinBeam(level, pos);
}

TextureAtlasSprite* StainedGlassPaneBlock::getIconTexture(const Direction*, const BlockState* state)
{
    return gStainedGlassPaneIcons[state->getValue(COLOR)->getBlockData()];
}

TextureAtlasSprite* StainedGlassPaneBlock::getEdgeTexture(const BlockState* state)
{
    return gStainedGlassPaneEdges[state->getValue(COLOR)->getBlockData()];
}

TextureAtlasSprite* StainedGlassPaneBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return getIconTexture(direction, state);
}

void StainedGlassPaneBlock::registerIcons(IconRegister* iconRegister)
{
    ThinFenceBlock::registerIcons(iconRegister);
    for (int data = 0; data < 16; ++data)
    {
        const DyeColor* color = DyeColor::byBlockData(data);
        std::wstring name = getIconName() + L"_" + color->getTexture();
        gStainedGlassPaneIcons[data] = iconRegister->registerIcon(name);
        name = getIconName() + L"_pane_top_" + color->getTexture();
        gStainedGlassPaneEdges[data] = iconRegister->registerIcon(name);
    }
}

TextureAtlasSprite* StainedGlassPaneBlock::getEdgeTexture()
{
    return getEdgeTexture(defaultBlockState());
}
