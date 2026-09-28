#include "game/minigame/WeaponMedal.hpp"

#include "game/core/DamageEvent.hpp"
#include "game/item/Item.hpp"

WeaponMedal::WeaponMedal(int name, float target, unsigned int weight, Item* item)
    : GameMedal(name, target, weight, OPERATOR_AT_LEAST, true, false)
    , mItem(item)
    , mItemTypes(new std::vector<int>())
{
}

WeaponMedal::WeaponMedal(int name, float target, unsigned int weight, int itemType)
    : GameMedal(name, target, weight, OPERATOR_AT_LEAST, true, false)
    , mItem(NULL)
    , mItemTypes(new std::vector<int>())
{
    mItemTypes->push_back(itemType);
}

WeaponMedal::WeaponMedal(int name, float target, unsigned int weight, std::vector<int>* itemTypes)
    : GameMedal(name, target, weight, OPERATOR_AT_LEAST, true, false)
    , mItem(NULL)
    , mItemTypes(itemTypes)
{
}

WeaponMedal::~WeaponMedal()
{
    delete mItemTypes;
}

bool WeaponMedal::IncludeDamage(const DamageEvent* damageEvent) const
{
    Item* item = damageEvent->mWeapon.GetItem();

    if (mItem == NULL)
    {
        if (!mItemTypes->empty())
        {
            if (item != NULL)
            {
                int itemType = item->getBaseItemType();
                for (std::vector<int>::const_iterator it = mItemTypes->begin();
                     it != mItemTypes->end();
                     ++it)
                {
                    if (*it == itemType)
                    {
                        return true;
                    }
                }
            }

            return false;
        }
    }

    return item == mItem;
}
