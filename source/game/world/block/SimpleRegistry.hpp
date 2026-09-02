#ifndef GAME_WORLD_BLOCK_SIMPLEREGISTRY_HPP
#define GAME_WORLD_BLOCK_SIMPLEREGISTRY_HPP

#include <set>

#include <boost/unordered_map.hpp>

#include "game/util/Random.hpp"

template <class K, class V, class KeyHash, class KeyEqual>
class SimpleRegistry
{
public:
    virtual V get(K key);
    virtual void registerKey(K key, V value);
    virtual const boost::unordered_map<K, V, KeyHash, KeyEqual>& keySet();
    virtual V getRandom(Random* random);
    virtual bool containsKey(K key);

private:
    int mNextId;
    boost::unordered_map<K, V, KeyHash, KeyEqual> mRegistry;
    std::set<V> mValues;
};

template <class K, class V, class KeyHash, class KeyEqual>
V SimpleRegistry<K, V, KeyHash, KeyEqual>::get(K key)
{
    return mRegistry.at(key);
}

template <class K, class V, class KeyHash, class KeyEqual>
void SimpleRegistry<K, V, KeyHash, KeyEqual>::registerKey(K key, V value)
{
    mRegistry[key] = value;

    mValues.insert(value);
}

template <class K, class V, class KeyHash, class KeyEqual>
const boost::unordered_map<K, V, KeyHash, KeyEqual>& SimpleRegistry<K, V, KeyHash, KeyEqual>::keySet()
{
    return mRegistry;
}

template <class K, class V, class KeyHash, class KeyEqual>
V SimpleRegistry<K, V, KeyHash, KeyEqual>::getRandom(Random* random)
{
    int index = random->nextInt((int)mValues.size());

    for (typename std::set<V>::const_iterator it = mValues.begin(); it != mValues.end(); ++it)
    {
        if (index == 0)
        {
            return *it;
        }

        --index;
    }

    return V();
}

template <class K, class V, class KeyHash, class KeyEqual>
bool SimpleRegistry<K, V, KeyHash, KeyEqual>::containsKey(K key)
{
    return mRegistry.find(key) != mRegistry.end();
}

#endif
