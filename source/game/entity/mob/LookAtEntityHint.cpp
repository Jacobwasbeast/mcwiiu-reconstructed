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

    if (type == EntityType::ENDER_CRYSTAL)
    {
        return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::endCrystal));
    }
    if (type == EntityType::ARMOR_STAND)
    {
        return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::armorStand));
    }
    if (type == EntityType::BOAT)
    {
        boost::shared_ptr<Boat> boat = boost::static_pointer_cast<Boat>(entity);
        Item* dropItem = boat->getDropItem();
        return boost::shared_ptr<ItemInstance>(new ItemInstance(dropItem));
    }
    if (type == EntityType::MINECART_TNT)
    {
        return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::tntMinecart));
    }
    if (type == EntityType::MINECART_HOPPER)
    {
        return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::hopperMinecart));
    }
    if (type == EntityType::MINECART_FURNACE)
    {
        return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::furnaceMinecart));
    }
    if (type == EntityType::MINECART_CHEST)
    {
        return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::chestMinecart));
    }
    if (type == EntityType::MINECART_RIDEABLE)
    {
        return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::minecart));
    }
    if (type == EntityType::LEASH_FENCE_KNOT_ENTITY)
    {
        return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::lead));
    }
    if (type == EntityType::ITEM_FRAME)
    {
        return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::itemFrame));
    }
    if (type == EntityType::PAINTING)
    {
        return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::painting));
    }

    return ItemInstance::EMPTY;
}

unsigned int LookAtEntityHint::onLookAtEntity(boost::shared_ptr<Entity> entity)
{
    bool matches = false;
    if (entityType == entity->GetType())
    {
        if (entityPredicate == NULL || entityPredicate(entity))
        {
            matches = true;
        }
    }

    if (matches)
    {
        Tutorial::PopupMessageDetails* details = new Tutorial::PopupMessageDetails();
        unsigned long long type = entity->GetType();
        if ((type & EntityType::HANGING_ENTITY) != EntityType::HANGING_ENTITY &&
            (type & EntityType::MINECART) != EntityType::MINECART &&
            entity->GetType() != EntityType::BOAT &&
            entity->GetType() != EntityType::ARMOR_STAND &&
            entity->GetType() != EntityType::ENDER_CRYSTAL)
        {
            details->titleId = descriptionId;
            details->promptId = entityId;
            details->isItem = true;
            return tutorial->setMessage(this, details);
        }

        boost::shared_ptr<ItemInstance> item = GetEntityAsItem(entity);
        if (!item->isEmpty())
        {
            details->titleId = item->getUseDescriptionId();
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
    }
    return 0;
}
