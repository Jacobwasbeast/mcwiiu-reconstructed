#ifndef GAME_CORE_GAMERULES_FLOATATTRIBUTE_HPP
#define GAME_CORE_GAMERULES_FLOATATTRIBUTE_HPP

#include "game/core/IGameRuleAttribute.hpp"
#include "game/data/GameRuleSaveInterface.hpp"

#include <string>

class GameRules_FloatAttribute : public IGameRuleAttribute
{
public:
    GameRules_FloatAttribute(float* value);
    virtual ~GameRules_FloatAttribute();

    virtual void setValue(const std::wstring& value);
    virtual void save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface);
    virtual void writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml);

    void setMin(float min);
    void setMax(float max);

private:
    float* mValue;
    float mMin;
    float mMax;
};

#endif
