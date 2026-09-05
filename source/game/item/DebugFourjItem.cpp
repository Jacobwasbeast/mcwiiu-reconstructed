#include "game/item/DebugFourjItem.hpp"

#include "game/entity/mob/Mob.hpp"
#include "game/world/Level.hpp"

DebugFourjItem::DebugFourjItem()
{
}

DebugFourjItem::~DebugFourjItem()
{
}

bool DebugFourjItem::interactEnemy(not_null_ptr<ItemInstance>,
                                   boost::shared_ptr<Player> player,
                                   boost::shared_ptr<LivingEntity> target,
                                   InteractionHand::EInteractionHand)
{
    if (!player->mLevel->mIsClientSide)
    {
        boost::shared_ptr<Mob> mob = boost::dynamic_pointer_cast<Mob>(target);
        if (mob)
        {
            mob->setPersistenceRequired(!mob->isPersistenceRequired());
            return true;
        }
    }

    return false;
}

InteractionResult::EInteractionResult DebugFourjItem::useOn(boost::shared_ptr<Player>,
                                                             Level*,
                                                             const BlockPos&,
                                                             InteractionHand::EInteractionHand,
                                                             const Direction*,
                                                             float,
                                                             float,
                                                             float,
                                                             bool)
{
    return InteractionResult::eInteractionResult_Pass;
}
