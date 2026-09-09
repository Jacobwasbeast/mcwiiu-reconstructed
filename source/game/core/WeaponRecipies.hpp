#ifndef GAME_CORE_WEAPONRECIPIES_HPP
#define GAME_CORE_WEAPONRECIPIES_HPP

#include <vector>

class Recipes;

class WeaponRecipies
{
private:
    struct Object
    {
        Object(void* object, int value, int auxValue, int flags)
            : object(object), value(value), auxValue(auxValue), flags(flags)
        {
        }

        void* object;
        int value;
        int auxValue;
        int flags;
    };

public:
    void _init();
    void addRecipes(Recipes* recipes);

private:
    std::vector<Object*> mIngredients;
    std::vector<Object*> mWeapons;
};

#endif
