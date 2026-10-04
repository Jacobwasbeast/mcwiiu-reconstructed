#include "game/core/CoralFan.hpp"
#include "game/world/LevelEvent.hpp"

#include <string>
#include <string.h>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<int>* CoralFan::VARIANT = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"variant", 0, 4));
AbstractProperty<int>* CoralFan::DIRECTION = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"direction", 0, 1));

namespace
{
    std::wstring DEAD_CORAL_FAN_ICON_NAMES[5] =
    {
        L"coral_fan_blue_dead", L"coral_fan_pink_dead", L"coral_fan_purple_dead",
        L"coral_fan_red_dead", L"coral_fan_yellow_dead"
    };

    std::wstring CORAL_FAN_ICON_NAMES[5] =
    {
        L"coral_fan_blue", L"coral_fan_pink", L"coral_fan_purple", L"coral_fan_red",
        L"coral_fan_yellow"
    };

    const AABB* CORAL_FAN_SHAPE = AABB::newPermanent(
        0.19999998807907104, 0.0, 0.19999998807907104,
        0.8f, 0.4f, 0.8f);
}

CoralFan::CoralFan()
    : Bush(Material::STONE)
{
    DerivedInit();

    memset(mIcons, 0, sizeof(mIcons));

    registerDefaultState(defaultBlockState()->setValue(CoralFan::VARIANT, 3)
                                            ->setValue(CoralFan::DIRECTION, 1));
}

int CoralFan::getColor(const BlockState* state)
{
    return -1;
}

int CoralFan::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    return -1;
}

const MaterialColor* CoralFan::getMapColor(const BlockState* state, LevelSource* levelSource,
                                           const BlockPos& pos)
{
    return Blocks::grass->getMapColor(state, levelSource, pos);
}

Item* CoralFan::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return NULL;
}

const AABB* CoralFan::getShape(const BlockState* state, LevelSource* levelSource,
                               const BlockPos& pos)
{
    return CORAL_FAN_SHAPE;
}

bool CoralFan::isSolidRender(const BlockState* state) const
{
    return false;
}

bool CoralFan::isCubeShaped(const BlockState* state)
{
    return false;
}

int CoralFan::getRenderLayer()
{
    return 0;
}

EBlockRenderShape CoralFan::getRenderShape(const BlockState* state)
{
    return BLOCK_RENDER_CORAL_FAN;
}

void CoralFan::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (level->mIsClientSide)
    {
        return;
    }

    checkAlive(level, pos, state);
}

bool CoralFan::canContainLiquid(const BlockState* state) const
{
    return true;
}

void CoralFan::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                               Block* block, const BlockPos& fromPos)
{
    if (level->mIsClientSide)
    {
        return;
    }

    if (!level->isSolidBlockingBlock(pos.below()))
    {
        level->addToTickNextTick(pos, this, 1);
        return;
    }

    int delay = level->mRandom->nextInt(40, 200);
    level->addToTickNextTick(pos, this, delay);
}

EBlockOffsetType CoralFan::getOffsetType()
{
    return BLOCK_OFFSET_XZ;
}

TextureAtlasSprite* CoralFan::getTexture(const Direction* direction, const BlockState* state)
{
    int type = state->getValue(CoralFan::VARIANT);

    if (this == Blocks::coral_fan_dead)
    {
        return mDeadIcons[type];
    }

    return mIcons[type];
}

void CoralFan::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 5; ++i)
    {
        mIcons[i] = iconRegister->registerIcon(CORAL_FAN_ICON_NAMES[i]);
        mDeadIcons[i] = iconRegister->registerIcon(DEAD_CORAL_FAN_ICON_NAMES[i]);
    }
}

int CoralFan::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(CoralFan::VARIANT) | (state->getValue(CoralFan::DIRECTION) << 3);
}

const BlockState* CoralFan::getBlockState(int data)
{
    return defaultBlockState()->setValue(CoralFan::VARIANT, data & 7)
                              ->setValue(CoralFan::DIRECTION, (data >> 3) & 1);
}

int CoralFan::getDescriptionId(int auxValue)
{
    const BlockState* state = getBlockState(auxValue);
    bool dead = state->getBlock() == Blocks::coral_fan_dead;

    int variant = state->getValue(CoralFan::VARIANT);

    if (variant != 0)
    {
        if (variant == 1)
        {
            return dead ? 0x49DEF5D4 : 0x54923487;
        }

        if (variant == 2)
        {
            return dead ? static_cast<int>(0x842794C6U) : 0x5C092885;
        }

        if (variant == 3)
        {
            return dead ? 0x4A67CEEB : 0x29A6A280;
        }

        if (variant == 4)
        {
            return dead ? static_cast<int>(0x93926B4CU) : 0x6932757F;
        }
    }

    return dead ? static_cast<int>(0xB34422EEU) : static_cast<int>(0xA1128C9DU);
}

bool CoralFan::mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos)
{
    return true;
}

not_null_ptr<ItemInstance> CoralFan::getSilkTouchItemInstance(const BlockState* state)
{
    const BlockState* upright = state->setValue(CoralFan::DIRECTION, 0);
    int auxValue = convertBlockStateToLegacyData(upright);
    return not_null_ptr<ItemInstance>(new ItemInstance(this, 1, auxValue));
}

not_null_ptr<ItemInstance> CoralFan::getCloneItemInstance(Level* level, const BlockPos& pos,
                                                          const BlockState* state)
{
    const BlockState* upright = state->setValue(CoralFan::DIRECTION, 0);
    int auxValue = convertBlockStateToLegacyData(upright);
    return not_null_ptr<ItemInstance>(new ItemInstance(this, 1, auxValue));
}

BlockStateDefinition* CoralFan::createBlockStateDefinition()
{
    static const Property* PROPERTIES[2] = { CoralFan::VARIANT, CoralFan::DIRECTION };

    return new BlockStateDefinition(this, PROPERTIES, 2);
}

bool CoralFan::isSilkTouchable()
{
    return true;
}

bool CoralFan::mayPlace(Level* level, const BlockPos& pos)
{
    return Block::mayPlace(level, pos);
}

bool CoralFan::mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state)
{
    const BlockState* support = level->getBlockState(pos);

    if (support->canProvideSupport(Direction::UP, BLOCK_SUPPORT_CENTER))
    {
        return true;
    }

    if (HalfSlabBlock::isHalfSlab(support))
    {
        if (support->getValue(HalfSlabBlock::TYPE) == HalfSlabBlock::TOP)
        {
            return true;
        }
    }

    return false;
}

void CoralFan::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (this != Blocks::coral_fan_dead && level->getLiquidBlock(pos)->mMaterial == mMaterial)
    {
        level->setExtraBlock(pos, Blocks::flowing_water->defaultBlockState(), Block::UPDATE_ALL, false);
    }
}

bool CoralFan::isWaterBlocking() const
{
    return false;
}

void CoralFan::checkAlive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!level->isUnderWater(pos, 263, 0))
    {
        const BlockState* dead = Blocks::coral_fan_dead->defaultBlockState();
        level->setBlock(pos,
                        dead->setValue(CoralFan::VARIANT, state->getValue(CoralFan::VARIANT))
                            ->setValue(CoralFan::DIRECTION, state->getValue(CoralFan::DIRECTION)),
                        Block::UPDATE_ALL, false);
    }

    if (!level->isSolidBlockingBlock(pos.below()) && !level->getBlock(pos.below())->isSlabBlock())
    {
        level->levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, pos, Block::getIdWithData(state));
        level->setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);
    }
}

