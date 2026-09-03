#include "game/core/CombatRules.hpp"

static float clampValue(float value, float minimum, float maximum)
{
    if (value < minimum)
    {
        return minimum;
    }

    if (value > maximum)
    {
        return maximum;
    }

    return value;
}

double CombatRules::getDamageAfterAbsorb(float damage, float armorValue, float armorToughness)
{
    float toughnessScale = armorToughness / 4.0f + 2.0f;
    float effectiveArmor = clampValue(armorValue - damage / toughnessScale, armorValue * 0.2f, 20.0f);
    return (double)(float)(damage * (1.0f - effectiveArmor / 25.0f));
}

double CombatRules::getDamageAfterMagicAbsorb(float damage, float enchantModifier)
{
    float effectiveModifier = clampValue(enchantModifier, 0.0f, 20.0f);
    return (double)(float)(damage * (1.0f - effectiveModifier / 25.0f));
}
