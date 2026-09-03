#include "game/world/block/BlockStatePredicate.hpp"

#include <algorithm>
#include <cassert>

#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

BlockStatePredicate::BlockStatePredicate(BlockStateDefinition* definition)
    : mDefinition(definition)
{
}

BlockStatePredicate::~BlockStatePredicate()
{
}

Predicate<const BlockState*>* BlockStatePredicate::forBlock(Block* block)
{
    return new BlockStatePredicate(block->getBlockStateDefinition());
}

Predicate<const BlockState*>* BlockStatePredicate::forDefinition(BlockStateDefinition* definition)
{
    return new BlockStatePredicate(definition);
}

bool BlockStatePredicate::apply(const BlockState* state) const
{
    if (state == NULL || state->getBlock() != mDefinition->getBlock())
    {
        return false;
    }
    for (boost::unordered_map<Property*, const Predicate<Boxed*>*>::const_iterator it = mPredicates.begin();
         it != mPredicates.end(); ++it)
    {
        if (!it->second->apply(state->getBoxedValue(it->first)))
        {
            return false;
        }
    }
    return true;
}

BlockStatePredicate* BlockStatePredicate::where(Property* property,
                                                const Predicate<Boxed*>* predicate)
{
    const std::vector<const Property*>& properties = mDefinition->getProperties();
    assert(std::find(properties.begin(), properties.end(), property) != properties.end());
    mPredicates[property] = predicate;
    return this;
}
