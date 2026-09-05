#ifndef GAME_ITEM_REPAIRCONTAINER_HPP
#define GAME_ITEM_REPAIRCONTAINER_HPP

#include "game/item/SimpleContainer.hpp"

class AnvilMenu;

class RepairContainer : public SimpleContainer
{
public:
    RepairContainer(AnvilMenu* menu, int id, bool hasCustomName, int size);
    virtual ~RepairContainer();

    virtual void setChanged();

private:
    AnvilMenu* mMenu;
};

#endif
