#ifndef GAME_WORLD_BLOCK_BLOCKSTATEPREDICATE_HPP
#define GAME_WORLD_BLOCK_BLOCKSTATEPREDICATE_HPP

#include <typeinfo>
#include <boost/unordered_map.hpp>

#include "game/core/Boxed.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/Predicate.hpp"

class Block;
class BlockState;
class BlockStateDefinition;
class Property;

class BlockStatePredicate : public Predicate<const BlockState*>
{
public:
    template <typename T>
    class PredicateConversion : public Predicate<Boxed*>
    {
    public:
        PredicateConversion(const Property* property, T value)
            : mProperty(property), mValue(value)
        {
        }
        virtual bool apply(Boxed* value) const;
        virtual ~PredicateConversion();
    private:
        const Property* mProperty;
        T mValue;
    };

    explicit BlockStatePredicate(BlockStateDefinition* definition);
    virtual ~BlockStatePredicate();
    static Predicate<const BlockState*>* forBlock(Block* block);
    static Predicate<const BlockState*>* forDefinition(BlockStateDefinition* definition);
    virtual bool apply(const BlockState* state) const;
    BlockStatePredicate* where(Property* property, const Predicate<Boxed*>* predicate);
private:
    BlockStateDefinition* mDefinition;
    boost::unordered_map<Property*, const Predicate<Boxed*>*> mPredicates;
};

template <typename T>
bool BlockStatePredicate::PredicateConversion<T>::apply(Boxed* value) const
{
    const TypedBoxed<T>* boxed = value != NULL && value->isA(typeid(T))
        ? static_cast<const TypedBoxed<T>*>(value) : NULL;
    return boxed != NULL && mValue == boxed->getValue();
}

template <typename T>
BlockStatePredicate::PredicateConversion<T>::~PredicateConversion()
{
}

#endif
