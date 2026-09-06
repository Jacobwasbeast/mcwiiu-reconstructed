#include "game/world/block/ConcretePowderBlock.hpp"

#include <typeinfo>
#include <vector>
#include <boost/unordered_set.hpp>

#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Direction.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

static EnumProperty<const DyeColor*>* gConcretePowderBlockColorProperty = NULL;

void ConcretePowderBlock::blockStaticCtor()
{
    boost::unordered_set<const DyeColor*> colors;
    for (int i = 0; i < 16; ++i)
    {
        colors.insert(DyeColor::byBlockData(i));
    }
    gConcretePowderBlockColorProperty = EnumProperty<const DyeColor*>::create(L"color", typeid(DyeColor), colors, NULL);
}

ConcretePowderBlock::ConcretePowderBlock()
    : FallingBlock(Material::SAND)
{
    DerivedInit();
    TypedBoxed<const DyeColor*> color(DyeColor::WHITE);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(gConcretePowderBlockColorProperty, &color));
    for (int i = 0; i < 16; ++i)
    {
        mIcons[i] = NULL;
    }
}

ConcretePowderBlock::~ConcretePowderBlock()
{
}

void ConcretePowderBlock::onLand(Level* level, const BlockPos& pos, const BlockState* state, const BlockState* replacedState)
{
    if (replacedState->getMaterial() == Material::WATER)
    {
        level->setBlock(pos, getBlockState(state->getValue(gConcretePowderBlockColorProperty)->getBlockData()), 3, false);
    }
}

unsigned int ConcretePowderBlock::tryTouchWater(Level* level, const BlockPos& pos, const BlockState* state)
{
    for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end(); ++it)
    {
        const Direction* direction = *it;
        if (direction != Direction::DOWN && level->getBlockState(pos.relative(direction))->getMaterial() == Material::WATER)
        {
            level->setBlock(pos, getBlockState(state->getValue(gConcretePowderBlockColorProperty)->getBlockData()), 3, false);
            return 1;
        }
    }
    return 0;
}

void ConcretePowderBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block, const BlockPos& fromPos)
{
    if (tryTouchWater(level, pos, state) == 0)
    {
        FallingBlock::neighborChanged(state, level, pos, block, fromPos);
    }
}

void ConcretePowderBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (tryTouchWater(level, pos, state) == 0)
    {
        FallingBlock::onPlace(level, pos, state);
    }
}

int ConcretePowderBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(gConcretePowderBlockColorProperty)->getBlockData();
}

const MaterialColor* ConcretePowderBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return MaterialColor::getDyeMaterial(state->getValue(gConcretePowderBlockColorProperty));
}

const BlockState* ConcretePowderBlock::getBlockState(int data)
{
    TypedBoxed<const DyeColor*> color(DyeColor::byBlockData(data));
    return defaultBlockState()->setBoxedValue(gConcretePowderBlockColorProperty, &color);
}

int ConcretePowderBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(gConcretePowderBlockColorProperty)->getBlockData();
}

BlockStateDefinition* ConcretePowderBlock::createBlockStateDefinition()
{
    const Property* properties[] = { gConcretePowderBlockColorProperty };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* ConcretePowderBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return mIcons[state->getValue(gConcretePowderBlockColorProperty)->getBlockData()];
}

void ConcretePowderBlock::registerIcons(IconRegister* iconRegister)
{
    std::wstring base = getIconName();
    for (int i = 0; i < 16; ++i)
    {
        const DyeColor* color = DyeColor::byBlockData(i);
        mIcons[color->getItemData()] = iconRegister->registerIcon(base + L"_" + color->getTexture());
    }
}
