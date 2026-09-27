#include "game/entity/mob/LookAtEntityHint.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Boat.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/minigame/Tutorial.hpp"

LookAtEntityHint::LookAtEntityHint(eTutorial_Hint hint, Tutorial* tutorialValue,
                                   int description, int id,
                                   unsigned long long typeValue)
    : TutorialHint(hint, tutorialValue, description, HINT_LOOK_AT_ENTITY, true, true, -1),
      entityType(typeValue), entityId(id), entityPredicate(NULL)
{
}

LookAtEntityHint::~LookAtEntityHint()
{
}

boost::shared_ptr<ItemInstance> LookAtEntityHint::GetEntityAsItem(boost::shared_ptr<Entity> entity)
{
    unsigned long long type = entity->GetType();
    Item* item = NULL;

    if (type == EntityType::ENDER_CRYSTAL)
    {
        item = Items::endCrystal;
    }
    else if (type == EntityType::ARMOR_STAND)
    {
        item = Items::armorStand;
    }
    else if (type == EntityType::BOAT)
    {
        item = boost::static_pointer_cast<Boat>(entity)->getDropItem();
    }
    else if (type == EntityType::MINECART_TNT)
    {
        item = Items::tntMinecart;
    }
    else if (type == EntityType::MINECART_HOPPER)
    {
        item = Items::hopperMinecart;
    }
    else if (type == EntityType::MINECART_FURNACE)
    {
        item = Items::furnaceMinecart;
    }
    else if (type == EntityType::MINECART_CHEST)
    {
        item = Items::chestMinecart;
    }
    else if (type == EntityType::MINECART_RIDEABLE)
    {
        item = Items::minecart;
    }
    else if (type == EntityType::LEASH_FENCE_KNOT_ENTITY)
    {
        item = Items::lead;
    }
    else if (type == EntityType::ITEM_FRAME)
    {
        item = Items::itemFrame;
    }
    else if (type == EntityType::PAINTING)
    {
        item = Items::painting;
    }
    else
    {
        return boost::shared_ptr<ItemInstance>();
    }

    return boost::shared_ptr<ItemInstance>(new ItemInstance(item));
}

unsigned int LookAtEntityHint::onLookAtEntity(boost::shared_ptr<Entity> entity)
{
    if (entity->GetType() != entityType)
    {
        return 0;
    }

    if (entityPredicate != NULL && !entityPredicate(entity))
    {
        return 0;
    }

    Tutorial::PopupMessageDetails* details = new Tutorial::PopupMessageDetails();
    details->messageId = descriptionId;
    details->itemId = entityId;
    details->isItem = true;

    unsigned long long type = entity->GetType();
    if ((type & EntityType::HANGING_ENTITY) != EntityType::HANGING_ENTITY &&
        (type & EntityType::MINECART) != EntityType::MINECART &&
        type != EntityType::ARMOR_STAND && type != EntityType::ENDER_CRYSTAL)
    {
        return tutorial->setMessage(this, details);
    }

    boost::shared_ptr<ItemInstance> item = GetEntityAsItem(entity);
    if (item->isEmpty())
    {
        return 0;
    }

    details->messageId = item->getUseDescriptionId();
    if (item->getItem() != NULL)
    {
        details->itemDescription = item->getHoverName();
    }
    details->itemDescriptionId = item->getDescriptionId(-1);
    details->itemInstance = item;
    details->itemId = Item::VALIDgetId(item->getItem());
    details->itemAuxValue = item->getAuxValue();
    details->isItem = true;
    return tutorial->setMessage(this, details);
}
