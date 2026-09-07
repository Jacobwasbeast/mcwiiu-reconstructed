#include "game/world/block/CoralBlock.hpp"

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

static AbstractProperty<int>* CORAL_TYPE = NULL;
static AbstractProperty<bool>* CORAL_DEAD = NULL;
Property* CoralBlock_PROPERTY_1 = NULL;
Property* CoralBlock_PROPERTY_2 = NULL;

namespace
{
    std::wstring gCoralBlockIconNames[10] =
    {
        L"coral_red", L"coral_orange", L"coral_yellow", L"coral_blue", L"coral_purple",
        L"dead_coral_red", L"dead_coral_orange", L"dead_coral_yellow", L"dead_coral_blue", L"dead_coral_purple"
    };

    void destroyCoralBlockIconNames(std::wstring* names, int count)
    {
        for (int i = 0; i < count; ++i)
            names[i].~basic_string();
    }
}

void __T13()
{
    destroyCoralBlockIconNames(gCoralBlockIconNames, 10);
}

static const unsigned int CORAL_DESCRIPTION_IDS[8] =
{
    0x3513E921U, 0xF7EF6A83U, 0x259DE196U, 0x919D90B9U,
    0x323B09A8U, 0xE8C04B12U, 0xCDCA46E0U, 0x30F2646DU
};

CoralBlock::CoralBlock()
    : Block(Material::CORAL)
{
    DerivedInit();
    for (int i = 0; i < 10; ++i)
    {
        mIcons[i] = NULL;
    }

    TypedBoxed<int> type(0);
    TypedBoxed<bool> dead(false);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(CORAL_TYPE, &type)->setBoxedValue(CORAL_DEAD, &dead));
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
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_block, 1,
                                                       convertBlockStateToLegacyData(state)));
}

void CoralBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                 Block*, const BlockPos&)
{
    if (!level->mIsClientSide)
    {
        level->levelEvent(getTickDelay(level), pos, 0);
    }
}

void CoralBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!level->mIsClientSide && !level->isTouchingMaterial(pos, Material::WATER)
        && level->isTouchingMaterial(pos, Material::CORAL))
    {
        TypedBoxed<bool> dead(true);
        level->setBlock(pos, state->setBoxedValue(CORAL_DEAD, &dead), 3, false);
    }
}

TextureAtlasSprite* CoralBlock::getTexture(const Direction*, const BlockState* state)
{
    int type = state->getValue(CORAL_TYPE);
    if (type > 4)
    {
        type = 4;
    }
    if (state->getValue(CORAL_DEAD))
    {
        type += 5;
    }
    return mIcons[type];
}

void CoralBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 10; ++i)
    {
        mIcons[i] = iconRegister->registerIcon(gCoralBlockIconNames[i]);
    }
}

int CoralBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(CORAL_TYPE);
    if (state->getValue(CORAL_DEAD))
    {
        data |= 8;
    }
    return data;
}

const BlockState* CoralBlock::getBlockState(int data)
{
    TypedBoxed<int> type(data & 7);
    const BlockState* state = defaultBlockState()->setBoxedValue(CORAL_TYPE, &type);
    TypedBoxed<bool> dead((data >> 3) & 1);
    return state->setBoxedValue(CORAL_DEAD, &dead);
}

const BlockState* CoralBlock::setPropertyValueFromString(const BlockState* state,
                                                         const Property* property,
                                                         const std::wstring& value)
{
    if (property == CORAL_DEAD && value.compare(L"dead_coral") == 0)
    {
        TypedBoxed<bool> dead(true);
        TypedBoxed<int> type(Math::randomInt(5));
        return state->setBoxedValue(CORAL_DEAD, &dead)->setBoxedValue(CORAL_TYPE, &type);
    }
    return Block::setPropertyValueFromString(state, property, value);
}

int CoralBlock::getDescriptionId(int auxValue)
{
    if (auxValue > 0 && auxValue < 5)
    {
        return CORAL_DESCRIPTION_IDS[auxValue];
    }
    if (auxValue > 7 && auxValue < 13)
    {
        return CORAL_DESCRIPTION_IDS[auxValue - 5];
    }
    return -1854932677;
}

int CoralBlock::getUseDescriptionId(int auxValue)
{
    if (auxValue > 7 && auxValue < 13)
    {
        return 0x7BF6B03F;
    }
    return 0x7A0CC2D4;
}

int CoralBlock::getTickDelay(Level* level)
{
    return level->mRandom->nextInt(40, 200);
}

void CoralBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!level->isTouchingMaterial(pos, Material::WATER)
        && level->isTouchingMaterial(pos, Material::CORAL))
    {
        level->levelEvent(0x2D, pos, 0);
    }
}

BlockStateDefinition* CoralBlock::createBlockStateDefinition()
{
    if (CORAL_TYPE == NULL)
    {
        CORAL_TYPE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"type", 0, 4));
        CORAL_DEAD = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"dead"));
        CoralBlock_PROPERTY_1 = CORAL_TYPE;
        CoralBlock_PROPERTY_2 = CORAL_DEAD;
    }
    const Property* properties[2] = { CORAL_TYPE, CORAL_DEAD };
    return new BlockStateDefinition(this, properties, 2);
}

int CoralBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(CORAL_TYPE) | 8;
}
