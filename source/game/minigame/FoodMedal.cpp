#include "game/minigame/FoodMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

FoodMedal::FoodMedal(int name, float target, unsigned int weight, Item* item)
    : GameMedal(name, target, weight, OPERATOR_AT_LEAST, true, false)
    , mItems(new std::vector<Item*>(&item, &item + 1))
{
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

float FoodMedal::GetScore(PlayerStats* playerStats) const
{
    const bool allFood = mItems->size() == 1 && mItems->at(0) == NULL;
    std::vector<Item*> itemsUsed;
    playerStats->getAllItemsUsed(itemsUsed);

    float score = 0.0f;
    if (allFood)
    {
        for (std::vector<Item*>::const_iterator it = itemsUsed.begin(); it != itemsUsed.end(); ++it)
        {
            if (*it != NULL && dynamic_cast<FoodItem*>(*it) != NULL)
            {
                score += 1.0f;
            }
        }
    }
    else
    {
        for (std::vector<Item*>::const_iterator it = itemsUsed.begin(); it != itemsUsed.end(); ++it)
        {
            FoodItem* foodItem = NULL;
            if (*it != NULL)
            {
                foodItem = dynamic_cast<FoodItem*>(*it);
            }
            if (foodItem != NULL)
            {
                std::vector<Item*>::const_iterator selected = mItems->begin();
                while (selected != mItems->end() && *selected != foodItem)
                {
                    ++selected;
                }
                if (selected != mItems->end())
                {
                    score += 1.0f;
                }
            }
        }
    }

    return static_cast<double>(score);
}
