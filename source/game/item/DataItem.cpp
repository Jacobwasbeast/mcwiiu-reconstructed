#include "game/item/DataItem.hpp"

DataItem::DataItem()
{
    DataItem* obj = this;
    if (obj != NULL || (obj = static_cast<DataItem*>(operator new(8)), obj != NULL))
    {
        obj->mDirty = true;
    }
}

DataItem::~DataItem()
{
}

EntityDataSerializerEnum::EEntityDataSerializer DataItem::getSerializerId()
{
    return EntityDataSerializerEnum::NUM_ENTITY_DATA_SERIALIZERS;
}

int DataItem::getAccessorId()
{
    return -1;
}

bool DataItem::isDirty()
{
    return mDirty;
}

void DataItem::setDirty(bool dirty)
{
    mDirty = dirty;
}
