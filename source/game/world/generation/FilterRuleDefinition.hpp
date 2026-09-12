#ifndef GAME_WORLD_GENERATION_FILTERRULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_FILTERRULEDEFINITION_HPP

#include <string>
#include <vector>

#include "game/data/GameRuleSaveInterface.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/LayerMetadata.hpp"

template <class T>
class FilterRuleDefinition : public LayerAttributeChildRuleDefinition<T>
{
public:
    typedef LayerAttributeChildRuleDefinition<T>* (*CreateChildFunc)(ConsoleGameRules::EGameRuleType type);

    FilterRuleDefinition(CreateChildFunc createChild);
    virtual ~FilterRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount);
    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);

    virtual T Generate() const;
    virtual bool CanUse(const LayerMetadata& metadata) const;

private:
    CreateChildFunc mCreateChild;
    LayerAttributeChildRuleDefinition<T>* mChild;
    int mVariant;
    LayerMetadata::eFilter mFilter;
};

// LayerAttributeRuleDefinition.cpp defines an explicit specialisation of this
// constructor, and FilterRuleDefinition.cpp explicitly instantiates the generic
// template for the same type. C++ requires the specialisation to be declared
// before any instantiation of it; without this declaration the instantiation
// silently generates a second definition. GCC tolerated the resulting ODR
// violation, so the RPX has always linked -- lld rejects it as a duplicate
// symbol, which is how it was found.
class LayerFill;
template <>
FilterRuleDefinition<LayerFill*>::FilterRuleDefinition(
    FilterRuleDefinition<LayerFill*>::CreateChildFunc createChild);

#endif
