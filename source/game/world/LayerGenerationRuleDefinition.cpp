#include "game/world/LayerGenerationRuleDefinition.hpp"

#include <algorithm>

#include "game/core/LayerSize.hpp"
#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/LayerMetadata.hpp"
#include "game/world/generation/LayerShape.hpp"
#include "game/world/generation/LayerTheme.hpp"
#include "game/util/Random.hpp"

class LayerFill;

template <class T>
T* randomUsableElement(Random& random, std::vector<T*>& definitions,
                       const LayerMetadata& metadata)
{
    while (true)
    {
        T* definition = static_cast<const std::vector<T*>&>(definitions).at(static_cast<unsigned int>(
            random.nextInt(static_cast<int>(definitions.size()))));

        typename std::vector<T*>::iterator it = std::find(definitions.begin(), definitions.end(), definition);

        definitions.erase(it);
        if (definition->CanUse(metadata))
        {
            return definition;
        }
    }
}

template LayerAttributeChildRuleDefinition<LayerSize*>*
randomUsableElement(
    Random&, std::vector<LayerAttributeChildRuleDefinition<LayerSize*>*>&,
    const LayerMetadata&);

template LayerAttributeChildRuleDefinition<LayerShape*>*
randomUsableElement(
    Random&, std::vector<LayerAttributeChildRuleDefinition<LayerShape*>*>&,
    const LayerMetadata&);

template LayerAttributeChildRuleDefinition<LayerFill*>*
randomUsableElement(
    Random&, std::vector<LayerAttributeChildRuleDefinition<LayerFill*>*>&,
    const LayerMetadata&);

template LayerAttributeChildRuleDefinition<LayerTheme*>*
randomUsableElement(
    Random&, std::vector<LayerAttributeChildRuleDefinition<LayerTheme*>*>&,
    const LayerMetadata&);
