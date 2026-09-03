#ifndef GAME_CORE_COMBATRULES_HPP
#define GAME_CORE_COMBATRULES_HPP

class CombatRules
{
public:
    static double getDamageAfterAbsorb(float damage, float armorValue, float armorToughness);
    static double getDamageAfterMagicAbsorb(float damage, float enchantModifier);
};

#endif
