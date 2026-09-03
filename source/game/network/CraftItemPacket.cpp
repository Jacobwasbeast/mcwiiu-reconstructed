#include "game/network/CraftItemPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

CraftItemPacket::CraftItemPacket()
    : mStackedItems(), mItemIds(), mContainerId(0), mCraftType(0)
{
}

CraftItemPacket::CraftItemPacket(int itemId, int count, short containerId)
    : mStackedItems(), mItemIds(), mContainerId(containerId), mCraftType(static_cast<unsigned char>(count))
{
    mStackedItems.push_back(std::make_pair(itemId, -1));
}

CraftItemPacket::CraftItemPacket(std::vector<std::pair<int, int> >& stackedItems, int craftType, short containerId)
    : mStackedItems(stackedItems), mItemIds(), mContainerId(containerId), mCraftType(static_cast<unsigned char>(craftType))
{
}

CraftItemPacket::CraftItemPacket(std::vector<int>& itemIds, short containerId)
    : mStackedItems(), mItemIds(itemIds), mContainerId(containerId), mCraftType(0)
{
}

CraftItemPacket::~CraftItemPacket()
{
}

int CraftItemPacket::getPacketId()
{
    return 150;
}

void CraftItemPacket::read(DataInputStream* input)
{
    mContainerId = input->readShort();
    mCraftType = input->readByte();
    unsigned char stackedItems = input->readByte();
    int itemCount = input->readInt();
    if (stackedItems == 0)
    {
        for (int index = 0; index < itemCount; ++index)
        {
            mItemIds.push_back(input->readInt());
        }
    }
    else
    {
        for (int index = 0; index < itemCount; ++index)
        {
            int itemId = input->readInt();
            int itemCountValue = input->readInt();
            mStackedItems.push_back(std::make_pair(itemId, itemCountValue));
        }
    }
}

void CraftItemPacket::write(DataOutputStream* output)
{
    output->writeShort(mContainerId);
    output->writeByte(mCraftType);
    if (!mStackedItems.empty())
    {
        output->writeByte(1);
        output->writeInt(static_cast<int>(mStackedItems.size()));
        for (std::vector<std::pair<int, int> >::const_iterator it = mStackedItems.begin(); it != mStackedItems.end(); ++it)
        {
            output->writeInt(it->first);
            output->writeInt(it->second);
        }
    }
    else
    {
        output->writeByte(0);
        output->writeInt(static_cast<int>(mItemIds.size()));
        for (std::vector<int>::const_iterator it = mItemIds.begin(); it != mItemIds.end(); ++it)
        {
            output->writeInt(*it);
        }
    }
}

void CraftItemPacket::handle(PacketListener* listener)
{
    listener->handleCraftItem(shared_from_this());
}

int CraftItemPacket::getEstimatedSize()
{
    return 7;
}

boost::shared_ptr<Packet> CraftItemPacket::create()
{
    return boost::shared_ptr<Packet>(new CraftItemPacket());
}
