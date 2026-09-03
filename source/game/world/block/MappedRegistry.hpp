#ifndef GAME_WORLD_BLOCK_MAPPEDREGISTRY_HPP
#define GAME_WORLD_BLOCK_MAPPEDREGISTRY_HPP

#include <functional>

#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>

#include "game/world/block/IdMap.hpp"
#include "game/world/block/SimpleRegistry.hpp"

template <class T>
class IdMapper;

template <class K,
          class V,
          class KeyHash = boost::hash<K>,
          class KeyEqual = std::equal_to<K>,
          class ValueHash = boost::hash<V>,
          class ValueEqual = std::equal_to<V> >
class MappedRegistry : public SimpleRegistry<K, V, KeyHash, KeyEqual>, public IdMap<V>
{
public:
    MappedRegistry();
    ~MappedRegistry();

    virtual V get(K key);
    virtual void registerKey(K key, V value);
    virtual bool containsKey(K key);

    virtual void registerMapping(int id, K key, V value);
    virtual K getKey(V value) const;
    virtual int getId(V value);
    virtual V byId(int id);
    virtual bool containsId(int id);

private:
    int mNextId;
    IdMapper<V>* mIdMapper;
    boost::unordered_map<V, K, ValueHash, ValueEqual> mInverseRegistry;
};

template <class K, class V, class KeyHash, class KeyEqual, class ValueHash, class ValueEqual>
MappedRegistry<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::MappedRegistry()
{
    mIdMapper = new IdMapper<V>();
    const boost::unordered_map<K, V, KeyHash, KeyEqual>& registry = this->keySet();

    for (typename boost::unordered_map<K, V, KeyHash, KeyEqual>::const_iterator it = registry.begin();
         it != registry.end();
         ++it)
    {
        mInverseRegistry[it->second] = it->first;
    }
}

template <class K, class V, class KeyHash, class KeyEqual, class ValueHash, class ValueEqual>
MappedRegistry<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::~MappedRegistry()
{
    delete mIdMapper;
}

template <class K, class V, class KeyHash, class KeyEqual, class ValueHash, class ValueEqual>
V MappedRegistry<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::get(K key)
{
    return SimpleRegistry<K, V, KeyHash, KeyEqual>::get(key);
}

template <class K, class V, class KeyHash, class KeyEqual, class ValueHash, class ValueEqual>
void MappedRegistry<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::registerKey(K key, V value)
{
    SimpleRegistry<K, V, KeyHash, KeyEqual>::registerKey(key, value);

    mInverseRegistry[value] = key;
}

template <class K, class V, class KeyHash, class KeyEqual, class ValueHash, class ValueEqual>
bool MappedRegistry<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::containsKey(K key)
{
    return SimpleRegistry<K, V, KeyHash, KeyEqual>::containsKey(key);
}

template <class K, class V, class KeyHash, class KeyEqual, class ValueHash, class ValueEqual>
void MappedRegistry<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::registerMapping(int id, K key, V value)
{
    mIdMapper->addMapping(value, id);

    registerKey(key, value);
}

template <class K, class V, class KeyHash, class KeyEqual, class ValueHash, class ValueEqual>
K MappedRegistry<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::getKey(V value) const
{
    typename boost::unordered_map<V, K, ValueHash, ValueEqual>::const_iterator found = mInverseRegistry.find(value);
    if (found == mInverseRegistry.end())
    {
        return K();
    }

    return found->second;
}

template <class K, class V, class KeyHash, class KeyEqual, class ValueHash, class ValueEqual>
int MappedRegistry<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::getId(V value)
{
    return mIdMapper->getId(value);
}

template <class K, class V, class KeyHash, class KeyEqual, class ValueHash, class ValueEqual>
V MappedRegistry<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::byId(int id)
{
    return mIdMapper->byId(id);
}

template <class K, class V, class KeyHash, class KeyEqual, class ValueHash, class ValueEqual>
bool MappedRegistry<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::containsId(int id)
{
    return mIdMapper->byId(id) != V();
}

#endif
