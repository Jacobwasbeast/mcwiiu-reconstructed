#include "game/world/block/CoralBlock.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>
#include <string>

#include "game/core/IconRegister.hpp"
#include "game/util/Math.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<int>* CoralBlock::VARIANT =
    static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"variant", 0, 5));
AbstractProperty<bool>* CoralBlock::DEAD =
    static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"dead_coral"));

static std::wstring ICON_NAMES[10] = {
    L"coral_blue",      L"coral_pink",      L"coral_purple",      L"coral_red",      L"coral_yellow",
    L"coral_blue_dead", L"coral_pink_dead", L"coral_purple_dead", L"coral_red_dead", L"coral_yellow_dead"
};

static const unsigned int DESCRIPTION_IDS[9] = {
    0x3513E921U, 0xF7EF6A83U, 0x259DE196U, 0x919D90B9U, 0x323B09A8U,
    0xE8C04B12U, 0xCDCA46E0U, 0x30F2646DU, 0xEE954CBAU
};

CoralBlock::CoralBlock()
    : Block(Material::STONE)
{
    DerivedInit();
    for (int i = 0; i < 10; ++i)
    {
        mIcons[i] = NULL;
    }

    TypedBoxed<int> variant(0);
    TypedBoxed<bool> dead(false);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(VARIANT, &variant)->setBoxedValue(DEAD, &dead));
    setTicking(true);
}

CoralBlock::~CoralBlock()
{
}

Item* CoralBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::coral_block);
}

not_null_ptr<ItemInstance> CoralBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_block, 1, convertBlockStateToLegacyData(state)));
}

void CoralBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (!level->mIsClientSide)
    {
        level->addToTickNextTick(pos, this, getTickDelay(level));
    }
}

void CoralBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!level->mIsClientSide && !level->isTouchingMaterial(pos, Material::WATER)
        && level->isTouchingMaterial(pos, Material::AIR))
    {
        TypedBoxed<bool> dead(true);
        level->setBlock(pos, state->setBoxedValue(DEAD, &dead), Block::UPDATE_ALL, false);
    }
}

TextureAtlasSprite* CoralBlock::getTexture(const Direction*, const BlockState* state)
{
    int variant = state->getValue(VARIANT);
    if (variant > 4)
    {
        variant = 4;
    }
    if (state->getValue(DEAD))
    {
        variant += 5;
    }
    return mIcons[variant];
}

void CoralBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 10; ++i)
    {
        mIcons[i] = iconRegister->registerIcon(ICON_NAMES[i]);
    }
}

int CoralBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(VARIANT);
    if (state->getValue(DEAD))
    {
        data |= 8;
    }
    return data;
}

const BlockState* CoralBlock::getBlockState(int data)
{
    TypedBoxed<int> variant(data & 7);
    TypedBoxed<bool> dead((data >> 3 & 1) != 0);
    return defaultBlockState()->setBoxedValue(VARIANT, &variant)->setBoxedValue(DEAD, &dead);
}

const BlockState* CoralBlock::setPropertyValueFromString(const BlockState* state, const Property* property,
                                                         const std::wstring& value)
{
    if (property == VARIANT && value.compare(L"dead_coral") == 0)
    {
        TypedBoxed<bool> dead(true);
        TypedBoxed<int> variant(Math::randomInt(5));
        return state->setBoxedValue(DEAD, &dead)->setBoxedValue(VARIANT, &variant);
    }
    return Block::setPropertyValueFromString(state, property, value);
}

int CoralBlock::getDescriptionId(int auxValue)
{
    if (auxValue > 0 && auxValue < 5)
    {
        return DESCRIPTION_IDS[auxValue - 1];
    }
    if (auxValue > 7 && auxValue < 13)
    {
        return DESCRIPTION_IDS[auxValue - 4];
    }
    return IDS_TILE_CORAL_BLOCK_BLUE;
}

int CoralBlock::getUseDescriptionId(int auxValue)
{
    if (auxValue > 7 && auxValue < 13)
    {
        return IDS_DESC_TILE_CORAL_BLOCK_DEAD;
    }
    return IDS_DESC_TILE_CORAL_BLOCK;
}

int CoralBlock::getTickDelay(Level* level)
{
    return level->mRandom->nextInt(40, 200);
}

void CoralBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    if (!level->isTouchingMaterial(pos, Material::WATER) && level->isTouchingMaterial(pos, Material::AIR))
    {
        level->addToTickNextTick(pos, this, 45);
    }
}

BlockStateDefinition* CoralBlock::createBlockStateDefinition()
{
    static const Property* properties[] = { DEAD, VARIANT };
    return new BlockStateDefinition(this, properties, 2);
}

int CoralBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT) | 8;
}
