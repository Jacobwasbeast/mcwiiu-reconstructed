#include "game/item/FlintAndSteelItem.hpp"

#include "game/core/GenericStats.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/PortalBlock.hpp"

FlintAndSteelItem::FlintAndSteelItem()
    : Item()
{
    mMaterialData = 1;
    setMaxDamage(0x40);
}

FlintAndSteelItem::~FlintAndSteelItem()
{
}

InteractionResult::EInteractionResult FlintAndSteelItem::useOn(boost::shared_ptr<Player> player,
                                                                Level* level,
                                                                const BlockPos& pos,
                                                                InteractionHand::EInteractionHand hand,
                                                                const Direction* direction,
                                                                float hitX,
                                                                float hitY,
                                                                float hitZ,
                                                                bool simulate)
{
    BlockPos firePos = pos.relative(direction);
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    const BlockState* state = level->getBlockState(firePos);

    if (!simulate)
    {
        if (state->getBlock() == Blocks::AIR)
        {
            if (static_cast<PortalBlock*>(Blocks::portal)->trySpawnPortal(level, firePos, false))
            {
                player->awardStat(GenericStats::InToTheNether(), GenericStats::param_InToTheNether());
            }
            level->setBlock(firePos, Blocks::FIRE->defaultBlockState(), 0x0B, false);
        }
        itemInstance->hurtAndBreak(1, player);
        return InteractionResult::eInteractionResult_Success;
    }

    if (state->getBlock() == Blocks::AIR && Blocks::FIRE->mayPlace(level, firePos))
    {
        return InteractionResult::eInteractionResult_Success;
    }
    return InteractionResult::eInteractionResult_Fail;
}
