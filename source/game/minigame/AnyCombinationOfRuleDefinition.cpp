#include "game/minigame/AnyCombinationOfRuleDefinition.hpp"

#include "game/minigame/CombinationVariationsRuleDefinition.hpp"
#include "game/core/IGameRuleAttribute.hpp"
#include "game/world/generation/LayerGenerationRuleDefinition.hpp"
#include "game/world/generation/LayerMetadata.hpp"
#include "game/world/generation/LayerThemeRuleDefinition.hpp"
#include "game/util/Random.hpp"

AnyCombinationOfRuleDefinition::AnyCombinationOfRuleDefinition(LayerGenerationRuleDefinition* parent)
    : mParent(parent)
    , mDefinition(NULL)
    , mName(L"AnyLayer")
    , mFilter(L"AnyLayer")
    , mCombinationVariations(NULL)
    , mSizes(new LayerSizeRuleDefinition())
    , mShapes(new LayerShapeRuleDefinition())
    , mFills(new LayerFillRuleDefinition())
{
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_name, &mName);
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_type, &mFilter);
}

AnyCombinationOfRuleDefinition::AnyCombinationOfRuleDefinition(
    LayerGenerationRuleDefinition* parent, AnyCombinationOfRuleDefinition* definition)
    : mParent(parent)
    , mDefinition(definition)
    , mName(L"AnyLayer")
    , mFilter(L"AnyLayer")
    , mCombinationVariations(NULL)
    , mSizes(new LayerSizeRuleDefinition())
    , mShapes(new LayerShapeRuleDefinition())
    , mFills(new LayerFillRuleDefinition())
{
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_name, &mName);
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_type, &mFilter);
}

AnyCombinationOfRuleDefinition::~AnyCombinationOfRuleDefinition()
{
    delete mCombinationVariations;
    delete mSizes;
    delete mShapes;
    delete mFills;
    for (std::vector<LayerThemeRuleDefinition*>::iterator it = mThemes.begin();
         it != mThemes.end(); ++it)
    {
        delete *it;
    }
}

AnyCombinationOfRuleDefinition* AnyCombinationOfRuleDefinition::GetRandomLeafChild(
    const LayerMetadata& metadata, Random& layerRandom, Random& random) const
{
    if (mCombinationVariations == NULL)
    {
        return const_cast<AnyCombinationOfRuleDefinition*>(this);
    }

    std::vector<AnyCombinationOfRuleDefinition*>* combinations =
        mCombinationVariations->GetCombinations();
    unsigned int count = static_cast<unsigned int>(combinations->size());
    bool wholeLevel = mCombinationVariations->IsForWholeLevel();
    AnyCombinationOfRuleDefinition* child;

    do
    {
        unsigned int index = static_cast<unsigned int>(layerRandom.nextInt(static_cast<int>(count)));
        if (!wholeLevel)
        {
            index = static_cast<unsigned int>(random.nextInt(static_cast<int>(count)));
        }

        child = combinations->at(index);
    }
    while (child == NULL || LayerMetadata::getFilter(child->mFilter)(metadata) == false);

    return child->GetRandomLeafChild(metadata, layerRandom, random);
}

void AnyCombinationOfRuleDefinition::GetPossibleLayerSizes(
    std::vector<LayerAttributeChildRuleDefinition<LayerSize*>*>& sizes) const
{
    std::vector<GameRuleDefinition*> children;
    mSizes->getChildren(&children);
    std::vector<GameRuleDefinition*>::reverse_iterator it;
    for (it = children.rbegin(); it != children.rend(); ++it)
    {
        sizes.insert(sizes.begin(), static_cast<LayerAttributeChildRuleDefinition<LayerSize*>*>(*it));
    }

    if (mDefinition != NULL)
    {
        mDefinition->GetPossibleLayerSizes(sizes);
    }
}

void AnyCombinationOfRuleDefinition::GetPossibleLayerShapes(
    std::vector<LayerAttributeChildRuleDefinition<LayerShape*>*>& shapes) const
{
    std::vector<GameRuleDefinition*> children;
    mShapes->getChildren(&children);
    std::vector<GameRuleDefinition*>::reverse_iterator it;
    for (it = children.rbegin(); it != children.rend(); ++it)
    {
        shapes.insert(shapes.begin(), static_cast<LayerAttributeChildRuleDefinition<LayerShape*>*>(*it));
    }

    if (mDefinition != NULL)
    {
        mDefinition->GetPossibleLayerShapes(shapes);
    }
}

void AnyCombinationOfRuleDefinition::GetPossibleLayerFills(
    std::vector<LayerAttributeChildRuleDefinition<LayerFill*>*>& fills) const
{
    std::vector<GameRuleDefinition*> children;
    mFills->getChildren(&children);
    std::vector<GameRuleDefinition*>::reverse_iterator it;
    for (it = children.rbegin(); it != children.rend(); ++it)
    {
        fills.insert(fills.begin(), static_cast<LayerAttributeChildRuleDefinition<LayerFill*>*>(*it));
    }

    if (mDefinition != NULL)
    {
        mDefinition->GetPossibleLayerFills(fills);
    }
}

void AnyCombinationOfRuleDefinition::GetPossibleLayerThemes(int passId,
    std::vector<LayerAttributeChildRuleDefinition<LayerTheme*>*>& themes) const
{
    const AnyCombinationOfRuleDefinition* definition = this;
    while (definition != NULL)
    {
        LayerThemeRuleDefinition* matchingTheme = NULL;
        std::vector<LayerThemeRuleDefinition*>::const_iterator it;
        for (it = definition->mThemes.begin(); it != definition->mThemes.end(); ++it)
        {
            if ((*it)->GetPassId() == static_cast<unsigned int>(passId))
            {
                matchingTheme = *it;
            }
        }

        if (matchingTheme != NULL)
        {
            std::vector<GameRuleDefinition*> children;
            matchingTheme->getChildren(&children);
            std::vector<GameRuleDefinition*>::reverse_iterator child = children.rbegin();
            for (; child != children.rend(); ++child)
            {
                themes.insert(themes.begin(),
                    static_cast<LayerAttributeChildRuleDefinition<LayerTheme*>*>(*child));
            }
        }

        definition = definition->mDefinition;
    }
}

void AnyCombinationOfRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    children->push_back(mCombinationVariations);
}

GameRuleDefinition* AnyCombinationOfRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    static CombinationVariationsRuleDefinition variationsActionType(NULL, NULL);
    static LayerThemeRuleDefinition themeActionType;

    if (type == variationsActionType.getActionType())
    {
        mCombinationVariations = new CombinationVariationsRuleDefinition(mParent, this);
        return mCombinationVariations;
    }

    if (type == mSizes->getActionType())
    {
        return mSizes;
    }

    if (type == mShapes->getActionType())
    {
        return mShapes;
    }

    if (type == mFills->getActionType())
    {
        return mFills;
    }

    if (type == themeActionType.getActionType())
    {
        LayerThemeRuleDefinition* theme = new LayerThemeRuleDefinition();
        mThemes.push_back(theme);
        return theme;
    }

    return GameRuleDefinition::addChild(type);
}

ConsoleGameRules::EGameRuleType AnyCombinationOfRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_AnyCombinationOf;
}
