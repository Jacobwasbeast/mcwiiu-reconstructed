#include "game/network/ClientboundMapItemDataPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#define private public
#include "game/item/MapItemSavedData.hpp"
#undef private
#include "game/network/PacketListener.hpp"

ClientboundMapItemDataPacket::ClientboundMapItemDataPacket()
    : Packet(), boost::enable_shared_from_this<ClientboundMapItemDataPacket>()
{
}

ClientboundMapItemDataPacket::ClientboundMapItemDataPacket(int mapId, unsigned char scale, bool trackingPosition, const std::vector<MapDecoration>& decorations, arrayWithLength<unsigned char> colors, int startX, int startY, int width, int height)
    : Packet(), boost::enable_shared_from_this<ClientboundMapItemDataPacket>()
{
    mMapId = mapId;
    mScale = scale;
    mTrackingPosition = trackingPosition;
    mDecorations = decorations;
    mStartX = startX;
    mStartY = startY;
    mWidth = width;
    mHeight = height;
    if (mWidth > 0)
    {
        mColors.length = (unsigned int)(mWidth * mHeight);
        mColors.data = new unsigned char[mColors.length]();
        for (int x = 0; x < mWidth; x++)
        {
            for (int y = 0; y < mHeight; y++)
            {
                mColors.data[y * mWidth + x] = colors.data[(mStartX + x) + (mStartY + y) * 128];
            }
        }
    }
}

ClientboundMapItemDataPacket::~ClientboundMapItemDataPacket()
{
    delete[] mColors.data;
}

void ClientboundMapItemDataPacket::handle(PacketListener* listener)
{
    listener->handleMapItemData(shared_from_this());
}

void ClientboundMapItemDataPacket::read(DataInputStream* input)
{
    mMapId = input->readVarInt();
    mScale = input->readByte();
    mTrackingPosition = input->readBoolean();

    int decorationCount = input->readVarInt();
    mDecorations = std::vector<MapDecoration>(decorationCount);
    for (int i = 0; i < decorationCount; i++)
    {
        unsigned char icon = input->readByte();
        char x = (char)input->readByte();
        char y = (char)input->readByte();
        unsigned int packed = (unsigned int)input->readByte();
        packed |= (unsigned int)input->readByte() << 8;
        packed |= (unsigned int)input->readByte() << 16;
        unsigned char rotationAndVisibility = input->readByte();
        int id = (int)(packed | (unsigned int)(rotationAndVisibility & 7) << 24);
        bool visible = (rotationAndVisibility & 0x80) != 0;
        int rotation = input->readByte();
        mDecorations[i] = MapDecoration(MapDecoration::Type::byIcon(icon), x, y, (char)rotation, id, visible);
    }

    mWidth = input->readByte();
    if (mWidth > 0)
    {
        mHeight = input->readByte();
        mStartX = input->readByte();
        mStartY = input->readByte();
        mColors.length = (unsigned int)input->readInt();
        mColors.data = new unsigned char[mColors.length]();
        input->readFully(mColors);
    }
}

void ClientboundMapItemDataPacket::write(DataOutputStream* output)
{
    output->writeVarInt(mMapId);
    output->writeByte(mScale);
    output->writeBoolean(mTrackingPosition);
    output->writeVarInt((int)mDecorations.size());
    for (std::vector<MapDecoration>::const_iterator it = mDecorations.begin(); it != mDecorations.end(); ++it)
    {
        output->writeByte(it->getImg());
        output->writeByte((unsigned char)it->getX());
        output->writeByte((unsigned char)it->getY());
        int id = it->id;
        output->writeByte((unsigned char)id);
        output->writeByte((unsigned char)(id >> 8));
        output->writeByte((unsigned char)(id >> 16));
        output->writeByte((unsigned char)((id >> 24) & 0x7f) | (it->visible ? 0x80 : 0));
        output->writeByte((unsigned char)it->getRot());
    }
    output->writeByte((unsigned char)mWidth);
    if (mWidth > 0)
    {
        output->writeByte((unsigned char)mHeight);
        output->writeByte((unsigned char)mStartX);
        output->writeByte((unsigned char)mStartY);
        output->writeInt((int)mColors.length);
        output->write(mColors);
    }
}

int ClientboundMapItemDataPacket::getMapId()
{
    return mMapId;
}

void ClientboundMapItemDataPacket::applyToMap(MapItemSavedData* mapData)
{
    mapData->mScale = mScale;
    mapData->mTrackingPosition = mTrackingPosition;
    mapData->mDecorations.clear();
    for (std::vector<MapDecoration>::const_iterator it = mDecorations.begin(); it != mDecorations.end(); ++it)
    {
        mapData->mDecorations[(int)(it - mDecorations.begin())] = *it;
    }
    if (mWidth > 0)
    {
        for (int x = 0; x < mWidth; x++)
        {
            for (int y = 0; y < mHeight; y++)
            {
                mapData->mColors.data[(mStartX + x) + (mStartY + y) * 128] = mColors.data[y * mWidth + x];
            }
        }
    }
}

bool ClientboundMapItemDataPacket::isDuplicatePacket(boost::shared_ptr<ClientboundMapItemDataPacket> packet)
{
    return packet->mMapId == mMapId && packet->mStartX == mStartX && packet->mStartY == mStartY && packet->mWidth == mWidth && packet->mHeight == mHeight && packet->mDecorations.size() == mDecorations.size();
}

bool ClientboundMapItemDataPacket::tryReplaceDuplicatePacket(std::deque<boost::shared_ptr<Packet>, std::allocator<boost::shared_ptr<Packet> > >* queuedPackets)
{
    for (std::deque<boost::shared_ptr<Packet>, std::allocator<boost::shared_ptr<Packet> > >::iterator it = queuedPackets->begin(); it != queuedPackets->end(); ++it)
    {
        if ((*it)->getPacketId() == getPacketId())
        {
            boost::shared_ptr<ClientboundMapItemDataPacket> packet = boost::static_pointer_cast<ClientboundMapItemDataPacket>(*it);
            if (isDuplicatePacket(packet))
            {
                it = queuedPackets->erase(it, it + 1);
                queuedPackets->insert(it, shared_from_this());
                return true;
            }
        }
    }
    return false;
}

int ClientboundMapItemDataPacket::getPacketId()
{
    return 211;
}

boost::shared_ptr<Packet> ClientboundMapItemDataPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundMapItemDataPacket());
}
