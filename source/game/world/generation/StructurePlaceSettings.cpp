#include "game/world/generation/StructurePlaceSettings.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/core/BoundingBox.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/core/System.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"

#include <stdint.h>

static std::wstring mirrorName(Mirror* mirror)
{
    if (mirror == Mirror::LEFT_RIGHT)
        return L"mirror_left_right";
    if (mirror == Mirror::FRONT_BACK)
        return L"mirror_front_back";
    return L"no_mirror";
}

static std::wstring rotationName(Rotation* rotation)
{
    if (rotation == Rotation::CLOCKWISE_90)
        return L"rotate_90";
    if (rotation == Rotation::CLOCKWISE_180)
        return L"rotate_180";
    if (rotation == Rotation::COUNTERCLOCKWISE_90)
        return L"rotate_270";
    return L"rotate_0";
}

StructurePlaceSettings::StructurePlaceSettings()
{
    mMirror = Mirror::NONE;
    mRotation = Rotation::NONE;
    mIgnoreEntities = false;
    mIgnoreBlock = NULL;
    mChunkPos = NULL;
    mBoundingBox = NULL;
    mIgnoreStructureBlocks = true;
    mIntegrity = 1.0f;
    mRandom = NULL;
    mSeed = -1LL;
    mPlaceWaterBelowSeaLevel = false;
}

StructurePlaceSettings::~StructurePlaceSettings()
{
    delete mBoundingBox;
    delete mChunkPos;
}

void StructurePlaceSettings::read(DataInputStream* input)
{
    if (StructureFeatureIO::getVersion() > 2)
    {
        mMirror = Mirror::byName(input->readUTF());
        mRotation = Rotation::byName(input->readUTF());
        mIgnoreEntities = input->readBoolean();
        mIgnoreStructureBlocks = input->readBoolean();
        mIntegrity = input->readFloat();
        mSeed = input->readLong();

        if (input->readBoolean())
        {
            if (mBoundingBox == NULL)
                mBoundingBox = new BoundingBox();
            mBoundingBox->read(input);
        }

        if (StructureFeatureIO::getVersion() > 3)
        {
            mPlaceWaterBelowSeaLevel = input->readBoolean();
            int count = input->readInt();
            for (int i = 0; i < count; ++i)
                addSwapAuxValue(input->readInt(), input->readInt());
        }
    }
}

void StructurePlaceSettings::write(DataOutputStream* output)
{
    output->writeUTF(mirrorName(mMirror));
    output->writeUTF(rotationName(mRotation));
    output->writeBoolean(mIgnoreEntities);
    output->writeBoolean(mIgnoreStructureBlocks);
    output->writeFloat(mIntegrity);
    output->writeLong(mSeed);
    output->writeBoolean(mBoundingBox != NULL);
    if (mBoundingBox != NULL)
        mBoundingBox->write(output);
    output->writeBoolean(mPlaceWaterBelowSeaLevel);
    output->writeInt(static_cast<int>(mSwapAuxValues.size()));
    boost::unordered_map<int, int>::const_iterator it = mSwapAuxValues.begin();
    for (; it != mSwapAuxValues.end(); ++it)
    {
        output->writeInt(it->first);
        output->writeInt(it->second);
    }
}

StructurePlaceSettings* StructurePlaceSettings::copy()
{
    StructurePlaceSettings* settings = new StructurePlaceSettings();
    settings->mMirror = mMirror;
    settings->mRotation = mRotation;
    settings->mIgnoreEntities = mIgnoreEntities;
    settings->mIgnoreBlock = mIgnoreBlock;
    settings->mChunkPos = mChunkPos;
    settings->mBoundingBox = mBoundingBox;
    settings->mIgnoreStructureBlocks = mIgnoreStructureBlocks;
    settings->mIntegrity = mIntegrity;
    settings->mRandom = mRandom;
    settings->mSeed = mSeed;
    settings->mSwapAuxValues = mSwapAuxValues;
    settings->mPlaceWaterBelowSeaLevel = mPlaceWaterBelowSeaLevel;
    return settings;
}

StructurePlaceSettings* StructurePlaceSettings::setMirror(Mirror* mirror) { mMirror = mirror; return this; }
StructurePlaceSettings* StructurePlaceSettings::setRotation(Rotation* rotation) { mRotation = rotation; return this; }
StructurePlaceSettings* StructurePlaceSettings::setIgnoreEntities(bool ignoreEntities) { mIgnoreEntities = ignoreEntities; return this; }
StructurePlaceSettings* StructurePlaceSettings::setIgnoreBlock(Block* block) { mIgnoreBlock = block; return this; }

StructurePlaceSettings* StructurePlaceSettings::setBoundingBox(BoundingBox* box)
{
    delete mBoundingBox;
    mBoundingBox = new BoundingBox(box);
    return this;
}

StructurePlaceSettings* StructurePlaceSettings::setRandom(Random* random) { mRandom = random; return this; }
StructurePlaceSettings* StructurePlaceSettings::setIntegrity(float integrity) { mIntegrity = integrity; return this; }

Mirror* StructurePlaceSettings::getMirror() const { return mMirror; }
Rotation* StructurePlaceSettings::getRotation() const { return mRotation; }

Random* StructurePlaceSettings::getRandom(const BlockPos* position, bool& generated)
{
    if (mRandom != NULL)
    {
        generated = false;
        return mRandom;
    }

    generated = true;
    if (mSeed < 0 && position != NULL)
    {
        uint32_t x = static_cast<uint32_t>(position->x);
        uint32_t z = static_cast<uint32_t>(position->z);
        uint32_t seed = x * x * 0x4c1906U + x * 0x5ac0dbU
            + z * z * 0x4307a7U + z * 0x5f24fU;
        int32_t signedSeed = static_cast<int32_t>(seed ^ 0x3ad8025fU);
        return new Random(static_cast<long long>(signedSeed));
    }
    if (mSeed != 0)
        return new Random();
    return new Random(System::processTimeInMilliSecs());
}

float StructurePlaceSettings::getIntegrity() { return mIntegrity; }
bool StructurePlaceSettings::isIgnoreEntities() { return mIgnoreEntities; }
Block* StructurePlaceSettings::getIgnoreBlock() { return mIgnoreBlock; }

BoundingBox* StructurePlaceSettings::calculateBoundingBox(ChunkPos* chunkPos)
{
    if (chunkPos != NULL)
    {
        delete mBoundingBox;
        mBoundingBox = new BoundingBox(chunkPos->x * 16, 0, chunkPos->z * 16,
                                       chunkPos->x * 16 + 15, 255, chunkPos->z * 16 + 15);
    }
    return mBoundingBox;
}

void StructurePlaceSettings::updateBoundingBoxFromChunkPos()
{
    mBoundingBox = calculateBoundingBox(mChunkPos);
}

BoundingBox* StructurePlaceSettings::getBoundingBox()
{
    if (mBoundingBox == NULL && mChunkPos != NULL)
        updateBoundingBoxFromChunkPos();
    return mBoundingBox;
}

bool StructurePlaceSettings::isIgnoreStructureBlocks() { return mIgnoreStructureBlocks; }

void StructurePlaceSettings::addSwapAuxValue(int blockId, int auxValue)
{
    mSwapAuxValues[blockId] = auxValue;
}

const BlockState* StructurePlaceSettings::getSwappedBlock(const BlockState* state)
{
    Block* block = state->getBlock();
    int blockId = Block::getId(block);
    boost::unordered_map<int, int>::const_iterator entry;
    int data;
    int key;

    if (blockId == BlockId::LOG || blockId == BlockId::LOG2)
    {
        data = block->convertBlockStateToLegacyData(state);
        key = data & 3;
        if (blockId == BlockId::LOG2)
            key += 4;
        entry = mSwapAuxValues.find(key);
        if (entry == mSwapAuxValues.end())
            return state;
        if (entry->second < 4)
            return Blocks::log->getBlockState((data & 0xc) | entry->second);
        return Blocks::log2->getBlockState((data & 0xc) | (entry->second - 4));
    }

    bool isStairs = blockId == BlockId::OAK_STAIRS || blockId == BlockId::STONE_STAIRS
        || blockId == BlockId::BRICK_STAIRS || blockId == BlockId::STONE_BRICK_STAIRS
        || blockId == BlockId::NETHER_BRICK_STAIRS || blockId == BlockId::SANDSTONE_STAIRS
        || blockId == BlockId::SPRUCE_STAIRS || blockId == BlockId::BIRCH_STAIRS
        || blockId == BlockId::JUNGLE_STAIRS || blockId == BlockId::QUARTZ_STAIRS
        || blockId == BlockId::ACACIA_STAIRS || blockId == BlockId::DARK_OAK_STAIRS
        || blockId == BlockId::RED_SANDSTONE_STAIRS || blockId == BlockId::PURPUR_STAIRS;
    bool isTrapdoor = blockId == BlockId::TRAPDOOR
        || (blockId >= BlockId::SPRUCE_TRAPDOOR && blockId <= BlockId::DARK_OAK_TRAPDOOR);
    bool isStrippedLog = block == Blocks::stripped_log_oak || block == Blocks::stripped_log_spruce
        || block == Blocks::stripped_log_birch || block == Blocks::stripped_log_jungle
        || block == Blocks::stripped_log_acacia || block == Blocks::stripped_log_dark_oak;

    if (!isStairs && !block->mIsDoor && !block->mIsFence && !block->mIsFenceGate
        && !isTrapdoor && !isStrippedLog)
    {
        if (block == Blocks::wooden_slab || block == Blocks::double_wooden_slab)
        {
            data = block->convertBlockStateToLegacyData(state);
            key = data & 7;
            entry = mSwapAuxValues.find(key);
            if (entry == mSwapAuxValues.end())
                return state;
            return block->getBlockState((data & 8) | entry->second);
        }
        if (block != Blocks::planks)
            return state;
        key = block->convertBlockStateToLegacyData(state);
        entry = mSwapAuxValues.find(key);
        if (entry == mSwapAuxValues.end())
            return state;
        return block->getBlockState(entry->second);
    }

    if (blockId == BlockId::SPRUCE_STAIRS || blockId == BlockId::SPRUCE_FENCE_GATE
        || blockId == BlockId::SPRUCE_FENCE || blockId == BlockId::SPRUCE_DOOR
        || blockId == BlockId::SPRUCE_TRAPDOOR || blockId == BlockId::STRIPPED_LOG_SPRUCE)
        key = 1;
    else if (blockId == BlockId::BIRCH_STAIRS || blockId == BlockId::BIRCH_FENCE_GATE
        || blockId == BlockId::BIRCH_FENCE || blockId == BlockId::BIRCH_DOOR
        || blockId == BlockId::BIRCH_TRAPDOOR || blockId == BlockId::STRIPPED_LOG_BIRCH)
        key = 2;
    else if (blockId == BlockId::JUNGLE_STAIRS || blockId == BlockId::JUNGLE_FENCE_GATE
        || blockId == BlockId::JUNGLE_FENCE || blockId == BlockId::JUNGLE_DOOR
        || blockId == BlockId::JUNGLE_TRAPDOOR || blockId == BlockId::STRIPPED_LOG_JUNGLE)
        key = 3;
    else if (blockId == BlockId::ACACIA_STAIRS || blockId == BlockId::ACACIA_FENCE_GATE
        || blockId == BlockId::ACACIA_FENCE || blockId == BlockId::ACACIA_DOOR
        || blockId == BlockId::ACACIA_TRAPDOOR || blockId == BlockId::STRIPPED_LOG_ACACIA)
        key = 4;
    else if (blockId == BlockId::DARK_OAK_STAIRS || blockId == BlockId::DARK_OAK_FENCE_GATE
        || blockId == BlockId::DARK_OAK_FENCE || blockId == BlockId::DARK_OAK_DOOR
        || blockId == BlockId::DARK_OAK_TRAPDOOR || blockId == BlockId::STRIPPED_LOG_DARK_OAK)
        key = 5;
    else
        key = 0;

    entry = mSwapAuxValues.find(key);
    if (entry == mSwapAuxValues.end())
        return state;

    Block* door = Blocks::wooden_door;
    Block* fence = Blocks::fence;
    Block* fenceGate = Blocks::fence_gate;
    Block* stairs = Blocks::oak_stairs;
    Block* trapdoor = Blocks::trapdoor;
    Block* strippedLog = Blocks::stripped_log_oak;

    if (entry->second == 1)
    {
        door = Blocks::spruce_door;
        fence = Blocks::spruce_fence;
        fenceGate = Blocks::spruce_fence_gate;
        stairs = Blocks::spruce_stairs;
        trapdoor = Blocks::spruce_trapdoor;
        strippedLog = Blocks::stripped_log_spruce;
    }
    else if (entry->second == 2)
    {
        door = Blocks::birch_door;
        fence = Blocks::birch_fence;
        fenceGate = Blocks::birch_fence_gate;
        stairs = Blocks::birch_stairs;
        trapdoor = Blocks::birch_trapdoor;
        strippedLog = Blocks::stripped_log_birch;
    }
    else if (entry->second == 3)
    {
        door = Blocks::jungle_door;
        fence = Blocks::jungle_fence;
        fenceGate = Blocks::jungle_fence_gate;
        stairs = Blocks::jungle_stairs;
        trapdoor = Blocks::jungle_trapdoor;
        strippedLog = Blocks::stripped_log_jungle;
    }
    else if (entry->second == 4)
    {
        door = Blocks::acacia_door;
        fence = Blocks::acacia_fence;
        fenceGate = Blocks::acacia_fence_gate;
        stairs = Blocks::acacia_stairs;
        trapdoor = Blocks::acacia_trapdoor;
        strippedLog = Blocks::stripped_log_acacia;
    }
    else if (entry->second == 5)
    {
        door = Blocks::dark_oak_door;
        fence = Blocks::dark_oak_fence;
        fenceGate = Blocks::dark_oak_fence_gate;
        stairs = Blocks::dark_oak_stairs;
        trapdoor = Blocks::dark_oak_trapdoor;
        strippedLog = Blocks::stripped_log_dark_oak;
    }

    data = block->convertBlockStateToLegacyData(state);
    if (isStairs)
        return stairs->getBlockState(data);
    if (block->mIsDoor)
        return door->getBlockState(data);
    if (block->mIsFence)
        return fence->getBlockState(data);
    if (block->mIsFenceGate)
        return fenceGate->getBlockState(data);
    if (isTrapdoor)
        return trapdoor->getBlockState(data);
    if (isStrippedLog)
        return strippedLog->getBlockState(data);
    return block->getBlockState(data);
}

void StructurePlaceSettings::placeWaterBelowSeaLevel(bool placeWater) { mPlaceWaterBelowSeaLevel = placeWater; }
bool StructurePlaceSettings::isPlacingWaterBelowSeaLevel() const { return mPlaceWaterBelowSeaLevel; }
