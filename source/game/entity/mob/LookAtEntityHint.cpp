#include "game/entity/mob/LookAtEntityHint.hpp"

#include "game/core/Boat.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/minigame/Tutorial.hpp"

LookAtEntityHint::LookAtEntityHint(eTutorial_Hint hint, Tutorial* tutorialValue,
                                   int description, int id,
                                   unsigned long long typeValue)
    : TutorialHint(hint, tutorialValue, description, (eHintType)7, true, true, -1),
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

    if (type == 0x800000eULL)
    {
        item = Items::endCrystal;
    }
    else if (type == 0xc000001ULL)
    {
        item = Items::armorStand;
    }
    else if (type == 0x8000008ULL)
    {
        item = boost::static_pointer_cast<Boat>(entity)->getDropItem();
    }
    else if (type == 0xa800004ULL)
    {
        item = Items::tntMinecart;
    }
    else if (type == 0xa840005ULL)
    {
        item = Items::hopperMinecart;
    }
    else if (type == 0xa800003ULL)
    {
        item = Items::furnaceMinecart;
    }
    else if (type == 0xa840002ULL)
    {
        item = Items::chestMinecart;
    }
    else if (type == 0xa800001ULL)
    {
        item = Items::minecart;
    }
    else if (type == 0xa080003ULL)
    {
        item = Items::lead;
    }
    else if (type == 0xa080002ULL)
    {
        item = Items::itemFrame;
    }
    else if (type == 0xa080001ULL)
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
    if ((type & 0xa080000ULL) != 0xa080000ULL &&
        (type & 0xa800000ULL) != 0xa800000ULL &&
        type != 0xc000001ULL && type != 0x800000eULL)
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
