#include "game/world/block/CoralFanBlockItem.hpp"

#include <math.h>

#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/CoralFan.hpp"
#include "game/core/CoralFanHang.hpp"
#include "game/core/Direction.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

CoralFanBlockItem::CoralFanBlockItem(Block* block)
    : BlockItem(block), mIsDead(block == Blocks::coral_fan_dead)
{
    setStackedByData(true);
    setMaxDamage(0);
}

int CoralFanBlockItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    int auxValue = itemInstance->getAuxValue();
    if (mIsDead)
    {
        return Blocks::coral_fan_dead->getDescriptionId(auxValue);
    }
    return Blocks::coral_fan->getDescriptionId(auxValue);
}

int CoralFanBlockItem::getUseDescriptionId(not_null_ptr<ItemInstance>)
{
    if (mIsDead)
    {
        return Blocks::coral_fan_dead->getUseDescriptionId(-1);
    }
    return Blocks::coral_fan->getUseDescriptionId(-1);
}

InteractionResult::EInteractionResult CoralFanBlockItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                                const BlockPos& pos,
                                                                InteractionHand::EInteractionHand hand,
                                                                const Direction* direction, float, float, float,
                                                                bool simulate)
{
    BlockPos placePos = pos.relative(direction);
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    int variant = itemInstance->getAuxValue() & 7;

    if (!level->isEmptyBlock(pos.relative(direction)) && !level->isEmptyWaterBlock(pos.relative(direction)))
        return InteractionResult::eInteractionResult_Fail;
    if (level->getLiquidBlock(placePos)->mMaterial == Material::WATER
        && static_cast<unsigned int>(level->getLiquidBlockState(placePos)->getValue(LiquidBlock::LEVEL) - 1) < 7)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    bool isUp = direction == Direction::UP;
    bool underWater = level->isUnderWater(placePos, 0x108, 0);
    const BlockState* state;
    if (isUp)
    {
        state = Blocks::coral_fan->defaultBlockState();
        if (direction != Direction::UP || !state->getBlock()->mayPlaceOn(level, pos, state))
            return InteractionResult::eInteractionResult_Fail;

        Vec3* facing = (pos - player->getPos(0.0f))->normalize();
        float angle = static_cast<float>(atan2(facing->z, facing->x));
        int fanDirection = ((angle > 0.7853982f && angle < 2.3561945f) || (angle > 3.926991f && angle < 5.4977875f))
                               ? 0
                               : 1;
        if (!underWater || itemInstance->getItem() == Item::byBlock(Blocks::coral_fan_dead))
            state = Blocks::coral_fan_dead->defaultBlockState();
        TypedBoxed<int> directionValue(fanDirection);
        TypedBoxed<int> variantValue(variant);
        state = state->setBoxedValue(CoralFan::DIRECTION, &directionValue)->setBoxedValue(CoralFan::VARIANT, &variantValue);
    }
    else
    {
        state = Blocks::coral_fan_hang->defaultBlockState();
        if (variant < 2)
        {
            TypedBoxed<int> hangVariant(variant);
            state = Blocks::coral_fan_hang->defaultBlockState()->setBoxedValue(CoralFanHang::VARIANT, &hangVariant);
        }
        else if (variant < 4)
        {
            TypedBoxed<int> hangVariant(variant - 2);
            state = Blocks::coral_fan_hang2->defaultBlockState()->setBoxedValue(CoralFanHang::VARIANT, &hangVariant);
        }
        else if (variant < 6)
        {
            TypedBoxed<int> hangVariant(0);
            state = Blocks::coral_fan_hang3->defaultBlockState()->setBoxedValue(CoralFanHang::VARIANT, &hangVariant);
        }

        const BlockState* support = level->getBlockState(pos);
        if (!support->isSolidBlockingBlock() && !support->getBlock()->isSlabBlock())
            return InteractionResult::eInteractionResult_Fail;
        if (direction == Direction::UP || direction == Direction::DOWN)
            return InteractionResult::eInteractionResult_Fail;
        if (!state->getBlock()->mayPlaceOn(level, pos, state))
            return InteractionResult::eInteractionResult_Fail;

        int hangDirection;
        if (direction == Direction::NORTH)
            hangDirection = 2;
        else if (direction == Direction::EAST)
            hangDirection = 1;
        else if (direction == Direction::SOUTH)
            hangDirection = 3;
        else
            hangDirection = 0;
        TypedBoxed<int> directionValue(hangDirection);
        state = state->setBoxedValue(CoralFanHang::DIRECTION, &directionValue);
        if (!underWater || itemInstance->getItem() == Item::byBlock(Blocks::coral_fan_dead))
        {
            TypedBoxed<bool> dead(true);
            state = state->setBoxedValue(CoralFanHang::DEAD, &dead);
        }
    }

    if (!simulate)
    {
        level->setBlock(placePos, state, Block::UPDATE_ALL, false);
        itemInstance->shrink(1);
        level->getBlockState(pos);
        player->swing(InteractionHand::eInteractionHand_MainHand);
        const SoundType* soundType = mBlock->getSoundType();
        level->playSound(player, pos, soundType->getPlaceSound(), SoundSource::eSoundSource_Blocks,
                         (soundType->getVolume() + 1.0f) * 0.5f, soundType->getPitch() * 0.8f, 16.0f);
    }
    return InteractionResult::eInteractionResult_Success;
}

int CoralFanBlockItem::getLevelDataForAuxValue(int auxValue)
{
    return auxValue;
}
