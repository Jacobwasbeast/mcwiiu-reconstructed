#include "game/item/EndCrystalItem.hpp"

#include <vector>

#include "game/core/EnderCrystal.hpp"
#include "game/entity/mob/EndDragonFight.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/TheEndDimension.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Player.hpp"

EndCrystalItem::EndCrystalItem()
    : Item()
{
    setIconName(std::wstring(L"end_crystal"));
}

EndCrystalItem::~EndCrystalItem()
{
}

InteractionResult::EInteractionResult EndCrystalItem::useOn(boost::shared_ptr<Player> player,
                                                             Level* level,
                                                             const BlockPos& pos,
                                                             InteractionHand::EInteractionHand hand,
                                                             const Direction* direction,
                                                             float hitX,
                                                             float hitY,
                                                             float hitZ,
                                                             bool simulate)
{
    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();
    if (block != Blocks::obsidian && block != Blocks::bedrock)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    BlockPos abovePos = pos.above();
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (!player->mayUseItemAt(abovePos, direction, itemInstance))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    BlockPos aboveAbovePos = abovePos.above();
    bool blocked = !level->isEmptyBlock(abovePos)
        && !level->getBlockState(abovePos)->getBlock()->mayReplaceWithPlace(level, abovePos);
    bool blockedAbove = !level->isEmptyBlock(aboveAbovePos)
        && !level->getBlockState(aboveAbovePos)->getBlock()->mayReplaceWithPlace(level, aboveAbovePos);
    if (blocked || blockedAbove)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntities(
        boost::shared_ptr<Entity>(),
        AABB::newTemp(static_cast<double>(pos.x), static_cast<double>(pos.y) + 1.0,
                      static_cast<double>(pos.z), static_cast<double>(pos.x) + 1.0,
                      static_cast<double>(pos.y) + 3.0, static_cast<double>(pos.z) + 1.0),
        NULL);
    if (entities->size() != 0)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (simulate)
    {
        return InteractionResult::eInteractionResult_Success;
    }

    if (!level->mIsClientSide)
    {
        if (!level->canCreateMore(0x000000000800000EULL, 0))
        {
            player->displayClientMessage(static_cast<int32_t>(0xC952E8B9), true);
            return InteractionResult::eInteractionResult_Fail;
        }

        boost::shared_ptr<EnderCrystal> crystal(new EnderCrystal(
            level,
            static_cast<double>(pos.x) + 0.5,
            static_cast<double>(pos.y) + 1.0,
            static_cast<double>(pos.z) + 0.5));
        crystal->setShowBottom(false);
        level->addEntity(crystal);

        TheEndDimension* dimension = dynamic_cast<TheEndDimension*>(level->mDimension);
        if (dimension != NULL)
        {
            dimension->getDragonFight()->tryRespawn();
        }
    }

    itemInstance->shrink(1);
    return InteractionResult::eInteractionResult_Success;
}

bool EndCrystalItem::isFoil(not_null_ptr<ItemInstance> itemInstance)
{
    return true;
}
