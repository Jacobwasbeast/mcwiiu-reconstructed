#include "game/core/Rarity.hpp"

const Rarity* Rarity::COMMON = new Rarity(eMinecraftColour_HTMLColor_White, L"Common", 10);
const Rarity* Rarity::UNCOMMON = new Rarity(eMinecraftColour_HTMLColor_e, L"Uncommon", 5);
const Rarity* Rarity::RARE = new Rarity(eMinecraftColour_HTMLColor_b, L"Rare", 2);
const Rarity* Rarity::EPIC = new Rarity(eMinecraftColour_HTMLColor_5, L"Epic", 1);

Rarity::Rarity(eMinecraftColour colour, const std::wstring& name, int weight)
    : colour(colour),
      weight(weight)
{
    this->name = name;
}

unsigned int Rarity::getWeight() const
{
    return weight;
}
