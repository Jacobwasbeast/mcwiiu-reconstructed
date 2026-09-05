#ifndef GAME_CORE_ORERECIPIES_HPP
#define GAME_CORE_ORERECIPIES_HPP

#include <vector>

class Block;
class Recipes;

class OreRecipies
{
public:
    void _init();
    void addRecipes(Recipes* recipes);

private:
    std::vector<void*> blockPairs[9];
};

#endif
