#include "game/world/block/FlowerPotBlockEntity.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"

FlowerPotBlockEntity::FlowerPotBlockEntity(Item* item, int data)
    : BlockEntity(), mPlantItem(item), mItemData(data)
{
}

FlowerPotBlockEntity::FlowerPotBlockEntity()
    : BlockEntity(), mPlantItem(NULL), mItemData(0)
{
}

FlowerPotBlockEntity::~FlowerPotBlockEntity()
{
}

BlockEntity* FlowerPotBlockEntity::create()
{
    return new FlowerPotBlockEntity();
}

Item* FlowerPotBlockEntity::getPlantItem()
{
    return mPlantItem;
}

not_null_ptr<ItemInstance> FlowerPotBlockEntity::getPlantItemInstance()
{
    return not_null_ptr<ItemInstance>(new ItemInstance(mPlantItem, 1, mItemData));
}

void FlowerPotBlockEntity::setItemInstance(const not_null_ptr<ItemInstance>& item)
{
    mPlantItem = item->getItem();
    mItemData = item->getAuxValue();
    setChanged();
}

int FlowerPotBlockEntity::getItemData()
{
    return mItemData;
}

void FlowerPotBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
}

void FlowerPotBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putInt(L"Item", Item::VALIDgetId(mPlantItem));
    tag->putInt(L"Data", mItemData);
}

void FlowerPotBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mPlantItem = Item::byId(tag->getInt(L"Item"));
    mItemData = tag->getInt(L"Data");
}

boost::shared_ptr<Packet> FlowerPotBlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<Packet>();
}

CompoundTag* FlowerPotBlockEntity::getUpdateTag()
{
    CompoundTag* tag = new CompoundTag();
    save(tag);
    return tag;
}

boost::shared_ptr<BlockEntity> FlowerPotBlockEntity::clone()
{
    boost::shared_ptr<BlockEntity> result(new FlowerPotBlockEntity(mPlantItem, mItemData));
    BlockEntity::clone(result);
    return result;
}

unsigned long long FlowerPotBlockEntity::GetType()
{
    return 0x0100020D00000000ULL;
}
