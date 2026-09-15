#include "game/world/block/DataItem_Impl.hpp"

#include "game/core/CriticalSection.hpp"
#include "game/core/Rotations.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/EntityDataAccessor.hpp"

static OSFastMutex sDataItemLock;

template <>
DataItem_Impl<BlockPos*>::~DataItem_Impl()
{
    delete mValue;
}

template <>
void DataItem_Impl<BlockPos*>::setValue(BlockPos* value)
{
    delete mValue;
    mValue = value;
}

template <>
bool DataItem_Impl<BlockPos*>::shouldUpdate(BlockPos* value)
{
    if (mValue == NULL || value == NULL)
    {
        return mValue != value;
    }

    return !(mValue->x == value->z && mValue->y == value->y && mValue->z == value->z);
}

template <>
DataItem_Impl<CompoundTag*>::~DataItem_Impl()
{
    delete mValue;
}

template <>
void DataItem_Impl<CompoundTag*>::setValue(CompoundTag* value)
{
    EnterCriticalSection(&sDataItemLock);

    delete mValue;
    mValue = value;

    LeaveCriticalSection(&sDataItemLock);
}

template <>
DataItem_Impl<const Rotations*>::~DataItem_Impl()
{
    delete mValue;
}

template <>
void DataItem_Impl<const Rotations*>::setValue(const Rotations* value)
{
    EnterCriticalSection(&sDataItemLock);

    delete mValue;

    if (value == NULL)
    {
        mValue = NULL;
    }
    else
    {
        mValue = new Rotations(*value);
    }

    LeaveCriticalSection(&sDataItemLock);
}
