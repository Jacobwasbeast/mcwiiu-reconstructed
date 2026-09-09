#include "game/core/FoodConstants.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"

uint32_t FoodConstants::MaxFood()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getMaxFood(20);
    }
    return 20;
}

double FoodConstants::StartSaturation()
{
    const double value = 5.0;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getStartSaturation((float)value);
    }
    return value;
}

double FoodConstants::ExhaustionHeal()
{
    const double value = 6.0;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionHeal((float)value);
    }
    return value;
}

double FoodConstants::ExhaustionDrop()
{
    const double value = 4.0;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionDrop((float)value);
    }
    return value;
}

uint32_t FoodConstants::HealthTickCount()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getHealthTickCount(80);
    }
    return 80;
}

uint32_t FoodConstants::HealthTickCountSaturated()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getHealthTickCountSaturated(10);
    }
    return 10;
}

double FoodConstants::MaxSaturation()
{
    const double value = 20.0;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getMaxSaturation((float)value);
    }
    return value;
}

uint32_t FoodConstants::HealLevel()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getHealLevel(18);
    }
    return 18;
}

uint32_t FoodConstants::StarveLevel()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getStarveLevel(0);
    }
    return 0;
}

uint32_t FoodConstants::SprintLevel()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getSprintLevel(6);
    }
    return 6;
}

double FoodConstants::ExhaustionJump()
{
    const double value = 0.05;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionJump((float)value);
    }
    return value;
}

double FoodConstants::ExhaustionSprintJump()
{
    const double value = 0.2;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionSprintJump((float)value);
    }
    return value;
}

double FoodConstants::ExhaustionMine()
{
    const double value = 0.005;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionMine((float)value);
    }
    return value;
}

double FoodConstants::ExhaustionAttack()
{
    const double value = 0.1;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionAttack((float)value);
    }
    return value;
}

double FoodConstants::ExhaustionWalk()
{
    const double value = 0.0;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionWalk((float)value);
    }
    return value;
}

double FoodConstants::ExhaustionSneak()
{
    const double value = 0.0;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionSneak((float)value);
    }
    return value;
}

double FoodConstants::ExhaustionSprint()
{
    const double value = 0.1;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionSprint((float)value);
    }
    return value;
}

double FoodConstants::ExhaustionSwim()
{
    const double value = 0.01;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionSwim((float)value);
    }
    return value;
}

double FoodConstants::ExhaustionIdle()
{
    const double value = 0.0;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionIdle((float)value);
    }
    return value;
}

double FoodConstants::SaturationFloor()
{
    const double value = 0.0;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getSaturationFloor((float)value);
    }
    return value;
}

double FoodConstants::ExhaustionDamage()
{
    const double value = 0.1;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionDamage((float)value);
    }
    return value;
}
