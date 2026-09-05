#include "game/world/block/StainedGlassBlock.hpp"

#include <typeinfo>
#include <vector>
#include <boost/unordered_set.hpp>

#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
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

static EnumProperty<const DyeColor*>* gStainedGlassBlockColorProperty = NULL;
static TextureAtlasSprite* gStainedGlassBlockIcons[16] = { NULL };

void StainedGlassBlock::blockStaticCtor()
{
    boost::unordered_set<const DyeColor*> colors;
    for (int data = 0; data < 16; ++data)
    {
        colors.insert(DyeColor::byBlockData(data));
    }

    gStainedGlassBlockColorProperty = EnumProperty<const DyeColor*>::create(
        L"color", typeid(DyeColor), colors, NULL);
}

StainedGlassBlock::StainedGlassBlock(Material* material)
    : HalfTransparentBlock(L"glass", material, false)
{
    DerivedInit();
    TypedBoxed<const DyeColor*> color(DyeColor::WHITE);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(gStainedGlassBlockColorProperty, &color));
}

StainedGlassBlock::~StainedGlassBlock()
{
}

int StainedGlassBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(gStainedGlassBlockColorProperty)->getBlockData();
}

const MaterialColor* StainedGlassBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    return MaterialColor::getDyeMaterial(state->getValue(gStainedGlassBlockColorProperty));
}

int StainedGlassBlock::getRenderLayer()
{
    return 1;
}

int StainedGlassBlock::getResourceCount(Random*)
{
    return 0;
}

bool StainedGlassBlock::isSilkTouchable()
{
    return true;
}

bool StainedGlassBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

bool StainedGlassBlock::isCubeShaped(const BlockState*)
{
    return true;
}

bool StainedGlassBlock::isTransparent(const BlockState*)
{
    return true;
}

const BlockState* StainedGlassBlock::getBlockState(int data)
{
    TypedBoxed<const DyeColor*> color(DyeColor::byBlockData(data));
    return defaultBlockState()->setBoxedValue(gStainedGlassBlockColorProperty, &color);
}

void StainedGlassBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    if (level->mIsClientSide)
    {
        return;
    }

    BeaconBlock::updateValidityWithinBeam(level, pos);
}

void StainedGlassBlock::onRemove(Level* level, const BlockPos& pos, const BlockState*)
{
    if (level->mIsClientSide)
    {
        return;
    }

    BeaconBlock::updateValidityWithinBeam(level, pos);
}

int StainedGlassBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(gStainedGlassBlockColorProperty)->getBlockData();
}

BlockStateDefinition* StainedGlassBlock::createBlockStateDefinition()
{
    const Property* properties[] = { gStainedGlassBlockColorProperty };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* StainedGlassBlock::getTexture(const Direction*, const BlockState* state)
{
    return gStainedGlassBlockIcons[state->getValue(gStainedGlassBlockColorProperty)->getBlockData()];
}

void StainedGlassBlock::registerIcons(IconRegister* iconRegister)
{
    std::wstring base = getIconName();
    for (int data = 0; data < 16; ++data)
    {
        const DyeColor* color = DyeColor::byBlockData(data);
        gStainedGlassBlockIcons[data] = iconRegister->registerIcon(base + L"_" + color->getTexture());
    }
}
