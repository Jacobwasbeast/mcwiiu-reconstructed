#include "game/core/Coral.hpp"
#include "game/core/StringIds.hpp"

#include <cstring>
#include <string>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/IntegerProperty.hpp"

AbstractProperty<int>* Coral::VARIANT = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"variant", 0, 4));

namespace
{
    std::wstring TEXTURE_BLUE = L"coral_plant_blue";
    std::wstring TEXTURE_PINK = L"coral_plant_pink";
    std::wstring TEXTURE_PURPLE = L"coral_plant_purple";
    std::wstring TEXTURE_RED = L"coral_plant_red";
    std::wstring TEXTURE_YELLOW = L"coral_plant_yellow";

    const AABB* CORAL_SHAPE = AABB::newPermanent(0.1f, 0.0, 0.1f, 0.9f, 0.8f, 0.9f);
}

Coral::Coral()
    : Bush(Material::REPLACEABLE_PLANT)
{
    DerivedInit();

    memset(mIcons, 0, sizeof(mIcons));
}

int Coral::getColor(const BlockState* state)
{
    return Blocks::grass->getColor(state);
}

int Coral::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    return Blocks::grass->getColor(levelSource, pos, state);
}

const MaterialColor* Coral::getMapColor(const BlockState* state, LevelSource* levelSource,
                                        const BlockPos& pos)
{
    return Blocks::grass->getMapColor(state, levelSource, pos);
}

Item* Coral::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return NULL;
}

not_null_ptr<ItemInstance> Coral::getCloneItemInstance(Level* level, const BlockPos& pos,
                                                       const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral, 1,
                                                       convertBlockStateToLegacyData(state)));
}

bool Coral::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    return mayPlaceOn(level, pos.below(), level->getBlockState(pos.below()));
}

void Coral::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                            Block* block, const BlockPos& fromPos)
{
    if (level->mIsClientSide)
    {
        return;
    }

    if (!level->isSolidBlockingBlock(pos.below()) && !level->getBlock(pos.below())->isSlabBlock())
    {
        level->setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);
    }

    level->addToTickNextTick(pos, this, 10);
}

TextureAtlasSprite* Coral::getTexture(const Direction* direction, const BlockState* state)
{
    return mIcons[state->getValue(Coral::VARIANT)];
}

void Coral::registerIcons(IconRegister* iconRegister)
{
    mIcons[0] = iconRegister->registerIcon(TEXTURE_BLUE);
    mIcons[1] = iconRegister->registerIcon(TEXTURE_PINK);
    mIcons[2] = iconRegister->registerIcon(TEXTURE_PURPLE);
    mIcons[3] = iconRegister->registerIcon(TEXTURE_RED);
    mIcons[4] = iconRegister->registerIcon(TEXTURE_YELLOW);
}

int Coral::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(Coral::VARIANT);
}

const BlockState* Coral::getBlockState(int data)
{
    const BlockState* defaultState = defaultBlockState();
    AbstractProperty<int>* property = Coral::VARIANT;
    TypedBoxed<int> variant(data);
    return defaultState->setBoxedValue(property, &variant);
}

int Coral::getDescriptionId(int auxValue)
{
    switch (auxValue)
    {
    case 1:
        return IDS_TILE_CORAL_PINK;
    case 2:
        return IDS_TILE_CORAL_PURPLE;
    case 3:
        return IDS_TILE_CORAL_RED;
    case 4:
        return IDS_TILE_CORAL_YELLOW;
    default:
        return IDS_TILE_CORAL_BLUE;
    }
}

int Coral::getUseDescriptionId(int auxValue)
{
    return IDS_DESC_TILE_CORAL;
}

const AABB* Coral::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return CORAL_SHAPE;
}

BlockStateDefinition* Coral::createBlockStateDefinition()
{
    static const Property* PROPERTIES[1] = { Coral::VARIANT };

    return new BlockStateDefinition(this, PROPERTIES, 1);
}

bool Coral::isSilkTouchable()
{
    return true;
}

bool Coral::mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state)
{
    BlockPos abovePos = pos.above();
    if (!level->isUnderWater(abovePos, 263, 0))
    {
        return false;
    }

    if (level->getBlock(pos.above(2))->mMaterial == Material::AIR)
    {
        return false;
    }

    if (level->getLiquidBlock(pos)->mMaterial == Material::WATER)
    {
        int liquidLevel = level->getLiquidBlockState(pos)->getValue(LiquidBlock::LEVEL);
        if (static_cast<unsigned int>(liquidLevel - 1) < 7U)
        {
            return false;
        }
    }

    Block* block = state->getBlock();
    bool onSlabTop = false;

    if (block->isSlabBlock())
    {
        if (!HalfSlabBlock::isBottomSlab(block->convertBlockStateToLegacyData(state)))
        {
            onSlabTop = true;
        }
    }

    if (state->canProvideSupport(Direction::UP, BLOCK_SUPPORT_CENTER) || onSlabTop)
    {
        return true;
    }

    return false;
}

void Coral::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (level->getLiquidBlock(pos)->mMaterial == mMaterial)
    {
        level->setExtraBlock(pos, Blocks::flowing_water->defaultBlockState(), Block::UPDATE_ALL, false);
    }
}
