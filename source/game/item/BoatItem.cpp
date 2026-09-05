#include "game/item/BoatItem.hpp"

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/Boat.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/InteractionResult.hpp"
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
    HitResult* hit = getPlayerPOVHitResult(level, player, false);
    if (hit == NULL)
    {
        return false;
    }

    const BlockState* state = level->getBlockState(hit->getBlockPos());
    if (state == NULL)
    {
        return false;
    }

    Material* material = state->getMaterial();
    double y = hit->mLocation->y;
    if (material == Material::WATER || material == Material::BUBBLE_COLUMN)
    {
        y -= 0.12;
    }

    boost::shared_ptr<Boat> boat(new Boat(level, hit->mLocation->x, y, hit->mLocation->z));
    boat->setType(mType);
    AABB* box = boat->getCollideBox()->grow(0.1);
    std::vector<AABB*>* collisions = level->getCollisionAABBs(boat, box, false, false, false);
    return collisions == NULL || collisions->empty();
}

InteractionResultHolder BoatItem::use(Level* level,
                                      boost::shared_ptr<Player> player,
                                      InteractionHand::EInteractionHand hand)
{
    HitResult* hit = getPlayerPOVHitResult(level, player, false);
    if (hit == NULL)
    {
        return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, ItemInstance::EMPTY);
    }

    const BlockState* state = level->getBlockState(hit->getBlockPos());
    if (state == NULL)
    {
        return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, ItemInstance::EMPTY);
    }

    Material* material = state->getMaterial();
    double y = hit->mLocation->y;
    if (material == Material::WATER || material == Material::BUBBLE_COLUMN)
    {
        y -= 0.12;
    }

    unsigned int count = 0;
    unsigned int result = 0;
    if (level->countInstanceOf(ENTITY_TYPE_BOAT, false, &count, &result) < 0 || count >= 40)
    {
        return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, ItemInstance::EMPTY);
    }

    boost::shared_ptr<Boat> boat(new Boat(level, hit->mLocation->x, y, hit->mLocation->z));
    boat->setType(mType);
    AABB* box = boat->getCollideBox()->grow(0.1);
    std::vector<AABB*>* collisions = level->getCollisionAABBs(boat, box, false, false, false);
    if (collisions != NULL && !collisions->empty())
    {
        return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, ItemInstance::EMPTY);
    }

    float rotation = Mth::wrapDegrees(player->mYRot - 180.0f);
    boat->moveTo(boat->mX, boat->mY, boat->mZ, rotation, 0.0f);
    if (!level->mIsClientSide)
    {
        level->addEntity(boat);
    }

    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, ItemInstance::EMPTY);
}

unsigned int BoatItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return TestUse(tooltipData.m_pLevel, tooltipData.m_pPlayer,
                   InteractionHand::eInteractionHand_MainHand) ? 0x4c36cd96u : 0xFFFFFFFFu;
}
