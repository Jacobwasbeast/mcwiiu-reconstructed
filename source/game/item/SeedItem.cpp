#include "game/item/SeedItem.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

class LivingEntity
{
public:
    virtual not_null_ptr<ItemInstance> getItemInHand(InteractionHand::EInteractionHand hand);
};

class Player : public LivingEntity
{
public:
    virtual bool mayUseItemAt(const BlockPos& pos, const Direction* direction,
                              not_null_ptr<ItemInstance> itemInstance);
};

class Level
{
public:
    const BlockState* getBlockState(const BlockPos& pos);
    bool isEmptyBlock(const BlockPos& pos);
    bool setBlock(const BlockPos& pos, const BlockState* state, int flags, bool forceUpdate);
};

SeedItem::SeedItem(Block* cropBlock, Block* soilBlock)
    : Item(), mCropBlock(cropBlock), mSoilBlock(soilBlock)
{
}

SeedItem::~SeedItem()
{
}

InteractionResult::EInteractionResult SeedItem::useOn(boost::shared_ptr<Player> player,
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
    if (hand == InteractionHand::eInteractionHand_MainHand
        && player->mayUseItemAt(pos.relative(direction), direction, itemInstance)
        && level->getBlockState(pos)->getBlock() == mSoilBlock
        && level->isEmptyBlock(pos.above()))
    {
        if (!simulate)
        {
            level->setBlock(pos.above(), mCropBlock->defaultBlockState(), 11, false);
            itemInstance->shrink(1);
        }
        return InteractionResult::eInteractionResult_Success;
    }
    return InteractionResult::eInteractionResult_Fail;
}
