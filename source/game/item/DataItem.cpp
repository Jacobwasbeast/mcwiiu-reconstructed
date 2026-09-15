#include "game/item/DataItem.hpp"

DataItem::DataItem()
    : mDirty(true)
{
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
