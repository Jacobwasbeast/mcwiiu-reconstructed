#ifndef GAME_WORLD_BLOCK_DATAITEM_IMPL_HPP
#define GAME_WORLD_BLOCK_DATAITEM_IMPL_HPP

#include "game/item/DataItem.hpp"
#include "game/world/block/EntityDataAccessor.hpp"

class BlockPos;
class CompoundTag;
class Rotations;

template <class T>
class DataItem_Impl : public DataItem
{
public:
    DataItem_Impl(const EntityDataAccessor<T>* accessor, T value);
    virtual ~DataItem_Impl();

    virtual DataItem* copy() const;
    virtual EntityDataSerializerEnum::EEntityDataSerializer getSerializerId();
    virtual int getAccessorId();
    virtual void setValue(T value);

    T getValue();
    const EntityDataAccessor<T>* getAccessor();
    bool shouldUpdate(T value);

private:
    const EntityDataAccessor<T>* mAccessor;
    T mValue;
};

template <class T>
DataItem_Impl<T>::DataItem_Impl(const EntityDataAccessor<T>* accessor, T value)
    : mAccessor(accessor)
{
    mValue = value;
}

template <class T>
DataItem_Impl<T>::~DataItem_Impl()
{
}

template <class T>
DataItem* DataItem_Impl<T>::copy() const
{
    return new DataItem_Impl<T>(mAccessor, mAccessor->getSerializer()->copy(mValue));
}

template <class T>
EntityDataSerializerEnum::EEntityDataSerializer DataItem_Impl<T>::getSerializerId()
{
    return mAccessor->getSerializer()->getId();
}

template <class T>
int DataItem_Impl<T>::getAccessorId()
{
    return mAccessor->getId();
}

template <class T>
void DataItem_Impl<T>::setValue(T value)
{
    mValue = value;
}

template <class T>
T DataItem_Impl<T>::getValue()
{
    return mValue;
}

template <class T>
const EntityDataAccessor<T>* DataItem_Impl<T>::getAccessor()
{
    return mAccessor;
}

template <class T>
bool DataItem_Impl<T>::shouldUpdate(T value)
{
    return mValue != value;
}

template <>
DataItem_Impl<BlockPos*>::~DataItem_Impl();

template <>
void DataItem_Impl<BlockPos*>::setValue(BlockPos* value);

template <>
bool DataItem_Impl<BlockPos*>::shouldUpdate(BlockPos* value);

template <>
DataItem_Impl<CompoundTag*>::~DataItem_Impl();

template <>
void DataItem_Impl<CompoundTag*>::setValue(CompoundTag* value);

template <>
DataItem_Impl<const Rotations*>::~DataItem_Impl();

template <>
void DataItem_Impl<const Rotations*>::setValue(const Rotations* value);

template <>
bool DataItem_Impl<const Rotations*>::shouldUpdate(const Rotations* value);

#endif
