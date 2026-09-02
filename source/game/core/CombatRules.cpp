#include "game/core/CombatRules.hpp"

#include "game/core/Mth.hpp"

float CombatRules::getDamageAfterAbsorb(float damage, float armorValue, float armorToughness)
{
    float toughnessScale = armorToughness / 4.0f + 2.0f;
    float effectiveArmor = Mth::clamp(armorValue - damage / toughnessScale, armorValue * 0.2f, 20.0f);
    return damage * (1.0f - effectiveArmor / 25.0f);
}

float CombatRules::getDamageAfterMagicAbsorb(float damage, float enchantModifier)
{
    float effectiveModifier = Mth::clamp(enchantModifier, 0.0f, 20.0f);
    return damage * (1.0f - effectiveModifier / 25.0f);
}
