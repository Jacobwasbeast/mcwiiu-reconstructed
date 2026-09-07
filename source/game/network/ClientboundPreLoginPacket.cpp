#include "game/network/ClientboundPreLoginPacket.hpp"

#include <memory>
#include <new>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/generation/PlayerUID.hpp"

extern CMinecraftApp* g_pApp;

namespace
{
    static void constructPlayerUID(PlayerUID* destination)
    {
        new (destination) PlayerUID(0);
    }
}

ClientboundPreLoginPacket::ClientboundPreLoginPacket()
    : mPlayers(0), mPlayerCount(0), mGameMode(0), mSeed(0),
      mHostPlayer(0), mDifficulty(0), mGameType(0), mProtocolVersion(0), mHardcore(false),
      mMiniGameId(static_cast<EMiniGameId>(7)), mName(L""), mMaxPlayers(0)
{
    unsigned int index;
    for (index = 0; index < 14; ++index)
        mUniqueMapName[index] = 0;
}

ClientboundPreLoginPacket::ClientboundPreLoginPacket(std::wstring name, PlayerUID* players,
    unsigned int playerCount, unsigned char gameMode, unsigned int seed, char* uniqueMapName,
    unsigned int hostPlayer, unsigned int maxPlayers, unsigned char difficulty, unsigned int gameType,
    bool hardcore, EMiniGameId miniGameId)
    : mPlayers(players), mPlayerCount(playerCount), mGameMode(gameMode), mSeed(seed),
      mHostPlayer(hostPlayer), mDifficulty(difficulty),
      mGameType(gameType), mProtocolVersion(0), mHardcore(hardcore), mMiniGameId(miniGameId), mName(name),
      mMaxPlayers(static_cast<unsigned char>(maxPlayers))
{
    unsigned int index;
    for (index = 0; index < 14; ++index)
        mUniqueMapName[index] = uniqueMapName[index];
}

ClientboundPreLoginPacket::~ClientboundPreLoginPacket()
{
    if (mPlayers != 0)
    {
        std::allocator<PlayerUID> allocator;
        unsigned int index;
        for (index = 0; index < mPlayerCount; ++index)
            allocator.destroy(mPlayers + index);
        allocator.deallocate(mPlayers, mPlayerCount);
    }
}

void ClientboundPreLoginPacket::read(DataInputStream* input)
{
    unsigned int index;
    mProtocolVersion = input->readShort();
    mName = Packet::readUtf(input, 32);
    mGameMode = input->readByte();
    mSeed = input->readInt();
    mPlayerCount = static_cast<unsigned int>(input->readByte());
    if (mPlayers != 0)
    {
        std::allocator<PlayerUID> allocator;
        unsigned int oldIndex;
        for (oldIndex = 0; oldIndex < mPlayerCount; ++oldIndex)
            allocator.destroy(mPlayers + oldIndex);
        allocator.deallocate(mPlayers, mPlayerCount);
    }
    mPlayers = 0;
    if (mPlayerCount != 0)
    {
        std::allocator<PlayerUID> allocator;
        mPlayers = allocator.allocate(mPlayerCount);
        for (index = 0; index < mPlayerCount; ++index)
            constructPlayerUID(mPlayers + index);
        for (index = 0; index < mPlayerCount; ++index)
            mPlayers[index] = input->readPlayerUID();
    }
    for (index = 0; index < 14; ++index)
        mUniqueMapName[index] = static_cast<char>(input->readByte());
    mHostPlayer = input->readInt();
    mDifficulty = input->readByte();
    mGameType = input->readInt();
    mHardcore = input->readBoolean();
    mMiniGameId = static_cast<EMiniGameId>(input->readInt());
    g_pApp->SetUniqueMapName(mUniqueMapName);
}

void ClientboundPreLoginPacket::write(DataOutputStream* output)
{
    unsigned int index;
    output->write(2177);
    Packet::writeUtf(mName, output);
    output->writeByte(mGameMode);
    output->writeInt(mSeed);
    output->writeByte(mMaxPlayers);
    for (index = 0; index < mPlayerCount; ++index)
        output->writePlayerUID(mPlayers[index]);
    for (index = 0; index < 14; ++index)
        output->writeByte(static_cast<unsigned char>(mUniqueMapName[index]));
    output->writeInt(mHostPlayer);
    output->writeByte(mDifficulty);
    output->writeInt(mGameType);
    output->writeBoolean(mHardcore);
    output->writeInt(static_cast<unsigned int>(mMiniGameId));
}

void ClientboundPreLoginPacket::handle(PacketListener* listener)
{
    listener->handlePreLogin(shared_from_this());
}

int ClientboundPreLoginPacket::getEstimatedSize()
{
    return static_cast<int>(mName.length()) + 0x28;
}

boost::shared_ptr<Packet> ClientboundPreLoginPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundPreLoginPacket());
}

int ClientboundPreLoginPacket::getPacketId()
{
    return 2;
}
