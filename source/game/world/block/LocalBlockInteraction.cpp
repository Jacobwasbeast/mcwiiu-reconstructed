#include "game/world/block/LocalBlockInteraction.hpp"

LocalBlockInteraction::LocalBlockInteraction(int typeId, const std::wstring& name)
    : InteractionObject()
    , mTypeId(typeId)
    , mName()
{
    mName.assign(name);
}

AbstractContainerMenu* LocalBlockInteraction::createMenu(boost::shared_ptr<Inventory> inventory,
                                                          boost::shared_ptr<Player> player)
{
    return 0;
}

int LocalBlockInteraction::getTypeId(bool large)
{
    return mTypeId;
}

std::wstring LocalBlockInteraction::getName()
{
    return mName;
}

std::wstring LocalBlockInteraction::getCustomName()
{
    return getDisplayName();
}

bool LocalBlockInteraction::hasCustomName()
{
    return true;
}

std::wstring LocalBlockInteraction::getDisplayName()
{
    return mName;
}
