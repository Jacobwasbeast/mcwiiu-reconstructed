#include "game/core/MushroomCow.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Player.hpp"

MushroomCow::MushroomCow(Level* level)
    : Cow(level, false)
{
    fjDerivedCtorCalls();
    setSize(0.9f, 1.4f);
}

MushroomCow::~MushroomCow()
{
}

void MushroomCow::create(Level* level)
{
    new MushroomCow(level);
}

void MushroomCow::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, 0x2d6a0001ULL);
}

bool MushroomCow::mobInteract(const boost::shared_ptr<Player>& player,
                              InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    Item* item = itemInstance->getItem();

    if (item == Items::bowl && getAge() >= 0)
    {
        itemInstance->shrink(1);
        if (!itemInstance->isEmpty())
        {
            not_null_ptr<ItemInstance> stew(new ItemInstance(Items::mushroomStew));
            if (!player->mInventory->add(stew))
                player->drop(not_null_ptr<ItemInstance>(new ItemInstance(Items::mushroomStew, 1, 0)), false);
        }
        else
        {
            player->setItemInHand(hand, not_null_ptr<ItemInstance>(new ItemInstance(Items::mushroomStew)));
        }
        return true;
    }

    if (item == Items::shears && getAge() >= 0)
    {
        if (mLevel->mIsClientSide)
            return true;

        remove();

        boost::shared_ptr<Cow> cow(new Cow(mLevel, true));
        if (isPassenger())
            cow->startRiding(getVehicle(), true);
        if (isLeashed())
            cow->setLeashedTo(getLeashHolder(), true);
        mLevel->addEntity(cow);
        cow->moveTo(mX, mY, mZ, mYRotO, mXRotO);
        cow->setHealth(getHealth());
        if (hasCustomName())
            cow->setCustomName(getCustomName(), getOwnerUUID(), wasNamedByRestrictedPlayer());

        for (int i = 0; i < 5; ++i)
        {
            not_null_ptr<ItemInstance> mushroom(new ItemInstance(Blocks::red_mushroom));
            boost::shared_ptr<Entity> mushroomEntity(new ItemEntity(mLevel, mX, mY + mWalkDist, mZ, mushroom));
            mLevel->addEntity(mushroomEntity);
        }

        itemInstance->hurtAndBreak(1, player);
        playSound(SoundEvents::SHEEP_SHEAR, 1.0f, 1.75f);
        return true;
    }

    return Cow::mobInteract(player, hand);
}

bool MushroomCow::checkSpawnRules()
{
    BlockPos pos(shared_from_this());
    Block* block = mLevel->getBlockState(pos.below())->getBlock();
    if (block != Blocks::mycelium && block != Blocks::grass)
        return false;
    return mLevel->getDaytimeRawBrightness(pos) > 8 && PathfinderMob::checkSpawnRules();
}

boost::shared_ptr<AgableMob> MushroomCow::getBreedOffspring(boost::shared_ptr<AgableMob>)
{
    return boost::shared_ptr<AgableMob>(new MushroomCow(mLevel));
}

unsigned int MushroomCow::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    Item* item = tooltipData.m_pItemInstance->getItem();
    if (item == Items::bowl && !isBaby())
        return 0x735a2f66U;
    if (item == Items::shears && !isBaby())
        return 0xbe5aab60U;
    return Cow::GetInteractTooltip(tooltipData);
}

unsigned long long MushroomCow::GetType()
{
    return 0x000000002D6A0001ULL;
}
