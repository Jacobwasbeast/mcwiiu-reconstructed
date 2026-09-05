#include "game/item/DoorItem.hpp"

#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Stats.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

DoorItem::DoorItem(Block* block) : Item(), mBlock(block) {}

DoorItem::~DoorItem() {}

void DoorItem::place(Level* level, const BlockPos& pos, const Direction* direction, Block* block, bool hingeRight)
{
    BlockPos clockwise = pos.relative(direction->getClockWise());
    BlockPos counterClockwise = pos.relative(direction->getCounterClockWise());
    BlockPos clockwiseAbove = clockwise.above();
    BlockPos counterClockwiseAbove = counterClockwise.above();
    const BlockState* clockwiseState = level->getBlockState(clockwise);
    const BlockState* counterClockwiseState = level->getBlockState(counterClockwise);
    const BlockState* clockwiseAboveState = level->getBlockState(clockwiseAbove);
    const BlockState* counterClockwiseAboveState = level->getBlockState(counterClockwiseAbove);
    int counterClockwiseSupport = counterClockwiseState->getBlock()->isSolidRender(counterClockwiseState)
        + counterClockwiseAboveState->getBlock()->isSolidRender(counterClockwiseAboveState);
    int clockwiseSupport = clockwiseState->getBlock()->isSolidRender(clockwiseState)
        + clockwiseAboveState->getBlock()->isSolidRender(clockwiseAboveState);
    bool counterClockwiseDoor = counterClockwiseState->getBlock() == block
        || counterClockwiseAboveState->getBlock() == block;
    bool clockwiseDoor = clockwiseState->getBlock() == block
        || clockwiseAboveState->getBlock() == block;
    if ((counterClockwiseDoor && !clockwiseDoor) || counterClockwiseSupport < clockwiseSupport)
    {
        hingeRight = true;
    }
    else
    {
        if ((clockwiseDoor && !counterClockwiseDoor) || clockwiseSupport < counterClockwiseSupport)
        {
            hingeRight = false;
        }
    }
    bool powered = level->hasNeighborSignal(pos) || level->hasNeighborSignal(pos.above());
    const BlockState* state = block->defaultBlockState();
    TypedBoxed<Direction*> facing(const_cast<Direction*>(direction));
    TypedBoxed<DoorBlock::HingeSide*> hinge(hingeRight ? DoorBlock::RIGHT : DoorBlock::LEFT);
    TypedBoxed<bool> open(powered);
    TypedBoxed<DoorBlock::DoorHalf*> half(DoorBlock::LOWER);
    state = state->setBoxedValue(DoorBlock::FACING, &facing);
    state = state->setBoxedValue(DoorBlock::HINGE, &hinge);
    state = state->setBoxedValue(static_cast<AbstractProperty<bool>*>(DoorBlock::OPEN), &open);
    state = state->setBoxedValue(static_cast<AbstractProperty<bool>*>(DoorBlock::POWERED), &open);
    state = state->setBoxedValue(DoorBlock::HALF, &half);
    level->setBlock(pos, state, 3, false);
    half = DoorBlock::UPPER;
    state = state->setBoxedValue(DoorBlock::HALF, &half);
    level->setBlock(pos.above(), state, 3, false);
    level->updateNeighborsAt(pos, block, false);
    level->updateNeighborsAt(pos.above(), block, false);
}

InteractionResult::EInteractionResult DoorItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                       const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                       const Direction* direction, float hitX, float hitY, float hitZ,
                                                       bool simulate)
{
    BlockPos placePos = pos;
    const BlockState* state = level->getBlockState(placePos);
    if (!state->getBlock()->mayReplaceWithPlace(level, placePos)) placePos = placePos.relative(direction);
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (itemInstance->isEmpty() || !player->mayUseItemAt(placePos, direction, itemInstance)
        || !level->mayPlace(mBlock, placePos, false, direction, boost::shared_ptr<Entity>()))
        return InteractionResult::eInteractionResult_Fail;
    if (!simulate)
    {
        const Direction* facing = Direction::fromYRot((double)player->mYRot);
        bool hingeRight = false;
        if (facing->getAxis()->getStep() < 1)
        {
            if (facing->getAxis()->getStep() >= -1 && hitY <= 0.5f) hingeRight = true;
        }
        else if (hitY > 0.5f) hingeRight = true;
        place(level, placePos, facing, mBlock, hingeRight);
        const SoundType* soundType = mBlock->getSoundType();
        player->awardStat(Stats::blockPlacedStats.data[Block::getId(mBlock)],
                          GenericStats::param_blocksPlaced(mBlock->getBlockState(itemInstance->getAuxValue()), 1));
        level->playSound(player, placePos, soundType->getPlaceSound(), SoundSource::eSoundSource_Blocks,
                         (soundType->getVolume() + 1.0f) * 0.5f, soundType->getPitch() * 0.8f, 16.0f);
        itemInstance->shrink(1);
    }
    return InteractionResult::eInteractionResult_Success;
}
