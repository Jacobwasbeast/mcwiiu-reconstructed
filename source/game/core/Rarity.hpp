#ifndef GAME_CORE_RARITY_HPP
#define GAME_CORE_RARITY_HPP

#include <string>

#include "game/core/ColourTable.hpp"

class Rarity
{
public:
    Rarity(eMinecraftColour colour, const std::wstring& name, int weight);

    int getWeight() const;

    static const Rarity* COMMON;
    static const Rarity* UNCOMMON;
    static const Rarity* RARE;
    static const Rarity* EPIC;

private:
    eMinecraftColour colour;
    std::wstring name;
    int weight;
};

#endif
