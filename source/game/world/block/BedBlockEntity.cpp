#include "game/world/block/BedBlockEntity.hpp"

#include <string>

#include "game/core/DyeColor.hpp"
#include "game/data/CompoundTag.hpp"

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
    if (item)
    {
        setColor(mColor);
    }
}

const DyeColor* BedBlockEntity::getColor()
{
    return mColor;
}

not_null_ptr<ItemInstance> BedBlockEntity::getItem()
{
    return not_null_ptr<ItemInstance>();
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

void BedBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putInt(L"color", mColor->getBlockData());
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
    return 0x0100021000000000ULL;
}

BlockEntity* BedBlockEntity::create()
{
    return new BedBlockEntity();
}
