#include "game/core/Rarity.hpp"

Rarity::Rarity(eMinecraftColour colour, const std::wstring& name, int weight)
    : colour(colour),
      name(name),
      weight(weight)
{
}

int Rarity::getWeight() const
{
    return weight;
}
