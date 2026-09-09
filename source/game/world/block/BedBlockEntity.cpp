#include "game/world/block/BedBlockEntity.hpp"

#include <string>

#include "game/core/DyeColor.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

BedBlockEntity::BedBlockEntity()
    : BlockEntity(), mColor(DyeColor::WHITE)
{
}

BedBlockEntity::~BedBlockEntity()
{
}

void BedBlockEntity::setColor(const DyeColor* color)
{
    mColor = color;
    setChanged();
}

void BedBlockEntity::setItemValues(not_null_ptr<ItemInstance> item)
{
    setColor(DyeColor::byBlockData(item->getAuxValue()));
}

const DyeColor* BedBlockEntity::getColor()
{
    return mColor;
}

not_null_ptr<ItemInstance> BedBlockEntity::getItem()
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, mColor->getBlockData()));
}

boost::shared_ptr<BlockEntity> BedBlockEntity::clone()
{
    boost::shared_ptr<BlockEntity> result(new BedBlockEntity());
    BlockEntity::clone(result);
    static_cast<BedBlockEntity*>(result.get())->mColor = mColor;
    return result;
}

void BedBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    const std::wstring name(L"color");
    if (tag->contains(name))
    {
        mColor = DyeColor::byBlockData(tag->getInt(name));
    }
}

CompoundTag* BedBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putInt(L"color", mColor->getBlockData());
    return tag;
}

CompoundTag* BedBlockEntity::getUpdateTag()
{
    CompoundTag* tag = new CompoundTag();
    save(tag);
    return tag;
}

boost::shared_ptr<Packet> BedBlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<Packet>();
}

bool BedBlockEntity::isHeadPiece()
{
    return (getData() & 8) != 0;
}

unsigned long long BedBlockEntity::GetType()
{
    return 0x0000000001000210ULL;
}

BlockEntity* BedBlockEntity::create()
{
    return new BedBlockEntity();
}
