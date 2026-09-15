#include "game/core/MinecartChest.hpp"

#include <boost/pointer_cast.hpp>

#include "game/core/DamageSource.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/Direction.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/ChestMenu.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"

MinecartChest::MinecartChest(Level* level)
    : MinecartContainer(level)
{
    fjDerivedCtorCalls();
}

MinecartChest::MinecartChest(Level* level, double x, double y, double z)
    : MinecartContainer(level, x, y, z)
{
    fjDerivedCtorCalls();
}

MinecartChest::~MinecartChest()
{
}

void MinecartChest::create(Level* level)
{
    new MinecartChest(level);
}

unsigned int MinecartChest::getContainerSize()
{
    return 27;
}

AbstractContainerMenu* MinecartChest::createMenu(boost::shared_ptr<Inventory> inventory,
                                                   boost::shared_ptr<Player> player)
{
    unpackLootTable(player);

    boost::shared_ptr<Container> playerInventory = boost::dynamic_pointer_cast<Container>(inventory);
    boost::shared_ptr<Container> container = boost::static_pointer_cast<MinecartChest>(shared_from_this());

    return new ChestMenu(playerInventory, container, player);
}

int MinecartChest::getTypeId(bool large)
{
    return (int)0xE08B6A47;
}

void MinecartChest::addDataWalkers(DataFixerUpper* fixer)
{
    MinecartContainer::addDataWalkers(fixer, 0x000000000A840002ULL);
}

unsigned int MinecartChest::getContainerType()
{
    return 16;
}

void MinecartChest::destroy(const DamageSource* source)
{
    MinecartContainer::destroy(source);

    if (mLevel->getGameRules()->getBoolean(6))
    {
        spawnAtLocation(Item::byBlock(Blocks::chest), 1, 0.0f);
    }
}

const Minecart::Type* MinecartChest::getType()
{
    return Minecart::Type::CHEST;
}

const BlockState* MinecartChest::getDefaultDisplayBlockState()
{
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    return Blocks::chest->defaultBlockState()->setBoxedValue(
        HorizontalDirectionalBlock::FACING_PROPERTY, &facing);
}

int MinecartChest::getDefaultDisplayOffset()
{
    return 8;
}

unsigned long long MinecartChest::GetType()
{
    return 0x000000000A840002ULL;
}
