#include "game/world/generation/VillagerInteractGoal.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>
#include <string>

#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/SimpleContainer.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Villager.hpp"

VillagerInteractGoal::VillagerInteractGoal(Villager* villager)
    : InteractGoal(villager, typeid(Villager), 3.0f, 0.02f), mInteractionDelay(0),
      mVillager(villager)
{
}

VillagerInteractGoal::~VillagerInteractGoal()
{
}

void VillagerInteractGoal::start()
{
    InteractGoal::start();

    mInteractionDelay = 0;
    if (mVillager->hasExcessFood() && mLookAt.lock() &&
        mLookAt.lock()->GetType() == EntityType::VILLAGER)
    {
        boost::shared_ptr<Villager> villager =
            boost::dynamic_pointer_cast<Villager>(mLookAt.lock());
        if (villager->wantsMoreFood())
            mInteractionDelay = 10;
    }
}

void VillagerInteractGoal::tick()
{
    InteractGoal::tick();

    if (mInteractionDelay <= 0 || --mInteractionDelay != 0)
        return;

    SimpleContainer* inventory = mVillager->getInventory();
    for (unsigned int slot = 0; slot < inventory->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = inventory->getItem(slot);
        if (item->isEmpty())
            continue;

        Item* itemType = item->getItem();
        int count = item->getCount();
        not_null_ptr<ItemInstance> itemToThrow = ItemInstance::EMPTY;

        if (itemType == Items::bread || itemType == Items::potato ||
            itemType == Items::carrot || itemType == Items::beetroot)
        {
            if (count >= 4)
            {
                int amount = count / 2;
                item->shrink(amount);
                itemToThrow = not_null_ptr<ItemInstance>(
                    new ItemInstance(itemType, amount, item->getAuxValue()));
            }
        }
        else if (itemType == Items::wheat && count > 5)
        {
            int amount = (count / 2) / 3 * 3;
            item->shrink(amount);
            itemToThrow = not_null_ptr<ItemInstance>(
                new ItemInstance(Items::bread, amount / 3, 0));
        }

        if (item->isEmpty())
            inventory->setItem(slot, ItemInstance::EMPTY);

        if (itemToThrow->isEmpty())
            continue;

        boost::shared_ptr<ItemEntity> entity(new ItemEntity(
            mVillager->mLevel,
            mVillager->mX,
            mVillager->mY - 0.3f + mVillager->getEyeHeight(),
            mVillager->mZ,
            itemToThrow));

        const float pitch = mVillager->mXRot * 0.017453292f;
        const float yaw = mVillager->mYHeadRot * 0.017453292f;
        entity->mXd = -Mth::sin(yaw) * Mth::cos(pitch) * 0.3f;
        entity->mYd = -Mth::sin(pitch) * 0.3f + 0.1f;
        entity->mZd = Mth::cos(yaw) * Mth::cos(pitch) * 0.3f;
        entity->setDefaultPickUpDelay();
        mVillager->mLevel->addEntity(entity);
        break;
    }
}

std::wstring VillagerInteractGoal::getName()
{
    return std::wstring(L"VillagerInteract");
}
