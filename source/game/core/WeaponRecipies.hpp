#ifndef GAME_CORE_WEAPONRECIPIES_HPP
#define GAME_CORE_WEAPONRECIPIES_HPP

#include <vector>

class Recipes;

class WeaponRecipies
{
public:
    void _init();
    void addRecipes(Recipes* recipes);

private:
    std::vector<void*> mIngredients;
    std::vector<void*> mWeapons;
};

#endif
