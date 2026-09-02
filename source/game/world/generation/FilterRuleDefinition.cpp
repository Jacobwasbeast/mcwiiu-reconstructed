#include "game/world/generation/FilterRuleDefinition.hpp"

#include <string>
#include <vector>

#include "game/data/GameRuleSaveInterface.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/LayerMetadata.hpp"

template <typename T>
std::wstring toString(T value);

template <typename T>
T fromString(const std::wstring& text);

template <class T>
FilterRuleDefinition<T>::FilterRuleDefinition(CreateChildFunc createChild)
    : mCreateChild(createChild)
    , mChild(NULL)
    , mVariant(-1)
    , mFilter(LayerMetadata::eFilter_AnyLayer)
{
}

template <class T>
FilterRuleDefinition<T>::~FilterRuleDefinition()
{
}

template <class T>
ConsoleGameRules::EGameRuleType FilterRuleDefinition<T>::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(60);
}

template <class T>
void FilterRuleDefinition<T>::writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 2);

    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(74), toString(mVariant));
    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(59), LayerMetadata::toStringFilter(mFilter));
}

template <class T>
void FilterRuleDefinition<T>::getChildren(std::vector<GameRuleDefinition*>* children)
{
    children->push_back(mChild);
}

template <class T>
GameRuleDefinition* FilterRuleDefinition<T>::addChild(ConsoleGameRules::EGameRuleType type)
{
    LayerAttributeChildRuleDefinition<T>* child = mCreateChild(type);
    if (child != NULL)
    {
        if (mChild != NULL)
        {
            delete mChild;
        }

        mChild = child;
        return child;
    }

    return GameRuleDefinition::addChild(type);
}

template <class T>
void FilterRuleDefinition<T>::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"variant") == 0)
    {
        mVariant = fromString<int>(value);
    }
    else if (name.compare(L"type") == 0)
    {
        mFilter = LayerMetadata::fromStringFilter(value);
    }
    else
    {
        GameRuleDefinition::addAttribute(name, value);
    }
}

template <class T>
T FilterRuleDefinition<T>::Generate() const
{
    return mChild->Generate();
}

template <class T>
bool FilterRuleDefinition<T>::CanUse(const LayerMetadata& metadata) const
{
    bool canUse = false;

    if (mVariant == -1 || mVariant == metadata.m_uGameVariant)
    {
        LayerMetadata::FilterFunc filter = LayerMetadata::getFilter(mFilter);
        if (filter(metadata))
        {
            canUse = true;
        }
    }

    return canUse;
}
