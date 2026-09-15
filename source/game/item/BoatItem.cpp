#include "game/item/BoatItem.hpp"

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/Boat.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

BoatItem::BoatItem(Boat::Type type) : Item(), mType(type)
{
    mMaxStackSize = 1;
}

BoatItem::~BoatItem()
{
}

bool BoatItem::TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    player->getItemInHand(hand);
    float xRot = player->mXRotO + (player->mXRot - player->mXRotO);
    float yRot = player->mYRotO + (player->mYRot - player->mYRotO);
    Vec3* from = Vec3::newTemp(player->mXo + (player->mX - player->mXo),
                               player->mYo + (player->mY - player->mYo) + player->getEyeHeight(),
                               player->mZo + (player->mZ - player->mZo));
    float yaw = -yRot * 0.01745329238474369f - 3.1415927410125732f;
    float pitch = -xRot * 0.01745329238474369f;
    float yawCos = Mth::cos(yaw);
    float yawSin = Mth::sin(yaw);
    float pitchCos = -Mth::cos(pitch);
    float pitchSin = Mth::sin(pitch);
    Vec3* to = from->add((double)(yawSin * pitchCos) * 5.0,
                         (double)pitchSin * 5.0,
                         (double)(yawCos * pitchCos) * 5.0);
    HitResult* hit = level->clip(from, to, true);
    if (hit == NULL)
    {
        return false;
    }
    if (hit->mType != HitResult::BLOCK)
    {
        delete hit;
        return false;
    }

    Block* block = level->getBlockState(hit->getBlockPos())->getBlock();
    bool onWater = block == Blocks::WATER || block == Blocks::FLOWING_WATER;
    boost::shared_ptr<Boat> boat(new Boat(level,
                                          hit->mLocation->x,
                                          onWater ? hit->mLocation->y - 0.12 : hit->mLocation->y,
                                          hit->mLocation->z));
    delete hit;
    boat->setType(mType);
    boat->mYRot = player->mYRot;
    return level->getCollisionAABBs(boat, boat->Entity::mBoundingBox->grow(-0.1), false, false, false)->size() == 0;
}

InteractionResultHolder BoatItem::use(Level* level,
                                      boost::shared_ptr<Player> player,
                                      InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    float xRot = player->mXRotO + (player->mXRot - player->mXRotO);
    float yRot = player->mYRotO + (player->mYRot - player->mYRotO);
    Vec3* from = Vec3::newTemp(player->mXo + (player->mX - player->mXo),
                               player->mYo + (player->mY - player->mYo) + player->getEyeHeight(),
                               player->mZo + (player->mZ - player->mZo));
    float yaw = -yRot * 0.01745329238474369f - 3.1415927410125732f;
    float pitch = -xRot * 0.01745329238474369f;
    float yawCos = Mth::cos(yaw);
    float yawSin = Mth::sin(yaw);
    float pitchCos = -Mth::cos(pitch);
    float pitchSin = Mth::sin(pitch);
    Vec3* to = from->add((double)(yawSin * pitchCos) * 5.0,
                         (double)pitchSin * 5.0,
                         (double)(yawCos * pitchCos) * 5.0);
    HitResult* hit = level->clip(from, to, true);
    if (hit == NULL)
    {
        return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
    }

    Vec3* view = player->getViewVector(1.0f);
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntities(
        player,
        player->mBoundingBox->expand(view->x * 5.0, view->y * 5.0, view->z * 5.0)->grow(1.0, 1.0, 1.0),
        NULL);
    bool blocked = false;
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Entity> entity = *it;
        if (entity->isPickable() && entity->mBoundingBox->grow(entity->getPickRadius())->contains(from))
        {
            blocked = true;
        }
    }
    if (blocked)
    {
        delete hit;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
    }

    if (hit->mType != HitResult::BLOCK)
    {
        delete hit;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
    }

    Block* block = level->getBlockState(hit->getBlockPos())->getBlock();
    bool onWater = block == Blocks::WATER || block == Blocks::FLOWING_WATER;
    if (level->countInstanceOf(ENTITY_TYPE_BOAT, true, NULL, NULL) < 40)
    {
        boost::shared_ptr<Boat> boat(new Boat(level,
                                              hit->mLocation->x,
                                              onWater ? hit->mLocation->y - 0.12 : hit->mLocation->y,
                                              hit->mLocation->z));
        boat->setType(mType);
        boat->mYRot = player->mYRot;
        if (level->getCollisionAABBs(boat, boat->Entity::mBoundingBox->grow(-0.1), false, false, false)->size() != 0)
        {
            delete hit;
            return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
        }
        if (!level->mIsClientSide)
        {
            level->addEntity(boat);
        }
        if (!player->mAbilities.mInstabuild)
        {
            itemInstance->shrink(1);
        }
        player->awardStat(gStats->get_itemsUsed(Item::VALIDgetId(this)),
                          GenericStats::param_itemsUsed(player, itemInstance));
    }
    else if (!level->mIsClientSide)
    {
        player->displayClientMessage(0xB2145884, true);
    }

    delete hit;
    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

unsigned int BoatItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return TestUse(tooltipData.m_pLevel, tooltipData.m_pPlayer, InteractionHand::eInteractionHand_MainHand)
        ? 0x4C36CD96u : 0xFFFFFFFFu;
}
