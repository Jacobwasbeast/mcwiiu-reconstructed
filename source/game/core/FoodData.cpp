#include "game/core/FoodData.hpp"

#include <algorithm>
#include <string>

#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/FoodItem.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

FoodData::FoodData()
{
    exhaustionLevel = 0.0f;
    foodTickTimer = 0;
    foodLevel = FoodConstants::MaxFood();
    lastFoodLevel = FoodConstants::MaxFood();
    saturationLevel = FoodConstants::StartSaturation();
}

void FoodData::eat(int foodLevelValue, float saturationModifier)
{
    int newFoodLevel = foodLevel + foodLevelValue;
    const int maxFood = static_cast<int>(FoodConstants::MaxFood());
    if (newFoodLevel > maxFood)
        newFoodLevel = maxFood;
    float newSaturation = saturationLevel + saturationModifier * static_cast<float>(foodLevelValue) * 2.0f;
    const float foodLevelAsFloat = static_cast<float>(newFoodLevel);
    foodLevel = newFoodLevel;
    saturationLevel = newSaturation >= foodLevelAsFloat ? foodLevelAsFloat : newSaturation;
}

void FoodData::eat(FoodItem* foodItem, const not_null_ptr<ItemInstance>& itemInstance)
{
    eat(foodItem->getNutrition(itemInstance), foodItem->getSaturationModifier(itemInstance));
}

void FoodData::addExhaustion(float exhaustion)
{
    float newExhaustion = exhaustionLevel + exhaustion;
    const float maxExhaustion = FoodConstants::MaxSaturation() * 2.0f;
    exhaustionLevel = newExhaustion >= maxExhaustion ? maxExhaustion : newExhaustion;
}

void FoodData::tick(const boost::shared_ptr<Player>& player)
{
    const Difficulty* difficulty = player->mLevel->getDifficulty();
    lastFoodLevel = foodLevel;

    if (static_cast<double>(exhaustionLevel) > FoodConstants::ExhaustionDrop())
    {
        exhaustionLevel -= FoodConstants::ExhaustionDrop();
        if (saturationLevel <= 0.0f)
        {
            if (difficulty != Difficulty::PEACEFUL)
            {
                const int reducedFoodLevel = foodLevel - 1;
                foodLevel = reducedFoodLevel < 0 ? 0 : reducedFoodLevel;
            }
        }
        else
        {
            const float reducedSaturation = saturationLevel - 1.0f;
            saturationLevel = reducedSaturation < 0.0f ? 0.0f : reducedSaturation;
        }
    }

    const bool naturalRegeneration = player->mLevel->getGameRules()->getBoolean(GameRules::RULE_NATURAL_REGENERATION);

    if (player->isAllowedToIgnoreExhaustion() && naturalRegeneration)
    {
        if (foodLevel > 0)
        {
            if (player->isHurt())
            {
                ++foodTickTimer;
                if (foodTickTimer >= static_cast<int>(FoodConstants::HealthTickCount()))
                {
                    player->heal(1.0f);
                    foodTickTimer = 0;
                    --foodLevel;
                }
            }
        }
        return;
    }

    if (!Minecraft::InMiniGame(eMiniGameId_Any, true) && naturalRegeneration && saturationLevel > 0.0f &&
        player->isHurt() && foodLevel >= static_cast<int>(FoodConstants::MaxFood()))
    {
        ++foodTickTimer;
        if (foodTickTimer >= static_cast<int>(FoodConstants::HealthTickCountSaturated()))
        {
            const float exhaustionHeal = FoodConstants::ExhaustionHeal();
            const float healAmount = saturationLevel >= exhaustionHeal ? exhaustionHeal : saturationLevel;
            player->heal(healAmount / FoodConstants::ExhaustionHeal());
            addExhaustion(healAmount);
            foodTickTimer = 0;
        }
        return;
    }

    if (player->mLevel->getGameRules()->getBoolean(GameRules::RULE_NATURAL_REGENERATION) &&
        foodLevel >= static_cast<int>(FoodConstants::HealLevel()) && player->isHurt())
    {
        ++foodTickTimer;
        if (foodTickTimer >= static_cast<int>(FoodConstants::HealthTickCount()))
        {
            player->heal(1.0f);
            addExhaustion(FoodConstants::ExhaustionHeal());
            foodTickTimer = 0;
        }
        return;
    }

    if (foodLevel <= static_cast<int>(FoodConstants::StarveLevel()))
    {
        ++foodTickTimer;
        if (foodTickTimer >= static_cast<int>(FoodConstants::HealthTickCount()))
        {
            if ((player->getHealth() > 10.0f || difficulty == Difficulty::HARD) ||
                (player->getHealth() > 1.0f && (difficulty == Difficulty::NORMAL || difficulty == Difficulty::HARD)))
            {
                if (!Minecraft::InMiniGame(eMiniGameId_Glide, true))
                    player->hurt(DamageSource::STARVE, 1.0f);
            }
            foodTickTimer = 0;
        }
    }
    else
    {
        foodTickTimer = 0;
    }
}

void FoodData::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag->contains(std::wstring(L"foodLevel"), Tag::TAG_ANY_NUMERIC))
    {
        foodLevel = tag->getInt(std::wstring(L"foodLevel"));
        foodTickTimer = tag->getInt(std::wstring(L"foodTickTimer"));
        saturationLevel = tag->getFloat(std::wstring(L"foodSaturationLevel"));
        exhaustionLevel = tag->getFloat(std::wstring(L"foodExhaustionLevel"));
    }
}

void FoodData::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putInt(std::wstring(L"foodLevel"), foodLevel);
    tag->putInt(std::wstring(L"foodTickTimer"), foodTickTimer);
    tag->putFloat(std::wstring(L"foodSaturationLevel"), saturationLevel);
    tag->putFloat(std::wstring(L"foodExhaustionLevel"), exhaustionLevel);
}

unsigned int FoodData::getFoodLevel()
{
    return foodLevel;
}

unsigned int FoodData::getLastFoodLevel()
{
    return lastFoodLevel;
}

bool FoodData::needsFood()
{
    return foodLevel < static_cast<int>(FoodConstants::MaxFood());
}

bool FoodData::isStarving()
{
    return foodLevel <= static_cast<int>(FoodConstants::StarveLevel());
}

double FoodData::getSaturationLevel()
{
    return static_cast<double>(saturationLevel);
}

void FoodData::setFoodLevel(int foodLevelValue)
{
    foodLevel = foodLevelValue;
}

void FoodData::setSaturation(float saturationLevelValue)
{
    saturationLevel = saturationLevelValue;
}

void FoodData::setExhaustion(float exhaustionLevelValue)
{
    exhaustionLevel = exhaustionLevelValue;
}
