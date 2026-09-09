#include "game/item/BoatItem.hpp"

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/Boat.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Material.hpp"

BoatItem::BoatItem(Boat::Type type) : Item(), mType(type)
{
    mMaxStackSize = 1;
}

BoatItem::~BoatItem()
{
}

bool BoatItem::TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = ((LivingEntity*)(void*)player.get())->getItemInHand(hand);
    Entity* playerEntity = (Entity*)player.get();
    Vec3* from = Vec3::newTemp(playerEntity->mX,
                               playerEntity->mY + playerEntity->getEyeHeight(),
                               playerEntity->mZ);
    float yaw = -playerEntity->mYRot * 0.01745329238474369f - 3.1415927410125732f;
    float pitch = -playerEntity->mXRot * 0.01745329238474369f;
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

    const BlockState* state = level->getBlockState(hit->getBlockPos());
    if (state == NULL)
    {
        delete hit;
        return false;
    }

    Material* material = state->getMaterial();
    double y = hit->mLocation->y;
    if (material == Material::WATER || material == Material::BUBBLE_COLUMN)
    {
        y -= 0.12;
    }

    boost::shared_ptr<Boat> boat(new Boat(level, hit->mLocation->x, y, hit->mLocation->z));
    delete hit;
    boat->setType(mType);
    boat->mYRot = playerEntity->mYRot;
    AABB* box = boat->getCollideBox()->grow(-0.1);
    std::vector<AABB*>* collisions = level->getCollisionAABBs(boat, box, false, false, false);
    return collisions == NULL || collisions->empty();
}

InteractionResultHolder BoatItem::use(Level* level,
                                      boost::shared_ptr<Player> player,
                                      InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = ((LivingEntity*)(void*)player.get())->getItemInHand(hand);
    Entity* playerEntity = (Entity*)player.get();
    Vec3* from = Vec3::newTemp(playerEntity->mX,
                               playerEntity->mY + playerEntity->getEyeHeight(),
                               playerEntity->mZ);
    float yaw = -playerEntity->mYRot * 0.01745329238474369f - 3.1415927410125732f;
    float pitch = -playerEntity->mXRot * 0.01745329238474369f;
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

    Vec3* view = playerEntity->getViewVector(1.0f);
    AABB* searchBox = playerEntity->mBoundingBox
        ->expand(view->x * 5.0, view->y * 5.0, view->z * 5.0)
        ->grow(1.0, 1.0, 1.0);
    std::vector<boost::shared_ptr<Entity> > entities;
    level->getEntities((boost::shared_ptr<Entity>&)player, searchBox, &entities);
    for (std::vector<boost::shared_ptr<Entity> >::iterator entity = entities.begin();
         entity != entities.end(); ++entity)
    {
        if ((*entity)->isPickable()
            && (*entity)->mBoundingBox->grow((*entity)->getPickRadius())->contains(from))
        {
            delete hit;
            return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
        }
    }

    if (hit->mType != HitResult::BLOCK)
    {
        delete hit;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
    }

    const BlockState* state = level->getBlockState(hit->getBlockPos());
    if (state == NULL)
    {
        delete hit;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
    }

    Material* material = state->getMaterial();
    double y = hit->mLocation->y;
    if (material == Material::WATER || material == Material::BUBBLE_COLUMN)
    {
        y -= 0.12;
    }

    if (level->countInstanceOf(ENTITY_TYPE_BOAT, true, NULL, NULL) >= 40)
    {
        delete hit;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
    }

    boost::shared_ptr<Boat> boat(new Boat(level, hit->mLocation->x, y, hit->mLocation->z));
    delete hit;
    boat->setType(mType);
    boat->mYRot = playerEntity->mYRot;
    AABB* box = boat->getCollideBox()->grow(-0.1);
    std::vector<AABB*>* collisions = level->getCollisionAABBs(boat, box, false, false, false);
    if (collisions != NULL && !collisions->empty())
    {
        return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
    }

    if (!level->mIsClientSide)
    {
        level->addEntity(boat);
    }
    player->awardStat(gStats->get_itemsUsed(VALIDgetId(this)),
                      GenericStats::param_itemsUsed(player, itemInstance));
    if (!player->mAbilities.mInstabuild)
    {
        itemInstance->shrink(1);
    }

    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

unsigned int BoatItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return TestUse((Level*)tooltipData.m_pLevel,
                   (boost::shared_ptr<Player>&)tooltipData.m_pPlayer,
                   InteractionHand::eInteractionHand_MainHand) ? 0x4c36cd96u : 0xFFFFFFFFu;
}
