#include "game/item/ItemStat.hpp"

ItemStat::ItemStat(int id, const std::wstring& name, int itemId)
    : Stat(id, name),
      itemId(itemId)
{
}

ItemStat::~ItemStat()
{
}

int ItemStat::getItemId()
{
    return this->itemId;
}

ItemStat* ItemStat::setExtended()
{
    Stat::setExtended();
    return this;
}
