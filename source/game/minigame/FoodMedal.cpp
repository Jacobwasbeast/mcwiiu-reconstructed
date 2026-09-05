#include "game/minigame/FoodMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

FoodMedal::FoodMedal(int name, float target, unsigned int weight, Item* item)
    : GameMedal(name, target, weight, OPERATOR_AT_LEAST, true, false)
    , mItems(new std::vector<Item*>())
{
    mItems->push_back(item);
}

FoodMedal::FoodMedal(int name, float target, unsigned int weight, std::vector<Item*>* items)
    : GameMedal(name, target, weight, OPERATOR_AT_LEAST, true, false)
    , mItems(items)
{
}

FoodMedal::~FoodMedal()
{
    delete mItems;
}

double FoodMedal::GetScore(PlayerStats* playerStats) const
{
    const bool allFood = mItems->size() == 1 && (*mItems)[0] == NULL;
    std::vector<Item*> itemsUsed;
    playerStats->getAllItemsUsed(itemsUsed);

    float score = 0.0f;
    for (std::vector<Item*>::const_iterator it = itemsUsed.begin(); it != itemsUsed.end(); ++it)
    {
        FoodItem* foodItem = dynamic_cast<FoodItem*>(*it);
        if (foodItem != NULL)
        {
            if (allFood)
            {
                score += 1.0f;
            }
            else
            {
                for (std::vector<Item*>::const_iterator selected = mItems->begin();
                     selected != mItems->end();
                     ++selected)
                {
                    if (*selected == foodItem)
                    {
                        score += 1.0f;
                        break;
                    }
                }
            }
        }
    }

    return static_cast<double>(score);
}
