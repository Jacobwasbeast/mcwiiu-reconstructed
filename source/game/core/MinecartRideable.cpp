#include "game/core/MinecartRideable.hpp"

#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/Level.hpp"

MinecartRideable::MinecartRideable(Level* level) : Minecart(level)
{
    fjDerivedCtorCalls();
    mCategories |= 0x100;
}

MinecartRideable::MinecartRideable(Level* level, double x, double y, double z)
    : Minecart(level, x, y, z)
{
    fjDerivedCtorCalls();
    mCategories |= 0x100;
}

MinecartRideable::~MinecartRideable()
{
}

void MinecartRideable::create(Level* level)
{
    new MinecartRideable(level);
}

void MinecartRideable::addDataWalkers(DataFixerUpper* dataFixerUpper)
{
}

bool MinecartRideable::interact(const boost::shared_ptr<Player>& player,
                                InteractionHand::EInteractionHand hand)
{
    Entity* playerEntity = (Entity*)player.get();
    if (playerEntity->isVehicle())
    {
        return false;
    }

    if (!mLevel->mIsClientSide)
    {
        playerEntity->startRiding(shared_from_this());
    }

    return true;
}

void MinecartRideable::activateMinecart(int x, int y, int z, bool powered)
{
    if (powered)
    {
        if (isVehicle())
        {
            ejectPassengers();
        }

        if (getHurtTime() == 0)
        {
            setHurtDir(-getHurtDir());
            setHurtTime(10);
            setDamage(50.0f);
            setDamage(50.0f);
        }
    }
}

unsigned int MinecartRideable::getType()
{
    return 0;
}

unsigned int MinecartRideable::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (!((Entity*)tooltipData.m_pPlayer.get())->isVehicle() && getPassengers()->empty())
    {
        return 0x257433afU;
    }

    return 0xffffffffU;
}

unsigned long long MinecartRideable::GetType()
{
    return ENTITY_TYPE_MINECART_RIDEABLE;
}
