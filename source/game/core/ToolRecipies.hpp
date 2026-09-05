#ifndef GAME_CORE_TOOLRECIPIES_HPP
#define GAME_CORE_TOOLRECIPIES_HPP

#include <vector>

class Item;
class Recipes;

class ToolRecipies
{
public:
    void _init();
    void addRecipes(Recipes* recipes);

private:
    std::vector<std::vector<Item*> > mTools;
};

#endif
