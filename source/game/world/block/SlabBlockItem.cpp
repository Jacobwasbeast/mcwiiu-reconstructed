#include "game/world/block/SlabBlockItem.hpp"

#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/Direction.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

SlabBlockItem::SlabBlockItem(Block* block, HalfSlabBlock* halfSlab, HalfSlabBlock* fullSlab)
    : BlockItem(block), mHalfSlab(halfSlab), mFullSlab(fullSlab)
{
    setMaxDamage(0);
    setStackedByData(true);
}

SlabBlockItem::~SlabBlockItem()
{
}

int SlabBlockItem::getLevelDataForAuxValue(int auxValue)
{
    return auxValue;
}

int SlabBlockItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    return mHalfSlab->getAuxName(itemInstance->getAuxValue());
}

const BlockState* SlabBlockItem::makeState(const Property* property, Boxed* value)
{
    return mFullSlab->defaultBlockState()->setBoxedValue(property, value);
}

bool SlabBlockItem::tryConvertTargetBlock(boost::shared_ptr<Player> player,
                                          not_null_ptr<ItemInstance> itemInstance,
                                          Level* level,
                                          const BlockPos& pos,
                                          Boxed* type,
                                          bool simulate)
{
    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() != mHalfSlab)
    {
        return false;
    }

    Property* property = mHalfSlab->getTypeProperty();
    Boxed* currentType = state->getBoxedValue(property);
    if (currentType == NULL || !(*currentType == type))
    {
        return false;
    }

    if (!simulate)
    {
        const BlockState* fullState = makeState(property, currentType);
        const AABB* box = mFullSlab->getClipAABB(fullState, level, pos);
        if (!level->isUnobstructed(box) || !level->setBlock(pos, fullState, 3, false))
        {
            return false;
        }

        const SoundType* soundType = mFullSlab->getSoundType();
        level->playSound(player, pos, soundType->getPlaceSound(),
                         SoundSource::eSoundSource_Blocks,
                         (soundType->getVolume() + 1.0f) * 0.5f,
                         soundType->getPitch() * 0.8f, 16.0f);
        itemInstance->shrink(1);
    }

    return true;
}

InteractionResult::EInteractionResult SlabBlockItem::useOn(boost::shared_ptr<Player> player,
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
    if (itemInstance->isEmpty() || !player->mayUseItemAt(pos, direction, itemInstance))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    Boxed* type = mHalfSlab->getTypeForItem(itemInstance);

    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() == mHalfSlab)
    {
        TypedBoxed<HalfSlabBlock::Half*>* half =
            static_cast<TypedBoxed<HalfSlabBlock::Half*>*>(state->getBoxedValue(HalfSlabBlock::TYPE));
        if (half != NULL && ((direction == Direction::UP && half->getValue() == HalfSlabBlock::BOTTOM)
                              || (direction == Direction::DOWN && half->getValue() == HalfSlabBlock::TOP)))
        {
            tryConvertTargetBlock(player, itemInstance, level, pos, type, simulate);
            delete type;
            return InteractionResult::eInteractionResult_Success;
        }
    }

    if (tryConvertTargetBlock(player, itemInstance, level, pos.relative(direction), type, simulate))
    {
        delete type;
        return InteractionResult::eInteractionResult_Success;
    }

    delete type;
    return BlockItem::useOn(player, level, pos, hand, direction, hitX, hitY, hitZ, simulate);
}

bool SlabBlockItem::mayPlace(Level* level, const BlockPos& pos, const Direction* direction,
                             boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> itemInstance)
{
    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() == mHalfSlab && (direction == Direction::UP || direction == Direction::DOWN))
    {
        Property* property = mHalfSlab->getTypeProperty();
        TypedBoxed<HalfSlabBlock::Half*>* half =
            static_cast<TypedBoxed<HalfSlabBlock::Half*>*>(state->getBoxedValue(property));
        if (half != NULL && ((direction == Direction::UP && half->getValue() == HalfSlabBlock::BOTTOM)
                              || (direction == Direction::DOWN && half->getValue() == HalfSlabBlock::TOP)))
        {
            Boxed* type = mHalfSlab->getTypeForItem(itemInstance);
            bool result = *state->getBoxedValue(property) == type;
            return result;
        }
    }

    return BlockItem::mayPlace(level, pos, direction, player, itemInstance);
}
