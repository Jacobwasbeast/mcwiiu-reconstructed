#include "game/world/block/Predicates.hpp"
#include "game/world/block/ColoredBlock.hpp"

#include <typeinfo>
#include <vector>
#include <boost/unordered_set.hpp>

#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<const DyeColor*>* ColoredBlock::COLOR = NULL;

void ColoredBlock::blockStaticCtor()
{
    boost::unordered_set<const DyeColor*> colors;
    for (int i = 0; i < 16; ++i)
    {
        colors.insert(DyeColor::byBlockData(i));
    }

    COLOR = EnumProperty<const DyeColor*>::create(L"color", typeid(DyeColor), colors, Predicates<const DyeColor*>::alwaysTrue);
}

ColoredBlock::ColoredBlock(Material* material)
    : Block(material)
{
    DerivedInit();
    TypedBoxed<const DyeColor*> color(DyeColor::WHITE);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(COLOR, &color));
    for (int i = 0; i < 16; ++i)
    {
        mIcons[i] = NULL;
    }
}

ColoredBlock::~ColoredBlock()
{
}

int ColoredBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(COLOR)->getBlockData();
}

const MaterialColor* ColoredBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return MaterialColor::getDyeMaterial(state->getValue(COLOR));
}

const BlockState* ColoredBlock::getBlockState(int data)
{
    TypedBoxed<const DyeColor*> color(DyeColor::byBlockData(data));
    return defaultBlockState()->setBoxedValue(COLOR, &color);
}

int ColoredBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(COLOR)->getBlockData();
}

TextureAtlasSprite* ColoredBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return mIcons[state->getValue(COLOR)->getBlockData()];
}

BlockStateDefinition* ColoredBlock::createBlockStateDefinition()
{
    const Property* properties[] = { COLOR };
    return new BlockStateDefinition(this, properties, 1);
}

void ColoredBlock::registerIcons(IconRegister* iconRegister)
{
    std::wstring base = getIconName();
    for (int i = 0; i < 16; ++i)
    {
        const DyeColor* color = DyeColor::byBlockData(i);
        mIcons[i] = iconRegister->registerIcon(base + L"_" + color->getTexture());
    }
}
