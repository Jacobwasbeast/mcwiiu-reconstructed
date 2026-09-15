#include "game/core/SpawnEggDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv.hpp"

#include <boost/shared_ptr.hpp>
#include <cmath>

#include "game/core/Direction.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DispenserBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"

not_null_ptr<ItemInstance> SpawnEggDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::execute(
    BlockSource* source, not_null_ptr<ItemInstance> item, eOUTCOME& outcome)
{
    Level* level = source->getWorld();
    const Direction* direction = source->getBlockState()->getValue(DispenserBlock::FACING);
    ResourceLocation type = SpawnEggItem::getNamedIdFrom(item);
    int error = 0;
    boost::shared_ptr<Mob> mob = SpawnEggItem::getMob(level, type,
        source->getX() + direction->getStepX(), source->getY() + direction->getStepY() + 0.2,
        source->getZ() + direction->getStepZ(), &error, L"");
    if (!mob)
    {
        outcome = eOUTCOME_Failure;
        return item;
    }

    if (mob->GetType() == ENTITY_TYPE_ENDER_DRAGON)
    {
        mob->absMoveTo(mob->mX, mob->mY + 1.0, mob->mZ);
    }
    else if (mob->GetType() == ENTITY_TYPE_WITHER_BOSS)
    {
        mob->absMoveTo(mob->mX, mob->mY - std::ceil(static_cast<double>(mob->mBbHeight)), mob->mZ);
    }

    const bool shouldInsert = (mob->GetType() & ENTITY_TYPE_MOB) == ENTITY_TYPE_MOB;
    boost::shared_ptr<Entity> entity;
    if (shouldInsert)
    {
        entity = boost::static_pointer_cast<Entity>(mob);
        level->addEntity(entity);
        mob->playAmbientSound();
        if (item->hasCustomHoverName())
        {
            mob->setCustomName(item->getHoverName(), item->getOwnerUUID(),
                               item->wasCreatedByRestrictedPlayer());
        }
    }

    boost::shared_ptr<Player> owner;
    SpawnEggItem::updateCustomEntityTag(level, owner, item, entity);
    item->shrink(1);
    outcome = eOUTCOME_Success;
    not_null_ptr<ItemInstance> result(item);
    return result;
}
