#include "game/item/EnchantmentContainer.hpp"

class EnchantmentMenu
{
public:
    virtual void slotsChanged();
};

EnchantmentContainer::EnchantmentContainer(EnchantmentMenu* menu)
    : SimpleContainer(0xf4bd500f, std::wstring(L""), false, 2),
      mMenu(menu)
{
}

EnchantmentContainer::~EnchantmentContainer()
{
}

int EnchantmentContainer::getMaxStackSize()
{
    return 64;
}

void EnchantmentContainer::setChanged()
{
    SimpleContainer::setChanged();
    mMenu->slotsChanged();
}
