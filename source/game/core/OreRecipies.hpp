#ifndef GAME_CORE_ORERECIPIES_HPP
#define GAME_CORE_ORERECIPIES_HPP

#include <vector>

class Recipes;

class OreRecipies
{
private:
    struct Object
    {
        Object(void* object, int auxValue, int flags)
            : object(object), auxValue(auxValue), flags(flags)
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
    std::vector<Object*> blockPairs[9];
};

#endif
