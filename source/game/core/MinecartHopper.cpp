#include "game/core/MinecartHopper.hpp"

#include <typeinfo>

#include <boost/pointer_cast.hpp>

#include "game/core/DamageSource.hpp"
#include "game/core/GameRules.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/ai/AliveEntitySelector.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/HopperMenu.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/HopperBlockEntity.hpp"
#include "game/world/generation/Player.hpp"

MinecartHopper::MinecartHopper(Level* level)
    : MinecartContainer(level), Hopper()
{
    _init();
}

MinecartHopper::MinecartHopper(Level* level, double x, double y, double z)
    : MinecartContainer(level, x, y, z), Hopper()
{
    _init();
}

MinecartHopper::~MinecartHopper()
{
}

void MinecartHopper::create(Level* level)
{
    new MinecartHopper(level);
}

unsigned int MinecartHopper::getContainerSize()
{
    return 5;
}

not_null_ptr<ItemInstance> MinecartHopper::getItem(unsigned int slot)
{
    return MinecartContainer::getItem(slot);
}

not_null_ptr<ItemInstance> MinecartHopper::removeItem(unsigned int slot, int count)
{
    return MinecartContainer::removeItem(slot, count);
}

not_null_ptr<ItemInstance> MinecartHopper::removeItemNoUpdate(int slot)
{
    return MinecartContainer::removeItemNoUpdate(slot);
}

void MinecartHopper::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    MinecartContainer::setItem(slot, item);
}

int MinecartHopper::getMaxStackSize()
{
    return MinecartContainer::getMaxStackSize();
}

void MinecartHopper::setChanged()
{
}

bool MinecartHopper::stillValid(boost::shared_ptr<Player> player)
{
    return MinecartContainer::stillValid(player);
}

void MinecartHopper::startOpen(boost::shared_ptr<Player> player)
{
}

void MinecartHopper::stopOpen(boost::shared_ptr<Player> player)
{
}

bool MinecartHopper::canPlaceItem(int slot, not_null_ptr<ItemInstance> item)
{
    return MinecartContainer::canPlaceItem(slot, item);
}

AbstractContainerMenu* MinecartHopper::createMenu(boost::shared_ptr<Inventory> inventory,
                                                   boost::shared_ptr<Player> player)
{
    unpackLootTable(player);
    boost::shared_ptr<Container> playerInventory = boost::dynamic_pointer_cast<Container>(inventory);
    boost::shared_ptr<Entity> entity = shared_from_this();
    boost::shared_ptr<MinecartHopper> hopper =
        boost::static_pointer_cast<MinecartHopper>(entity);
    boost::shared_ptr<Container> container = boost::static_pointer_cast<Container>(hopper);
    return new HopperMenu(playerInventory, container, player);
}

int MinecartHopper::getTypeId(bool large)
{
    return 0;
}

Level* MinecartHopper::getLevel()
{
    return mLevel;
}

double MinecartHopper::getLevelX()
{
    return mX;
}

double MinecartHopper::getLevelY()
{
    return mPosition.y + 0.5;
}

double MinecartHopper::getLevelZ()
{
    return mZ;
}

void MinecartHopper::_init()
{
    fjDerivedCtorCalls();
    mPosition = BlockPos::ZERO;
    mEnabled = true;
    mCooldown = -1;
}

void MinecartHopper::addDataWalkers(DataFixerUpper* fixer)
{
    MinecartContainer::addDataWalkers(fixer, 0x0A84000500000000ULL);
}

const Minecart::Type* MinecartHopper::getType()
{
    return Minecart::Type::HOPPER;
}

const BlockState* MinecartHopper::getDefaultDisplayBlockState()
{
    return Blocks::hopper->defaultBlockState();
}

int MinecartHopper::getDefaultDisplayOffset()
{
    return 1;
}

bool MinecartHopper::interact(const boost::shared_ptr<Player>& player,
                              InteractionHand::EInteractionHand hand)
{
    if (!player->isAllowedToInteract(shared_from_this()))
        return false;
    if (!mLevel->mIsClientSide)
    {
        boost::shared_ptr<MinecartHopper> hopper =
            boost::static_pointer_cast<MinecartHopper>(shared_from_this());
        player->openContainer(boost::static_pointer_cast<Container>(hopper));
    }
    return true;
}

void MinecartHopper::activateMinecart(int x, int y, int z, bool powered)
{
    bool enabled = !powered;
    if (isEnabled() != enabled)
        setEnabled(enabled);
}

bool MinecartHopper::isEnabled()
{
    return mEnabled;
}

void MinecartHopper::setEnabled(bool enabled)
{
    mEnabled = enabled;
}

void MinecartHopper::setCooldown(int cooldown)
{
    mCooldown = cooldown;
}

bool MinecartHopper::isOnCooldown()
{
    return mCooldown > 0;
}

void MinecartHopper::tick()
{
    Minecart::tick();
    if (!mLevel->mIsClientSide && isAlive() && isEnabled())
    {
        BlockPos position(shared_from_this());
        if (position != mPosition)
            setCooldown(0);
        else
            --mCooldown;
        if (!isOnCooldown())
        {
            if (suckInItems())
                setCooldown(4);
        }
    }
}

bool MinecartHopper::suckInItems()
{
    if (HopperBlockEntity::suckInItems(this))
        return true;
    AliveEntitySelector selector;
    std::vector<boost::shared_ptr<Entity> >* entities =
        mLevel->getEntitiesOfClass(typeid(ItemEntity), mBoundingBox->grow(0.5, 0.0, 0.5),
                                   &selector, false);
    if (entities != NULL && !entities->empty())
    {
        boost::shared_ptr<ItemEntity> item =
        boost::static_pointer_cast<ItemEntity>(entities->at(0));
        return HopperBlockEntity::addItem(NULL, this, item);
    }
    return false;
}

void MinecartHopper::destroy(const DamageSource* source)
{
    MinecartContainer::destroy(source);
    if (mLevel->getGameRules()->getBoolean(6))
        spawnAtLocation(Item::byBlock(Blocks::hopper), 1, 4.5f);
}

void MinecartHopper::addAdditonalSaveData(CompoundTag* tag)
{
    MinecartContainer::addAdditonalSaveData(tag);
    tag->putInt(L"TransferCooldown", mCooldown);
    tag->putBoolean(L"Enabled", mEnabled);
}

void MinecartHopper::readAdditionalSaveData(CompoundTag* tag)
{
    MinecartContainer::readAdditionalSaveData(tag);
    mCooldown = tag->getInt(L"TransferCooldown");
    mEnabled = tag->contains(L"Enabled") ? tag->getBoolean(L"Enabled") : true;
}

unsigned long long MinecartHopper::GetType()
{
    return 0x000000000A840005ULL;
}
