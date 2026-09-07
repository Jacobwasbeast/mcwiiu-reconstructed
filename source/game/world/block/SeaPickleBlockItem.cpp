#include "game/world/block/SeaPickleBlockItem.hpp"

#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/Direction.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/Player.hpp"

SeaPickleBlockItem::SeaPickleBlockItem(Block* block)
    : BlockItem(block)
{
    Item::setMaxDamage(0);
    Item::setStackedByData(true);
}

SeaPickleBlockItem::~SeaPickleBlockItem()
{
}

int SeaPickleBlockItem::getLevelDataForAuxValue(int auxValue)
{
    return auxValue;
}

InteractionResult::EInteractionResult SeaPickleBlockItem::useOn(boost::shared_ptr<Player> player,
                                                                Level* level,
                                                                const BlockPos& pos,
                                                                InteractionHand::EInteractionHand hand,
                                                                const Direction* direction,
                                                                float hitX,
                                                                float hitY,
                                                                float hitZ,
                                                                bool simulate)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (itemInstance->isEmpty())
        return InteractionResult::eInteractionResult_Fail;

    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();
    if (block == getBlock())
    {
        int data = block->convertBlockStateToLegacyData(state);
        int count = (data & 7) + 1;
        if (count < 4 && level->isUnderWater(pos, state))
        {
            if (simulate)
                return InteractionResult::eInteractionResult_Success;

            const BlockState* newState = block->getBlockState(data + 1);
            if (level->setBlock(pos, newState, 2, false))
            {
                itemInstance->shrink(1);
                return InteractionResult::eInteractionResult_Success;
            }
            return InteractionResult::eInteractionResult_Fail;
        }
    }

    BlockPos placePos = pos.relative(direction);
    const BlockState* placeState = level->getBlockState(placePos);
    if (placeState->getBlock() == getBlock())
    {
        int data = getBlock()->convertBlockStateToLegacyData(placeState);
        int count = (data & 7) + 1;
        if (count < 4 && level->isUnderWater(placePos, placeState))
        {
            if (simulate)
                return InteractionResult::eInteractionResult_Success;

            const BlockState* newState = getBlock()->getBlockState((data & 8) | count);
            if (level->setBlock(placePos, newState, 2, false))
            {
                itemInstance->shrink(1);
                return InteractionResult::eInteractionResult_Success;
            }
            return InteractionResult::eInteractionResult_Fail;
        }
    }

    if (!level->isUnderWater(placePos, placeState)
        || !level->canProvideSupport(placePos.below(), Direction::UP, BLOCK_SUPPORT_ANY))
        return InteractionResult::eInteractionResult_Fail;

    const BlockState* newState = getBlock()->getStateForPlacement(level, placePos, direction,
                                                                   hitX, hitY, hitZ, 0, player);
    if (!simulate)
    {
        const AABB* shape = getBlock()->getShape(newState, level, placePos);
        if (!level->isUnobstructed(shape)
            || !level->setBlock(placePos, newState, 3, false))
            return InteractionResult::eInteractionResult_Fail;

        const SoundType* soundType = getBlock()->getSoundType();
        level->playSound(boost::shared_ptr<Player>(), placePos, soundType->getPlaceSound(),
                         SoundSource::eSoundSource_Blocks,
                         (soundType->getVolume() + 1.0f) * 0.5f,
                         soundType->getPitch() * 0.8f, 16.0f);
        itemInstance->shrink(1);
    }
    return InteractionResult::eInteractionResult_Success;
}
