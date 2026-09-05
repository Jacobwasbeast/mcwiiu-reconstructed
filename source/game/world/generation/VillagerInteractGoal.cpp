#include "game/world/generation/VillagerInteractGoal.hpp"

#include <boost/shared_ptr.hpp>
#include <cmath>
#include <string>

#include "game/core/Mth.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Villager.hpp"

VillagerInteractGoal::VillagerInteractGoal(Villager* villager)
    : InteractGoal(villager, typeid(Villager), 3.0f, 0.02f), mTime(0), mVillager(villager)
{
}

VillagerInteractGoal::~VillagerInteractGoal()
{
}

void VillagerInteractGoal::start()
{
    InteractGoal::start();

    mTime = 0;
    if (mVillager->hasExcessFood())
    {
        boost::shared_ptr<Villager> villager = boost::dynamic_pointer_cast<Villager>(mLookAt);
        if (villager && villager->wantsMoreFood())
            mTime = 10;
    }
}

void VillagerInteractGoal::tick()
{
    InteractGoal::tick();

    if (mTime <= 0 || --mTime != 0)
        return;

    Inventory* inventory = mVillager->getInventory();
    for (unsigned int slot = 0; slot < inventory->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = inventory->getItem(slot);
        if (item->isEmpty())
            continue;

        Item* itemType = item->getItem();
        int count = item->getCount();
        int amount = 0;
        int resultCount = 0;

        if (itemType == Items::bread || itemType == Items::potato ||
            itemType == Items::carrot || itemType == Items::beetroot)
        {
            if (count >= 4)
            {
                amount = count / 2;
                resultCount = amount;
            }
        }
        else if (itemType == Items::wheat && count > 5)
        {
            amount = (count / 2) / 3 * 3;
            resultCount = amount / 3;
        }

        if (amount == 0)
            continue;

        item->shrink(amount);
        not_null_ptr<ItemInstance> sharedItem(new ItemInstance(itemType, resultCount, 0));
        boost::shared_ptr<ItemEntity> entity(new ItemEntity(
            mVillager->mLevel,
            mVillager->mX,
            mVillager->mY - 1.65 + mVillager->getEyeHeight(),
            mVillager->mZ,
            sharedItem));

        const float yaw = mVillager->mYRot * 0.01745329238474369f;
        const float pitch = mVillager->mXRot * 0.01745329238474369f;
        const float horizontal = Mth::cos(pitch) * 0.3f;
        entity->mXd = -static_cast<double>(Mth::sin(yaw) * horizontal);
        entity->mYd = -static_cast<double>(Mth::sin(pitch) * 0.3f) + 0.1;
        entity->mZd = static_cast<double>(Mth::cos(yaw) * horizontal);
        entity->setDefaultPickUpDelay();
        mVillager->mLevel->addEntity(entity);

        if (item->isEmpty())
            inventory->setItem(slot, ItemInstance::EMPTY);
    }
}

std::wstring VillagerInteractGoal::getName()
{
    return std::wstring(L"VillagerInteract");
}
