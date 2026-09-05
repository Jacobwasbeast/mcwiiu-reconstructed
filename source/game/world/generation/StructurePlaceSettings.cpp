#include "game/world/generation/StructurePlaceSettings.hpp"

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

void StructurePlaceSettings::setMirror(Mirror* mirror) { mMirror = mirror; }
void StructurePlaceSettings::setRotation(Rotation* rotation) { mRotation = rotation; }
void StructurePlaceSettings::setIgnoreEntities(bool ignoreEntities) { mIgnoreEntities = ignoreEntities; }
void StructurePlaceSettings::setIgnoreBlock(Block* block) { mIgnoreBlock = block; }

StructurePlaceSettings* StructurePlaceSettings::setBoundingBox(BoundingBox* box)
{
    delete mBoundingBox;
    mBoundingBox = new BoundingBox(box);
    return this;
}

void StructurePlaceSettings::setRandom(Random* random) { mRandom = random; }
void StructurePlaceSettings::setIntegrity(float integrity) { mIntegrity = integrity; }

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

    if (blockId == 0x11 || blockId == 0xa2)
    {
        data = block->convertBlockStateToLegacyData(state);
        key = data & 3;
        if (blockId == 0xa2)
            key += 4;
        entry = mSwapAuxValues.find(key);
        if (entry == mSwapAuxValues.end())
            return state;
        if (entry->second < 4)
            return Blocks::log->getBlockState((data & 0xc) | entry->second);
        return Blocks::log2->getBlockState((data & 0xc) | (entry->second - 4));
    }

    bool isStairs = blockId == 0x35 || blockId == 0x43 || blockId == 0x6c || blockId == 0x6d
        || blockId == 0x72 || blockId == 0x80 || blockId == 0x86 || blockId == 0x87
        || blockId == 0x88 || blockId == 0x9c || blockId == 0xa3 || blockId == 0xa4
        || blockId == 0xb4 || blockId == 0xcb;
    bool isTrapdoor = blockId == 0x60 || (blockId >= 0x112 && blockId <= 0x116);
    bool isStrippedLog = block == Blocks::stripped_log_oak || block == Blocks::stripped_log_spruce
        || block == Blocks::stripped_log_birch || block == Blocks::stripped_log_jungle
        || block == Blocks::stripped_log_acacia || block == Blocks::stripped_log_dark_oak;

    if (!isStairs && !block->mUseSharedIcon && !block->mHasTintedFace && !block->mIsAlwaysRendered
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

    if (blockId == 0x86 || blockId == 0xb7 || blockId == 0xbc || blockId == 0xc1
             || blockId == 0x112 || blockId == 0x127)
        key = 1;
    else if (blockId == 0x87 || blockId == 0xb8 || blockId == 0xbd
             || blockId == 0xc2 || blockId == 0x113 || blockId == 0x128)
        key = 2;
    else if (blockId == 0x88 || blockId == 0xb9 || blockId == 0xbe
             || blockId == 0xc3 || blockId == 0x114 || blockId == 0x129)
        key = 3;
    else if (blockId == 0xa3 || blockId == 0xbb || blockId == 0xc0
             || blockId == 0xc4 || blockId == 0x115 || blockId == 0x12a)
        key = 4;
    else if (blockId == 0xa4 || blockId == 0xba || blockId == 0xbf
             || blockId == 0xc5 || blockId == 0x116 || blockId == 299)
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
    if (block->mUseSharedIcon)
        return door->getBlockState(data);
    if (block->mHasTintedFace)
        return fence->getBlockState(data);
    if (block->mIsAlwaysRendered)
        return fenceGate->getBlockState(data);
    if (isTrapdoor)
        return trapdoor->getBlockState(data);
    if (isStrippedLog)
        return strippedLog->getBlockState(data);
    return block->getBlockState(data);
}

void StructurePlaceSettings::placeWaterBelowSeaLevel(bool placeWater) { mPlaceWaterBelowSeaLevel = placeWater; }
bool StructurePlaceSettings::isPlacingWaterBelowSeaLevel() const { return mPlaceWaterBelowSeaLevel; }
