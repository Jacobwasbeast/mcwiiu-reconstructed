#include "game/entity/mob/HorseChest.hpp"

HorseChest::HorseChest(const std::wstring& name, int size)
    : SimpleContainer(name, size)
{
}

HorseChest::HorseChest(int id, const std::wstring& name, bool hasCustomName, int size)
    : SimpleContainer(id, name, hasCustomName, size)
{
}

HorseChest::~HorseChest()
{
}
