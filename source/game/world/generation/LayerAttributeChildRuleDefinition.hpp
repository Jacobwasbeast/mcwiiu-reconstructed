#ifndef GAME_WORLD_GENERATION_LAYERATTRIBUTECHILDRULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_LAYERATTRIBUTECHILDRULEDEFINITION_HPP

#include "game/minigame/GameRuleDefinition.hpp"

class LayerMetadata;

template <class T>
class LayerAttributeChildRuleDefinition : public GameRuleDefinition
{
public:
    LayerAttributeChildRuleDefinition();
    virtual ~LayerAttributeChildRuleDefinition();

    virtual T Generate() const = 0;
    virtual bool CanUse(const LayerMetadata& metadata) const;

    T GetValue();

private:
    T mGeneratedValue;
    bool mWholeLevel;
};

template <class T>
LayerAttributeChildRuleDefinition<T>::LayerAttributeChildRuleDefinition()
    : mGeneratedValue(NULL)
    , mWholeLevel(false)
{
}

template <class T>
LayerAttributeChildRuleDefinition<T>::~LayerAttributeChildRuleDefinition()
{
}

template <class T>
bool LayerAttributeChildRuleDefinition<T>::CanUse(const LayerMetadata& metadata) const
{
    return true;
}

template <class T>
T LayerAttributeChildRuleDefinition<T>::GetValue()
{
    if (!mWholeLevel)
    {
        mGeneratedValue = Generate();
    }

    return mGeneratedValue;
}

#endif
