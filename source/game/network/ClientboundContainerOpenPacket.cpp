#include "game/network/ClientboundContainerOpenPacket.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/PacketListener.hpp"

ClientboundContainerOpenPacket::ClientboundContainerOpenPacket()
    : mContainerId(0),
      mType(0),
      mSize(0),
      mHasCustomName(false),
      mEntityId(0),
      mNameData(L"", L"", true, false)
{
}

ClientboundContainerOpenPacket::ClientboundContainerOpenPacket(int containerId, int type, InteractionObject::NameData data)
    : mNameData(L"", L"", true, false)
{
    _init(containerId, type, data, 0, 0);
}

ClientboundContainerOpenPacket::ClientboundContainerOpenPacket(int containerId, int type, InteractionObject::NameData data, int size)
    : mNameData(L"", L"", true, false)
{
    _init(containerId, type, data, size, 0);
}

ClientboundContainerOpenPacket::ClientboundContainerOpenPacket(int containerId, int type, InteractionObject::NameData data, int size, int entityId)
    : mNameData(L"", L"", true, false)
{
    _init(containerId, type, data, size, entityId);
}

ClientboundContainerOpenPacket::~ClientboundContainerOpenPacket()
{
}

void ClientboundContainerOpenPacket::_init(int containerId, int type, InteractionObject::NameData data, int size, int entityId)
{
    mContainerId = containerId;
    mType = type;
    mNameData.name = data.name;
    mNameData.ownerUuid = data.ownerUuid;
    mSize = size;
    mNameData.createdOnHost = data.createdOnHost;
    mEntityId = entityId;
    mNameData.createdByRestrictedPlayer = data.createdByRestrictedPlayer;
    mHasCustomName = !data.name.empty();
}

void ClientboundContainerOpenPacket::handle(PacketListener* listener)
{
    listener->handleContainerOpen(shared_from_this());
}

void ClientboundContainerOpenPacket::read(DataInputStream* input)
{
    mContainerId = input->readByte();
    mType = input->readByte();
    mSize = input->readByte();
    mHasCustomName = input->readBoolean();
    if (mType == 11 || mType == 6)
    {
        mEntityId = input->readInt();
    }
    if (mHasCustomName)
    {
        mNameData.name = Packet::readUtf(input, 64);
        mNameData.ownerUuid = Packet::readUtf(input, 64);
        mNameData.createdByRestrictedPlayer = input->readBoolean();
        mNameData.createdOnHost = input->readBoolean();
    }
}

void ClientboundContainerOpenPacket::write(DataOutputStream* output)
{
    output->writeByte((unsigned char)mContainerId);
    output->writeByte((unsigned char)mType);
    output->writeByte((unsigned char)mSize);
    output->writeBoolean(mHasCustomName);
    if (mType == 11 || mType == 6)
    {
        output->writeInt(mEntityId);
    }
    if (mHasCustomName)
    {
        Packet::writeUtf(mNameData.name, output);
        Packet::writeUtf(mNameData.ownerUuid, output);
        output->writeBoolean(mNameData.createdByRestrictedPlayer);
        output->writeBoolean(mNameData.createdOnHost);
    }
}

int ClientboundContainerOpenPacket::getEstimatedSize()
{
    if (mType == 11 || mType == 6)
    {
        return 10;
    }
    return 6;
}

int ClientboundContainerOpenPacket::getContainerId()
{
    return mContainerId;
}

int ClientboundContainerOpenPacket::getType()
{
    return mType;
}

uint32_t ClientboundContainerOpenPacket::hasCustomName()
{
    if (!mHasCustomName)
    {
        return 0;
    }
    if ((!g_pApp->IsConsoleChatCensored() && !mNameData.createdByRestrictedPlayer) ||
        g_pApp->IsUUIDLocal(mNameData.ownerUuid) ||
        (CGameNetworkManager::sInstance.IsHost() && mNameData.createdOnHost))
    {
        return 1;
    }
    return 0;
}

std::wstring ClientboundContainerOpenPacket::getTitle()
{
    if (!hasCustomName())
    {
        return L"";
    }
    return mNameData.name;
}

int ClientboundContainerOpenPacket::getSize()
{
    return mSize;
}

int ClientboundContainerOpenPacket::getEntityId()
{
    return mEntityId;
}

uint32_t ClientboundContainerOpenPacket::isContainer()
{
    return mSize != 0;
}

int ClientboundContainerOpenPacket::getPacketId()
{
    return 100;
}

boost::shared_ptr<Packet> ClientboundContainerOpenPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundContainerOpenPacket());
}
