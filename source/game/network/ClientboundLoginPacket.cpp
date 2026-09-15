#include "game/network/ClientboundLoginPacket.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/LevelType.hpp"

ClientboundLoginPacket::ClientboundLoginPacket()
    : mPlayerUID(0), mPlayerUID2(0)
{
    mLevelName = L"";
    mEntityId = 0;
    mSeed = 0;
    mGameMode = 0;
    mPlayerUID = PlayerUID(0);
    mPlayerUID2 = PlayerUID(0);
    mPlayerFlag = 1;
    mHardcore = false;
    mViewDistance = 0;
    mChunkDistance = 0;
    mReducedDebugInfo = 0;
    mWorldHeight = 0;
    mSeaLevel = 0;
    mAllowCommands = false;
    mIsFlat = false;
    mLevelType = NULL;
    mCurrentTick = 0;
    mXZSize = 54;
    mHellScale = 3;
    mMaxTick = 0x80;
    mIsFlatWorld = true;
    mPlayerFlags = 0;
    mSpawnProtection = 0;
    mSimulationDistance = 0;
    mHasAchievements = false;
    mUnknownValue3 = -1;
    mGameType = 0;
    mDifficultyValue = 0;
    mMaxPlayers = 0;
    mDimension = -1;
    mDifficulty = -1;
}

ClientboundLoginPacket::ClientboundLoginPacket(const std::wstring& levelName, int entityId, LevelType* levelType,
    long long seed, int gameType, int dimension, int difficulty, char gameMode, unsigned char difficultyValue,
    unsigned char maxPlayers, char playerFlag, int chunkDistance, unsigned char reducedDebugInfo, bool isFlat,
    unsigned int currentTick, int xzSize, int hellScale, int maxTick, bool isFlatWorld,
    unsigned char playerFlags, int spawnProtection, int simulationDistance, int unknownValue3)
    : mPlayerUID(0), mPlayerUID2(0)
{
    mLevelName = levelName;
    mEntityId = entityId;
    mSeed = seed;
    mGameMode = gameMode;
    mPlayerUID = PlayerUID(0);
    mPlayerUID2 = PlayerUID(0);
    mPlayerFlag = playerFlag;
    mHardcore = false;
    mViewDistance = 0;
    mChunkDistance = chunkDistance;
    mReducedDebugInfo = reducedDebugInfo;
    mWorldHeight = 0;
    mSeaLevel = 0;
    mAllowCommands = false;
    mIsFlat = isFlat;
    mLevelType = levelType;
    mCurrentTick = static_cast<int>(currentTick);
    mXZSize = xzSize;
    mHellScale = hellScale;
    mMaxTick = maxTick;
    mIsFlatWorld = isFlatWorld;
    mPlayerFlags = playerFlags;
    mSpawnProtection = spawnProtection;
    mSimulationDistance = simulationDistance;
    mHasAchievements = false;
    mUnknownValue3 = unknownValue3;
    mGameType = gameType;
    mDifficultyValue = difficultyValue;
    mMaxPlayers = maxPlayers;
    mDimension = dimension;
    mDifficulty = difficulty;
}

ClientboundLoginPacket::ClientboundLoginPacket(const std::wstring& levelName, int entityId, PlayerUID playerUID,
    PlayerUID playerUID2, bool hardcore, unsigned int viewDistance, unsigned int worldHeight, unsigned int seaLevel,
    bool allowCommands, bool hasAchievements)
    : mPlayerUID(0), mPlayerUID2(0)
{
    mLevelName = levelName;
    mEntityId = entityId;
    mSeed = 0;
    mGameMode = 0;
    mPlayerUID = playerUID;
    mPlayerUID2 = playerUID2;
    mPlayerFlag = 1;
    mHardcore = hardcore;
    mViewDistance = static_cast<int>(viewDistance);
    mChunkDistance = 0;
    mReducedDebugInfo = 0;
    mWorldHeight = static_cast<int>(worldHeight);
    mSeaLevel = static_cast<int>(seaLevel);
    mAllowCommands = allowCommands;
    mIsFlat = false;
    mLevelType = NULL;
    mCurrentTick = 0;
    mXZSize = 54;
    mHellScale = 3;
    mMaxTick = 0x80;
    mIsFlatWorld = true;
    mPlayerFlags = 0;
    mSpawnProtection = 0;
    mSimulationDistance = 0;
    mHasAchievements = hasAchievements;
    mUnknownValue3 = -1;
    mGameType = 0;
    mDifficultyValue = 0;
    mMaxPlayers = 0;
    mDimension = -1;
    mDifficulty = -1;
}

ClientboundLoginPacket::~ClientboundLoginPacket()
{
}

int ClientboundLoginPacket::getPacketId()
{
    return 1;
}

void ClientboundLoginPacket::read(DataInputStream* input)
{
    mEntityId = input->readInt();
    mLevelName = Packet::readUtf(input, 20);
    std::wstring generatorName = Packet::readUtf(input, 16);
    mLevelType = LevelType::getLevelType(generatorName);
    if (mLevelType == NULL)
    {
        mLevelType = LevelType::DEFAULT;
    }
    mSeed = input->readLong();
    mGameType = input->readInt();
    mDimension = input->readInt();
    mDifficulty = input->readInt();
    mGameMode = input->readByte();
    mDifficultyValue = input->readByte();
    mMaxPlayers = input->readByte();
    mPlayerUID = input->readPlayerUID();
    mPlayerUID2 = input->readPlayerUID();
    mHardcore = input->readBoolean();
    mViewDistance = input->readInt();
    mPlayerFlag = input->readByte();
    mChunkDistance = input->readInt();
    mReducedDebugInfo = input->readByte();
    mWorldHeight = input->readInt();
    mSeaLevel = input->readInt();
    mAllowCommands = input->readBoolean();
    mIsFlat = input->readBoolean();
    mCurrentTick = input->readInt();
    mMaxTick = input->readInt();
    mIsFlatWorld = input->readBoolean();
    mHasAchievements = input->readBoolean();
    mPlayerFlags = input->readByte();
    mSpawnProtection = input->readInt();
    mSimulationDistance = input->readInt();
    mUnknownValue3 = input->readInt();
}

void ClientboundLoginPacket::write(DataOutputStream* output)
{
    output->writeInt(mEntityId);
    Packet::writeUtf(mLevelName, output);
    if (mLevelType == NULL)
    {
        Packet::writeUtf(L"", output);
    }
    else
    {
        Packet::writeUtf(mLevelType->getGeneratorName(), output);
    }
    output->writeLong(mSeed);
    output->writeInt(mGameType);
    output->writeInt(mDimension);
    output->writeInt(mDifficulty);
    output->writeByte(mGameMode);
    output->writeByte(mDifficultyValue);
    output->writeByte(mMaxPlayers);
    output->writePlayerUID(mPlayerUID);
    output->writePlayerUID(mPlayerUID2);
    output->writeBoolean(mHardcore);
    output->writeInt(mViewDistance);
    output->writeByte(mPlayerFlag);
    output->writeInt(mChunkDistance);
    output->writeByte(mReducedDebugInfo);
    output->writeInt(mWorldHeight);
    output->writeInt(mSeaLevel);
    output->writeBoolean(mAllowCommands);
    output->writeBoolean(mIsFlat);
    output->writeInt(mCurrentTick);
    output->writeInt(mMaxTick);
    output->writeBoolean(mIsFlatWorld);
    output->writeBoolean(mHasAchievements);
    output->writeByte(mPlayerFlags);
    output->writeInt(mSpawnProtection);
    output->writeInt(mSimulationDistance);
    output->writeInt(mUnknownValue3);
}

void ClientboundLoginPacket::handle(PacketListener* listener)
{
    listener->handleLogin(shared_from_this());
}

int ClientboundLoginPacket::getEstimatedSize()
{
    int generatorSize = 0;
    if (mLevelType != NULL)
    {
        generatorSize = static_cast<int>(mLevelType->getGeneratorName().length());
    }
    return static_cast<int>(mLevelName.length()) + generatorSize + 0x56;
}

boost::shared_ptr<Packet> ClientboundLoginPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundLoginPacket());
}
