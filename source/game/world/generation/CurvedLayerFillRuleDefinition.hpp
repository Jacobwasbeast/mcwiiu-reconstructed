#ifndef GAME_WORLD_GENERATION_CURVEDLAYERFILLRULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_CURVEDLAYERFILLRULEDEFINITION_HPP

#include <string>

#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"

class LayerFill;

class CurvedLayerFillRuleDefinition : public LayerAttributeChildRuleDefinition<LayerFill*>
{
public:
    CurvedLayerFillRuleDefinition();
    virtual ~CurvedLayerFillRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount);
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);
    virtual LayerFill* Generate() const;

private:
    int mCurve;
    int mOffset;
    int mThickness;
};

#endif
