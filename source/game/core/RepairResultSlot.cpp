#include "game/core/RepairResultSlot.hpp"

#include "game/ui/AnvilMenu.hpp"
#include "game/item/ItemInstance.hpp"

RepairResultSlot::RepairResultSlot(AnvilMenu* menu, const BlockPos& position,
                                   boost::shared_ptr<Container> container, int index, int x, int y)
    : Slot(container, index, x, y),
      mMenu(menu),
      mPosition(position)
{
}

RepairResultSlot::~RepairResultSlot()
{
}

bool RepairResultSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    return false;
}

bool RepairResultSlot::mayPickup(boost::shared_ptr<Player> player)
{
    if (mMenu->mRepairCost > 0 && !player->mAbilities.mInstabuild &&
        player->mExperienceLevel < mMenu->mRepairCost)
    {
        return false;
    }

    return mMenu->mRepairCost > 0 && hasItem();
}

void RepairResultSlot::onTake(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item)
{
    if (!player->mAbilities.mInstabuild)
    {
        player->giveExperienceLevels(-mMenu->mRepairCost);
    }

    if (mMenu->mMaterialCost < 1)
    {
        mMenu->mInputSlots->removeItem(0, 1);
    }
    else
    {
        not_null_ptr<ItemInstance> material = mMenu->mInputSlots->getItem(1);
        if (!material->isEmpty())
        {
            if (material->getCount() <= mMenu->mMaterialCost)
            {
                mMenu->mInputSlots->removeItem(1, material->getCount());
            }
            else
            {
                material->shrink(mMenu->mMaterialCost);
                mMenu->mInputSlots->setItem(1, material);
            }
        }
    }

    mMenu->mRepairCost = 0;
    mMenu->createResult();
    player->OnTakeFromAnvil(item);
}

bool RepairResultSlot::mayCombine(not_null_ptr<ItemInstance> item)
{
    return false;
}
