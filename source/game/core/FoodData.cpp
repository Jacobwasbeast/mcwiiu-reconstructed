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
    : foodLevel(FoodConstants::MaxFood()), saturationLevel(static_cast<float>(FoodConstants::StartSaturation())),
      exhaustionLevel(0.0f), foodTickTimer(0), lastFoodLevel(FoodConstants::MaxFood())
{
}

void FoodData::eat(int foodLevelValue, float saturationModifier)
{
    int newFoodLevel = foodLevel + foodLevelValue;
    const int maxFood = static_cast<int>(FoodConstants::MaxFood());
    if (newFoodLevel > maxFood)
        newFoodLevel = maxFood;
    float newSaturation = saturationLevel + saturationModifier * static_cast<float>(foodLevelValue) * 2.0f;
    const float foodLevelAsFloat = static_cast<float>(newFoodLevel);
    if (newSaturation > foodLevelAsFloat)
        newSaturation = foodLevelAsFloat;
    foodLevel = newFoodLevel;
    saturationLevel = newSaturation;
}

void FoodData::eat(FoodItem* foodItem, const not_null_ptr<ItemInstance>& itemInstance)
{
    eat(foodItem->getNutrition(itemInstance), foodItem->getSaturationModifier(itemInstance));
}

void FoodData::addExhaustion(float exhaustion)
{
    exhaustionLevel += exhaustion;
    const float maxExhaustion = static_cast<float>(FoodConstants::MaxSaturation() * 2.0);
    if (exhaustionLevel >= maxExhaustion)
        exhaustionLevel = maxExhaustion;
}

void FoodData::tick(const boost::shared_ptr<Player>& player)
{
    const Difficulty* difficulty = player->mLevel->getDifficulty();
    lastFoodLevel = foodLevel;
    const double exhaustionDrop = FoodConstants::ExhaustionDrop();
    const double one = 1.0;
    const bool naturalRegeneration = player->mLevel->getGameRules()->getBoolean(7);

    if (static_cast<double>(exhaustionLevel) > exhaustionDrop)
    {
        exhaustionLevel = static_cast<float>(static_cast<double>(exhaustionLevel) - exhaustionDrop);
        if (saturationLevel <= 0.0f)
        {
            if (difficulty != Difficulty::PEACEFUL)
            {
                --foodLevel;
                if (foodLevel < 0)
                    foodLevel = 0;
            }
        }
        else
        {
            saturationLevel = static_cast<float>(static_cast<double>(saturationLevel) - one);
            if (saturationLevel < 0.0f)
                saturationLevel = 0.0f;
        }
    }

    if (player->isAllowedToIgnoreExhaustion() && naturalRegeneration)
    {
        if (foodLevel > 0 && player->isHurt())
        {
            ++foodTickTimer;
            if (foodTickTimer >= static_cast<int>(FoodConstants::HealthTickCount()))
            {
                player->heal(1.0f);
                foodTickTimer = 0;
                --foodLevel;
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
            const double healAmount = std::min(FoodConstants::ExhaustionHeal(), static_cast<double>(saturationLevel));
            player->heal(static_cast<float>(healAmount / FoodConstants::ExhaustionHeal()));
            addExhaustion(static_cast<float>(healAmount));
            foodTickTimer = 0;
        }
        return;
    }

    if (naturalRegeneration && foodLevel >= static_cast<int>(FoodConstants::HealLevel()) && player->isHurt())
    {
        ++foodTickTimer;
        if (foodTickTimer >= static_cast<int>(FoodConstants::HealthTickCount()))
        {
            player->heal(static_cast<float>(one));
            addExhaustion(static_cast<float>(FoodConstants::ExhaustionHeal()));
            foodTickTimer = 0;
        }
        return;
    }

    if (foodLevel <= static_cast<int>(FoodConstants::StarveLevel()))
    {
        ++foodTickTimer;
        if (foodTickTimer >= static_cast<int>(FoodConstants::HealthTickCount()))
        {
            const float health = player->getHealth();
            if ((health > 10.0f || difficulty == Difficulty::HARD) &&
                (health > 1.0f && (difficulty == Difficulty::NORMAL || difficulty == Difficulty::HARD)) &&
                !Minecraft::InMiniGame(eMiniGameId_Tumble, true))
                player->hurt(DamageSource::STARVE, 1.0f);
        }
    }
    foodTickTimer = 0;
}

void FoodData::readAdditionalSaveData(CompoundTag* tag)
{
    const std::wstring foodLevelName(L"foodLevel");
    if (tag->contains(foodLevelName, 99))
    {
        foodLevel = tag->getInt(foodLevelName);
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
