#ifndef GAME_CORE_FOODCONSTANTS_HPP
#define GAME_CORE_FOODCONSTANTS_HPP

#include <stdint.h>

class FoodConstants
{
public:
    static uint32_t MaxFood();
    static double StartSaturation();
    static double ExhaustionHeal();
    static double ExhaustionDrop();
    static uint32_t HealthTickCount();
    static uint32_t HealthTickCountSaturated();
    static double MaxSaturation();
    static uint32_t HealLevel();
    static uint32_t StarveLevel();
    static uint32_t SprintLevel();
    static double ExhaustionJump();
    static double ExhaustionSprintJump();
    static double ExhaustionMine();
    static double ExhaustionAttack();
    static double ExhaustionWalk();
    static double ExhaustionSneak();
    static double ExhaustionSprint();
    static double ExhaustionSwim();
    static double ExhaustionIdle();
    static double SaturationFloor();
    static double ExhaustionDamage();
};

#endif
