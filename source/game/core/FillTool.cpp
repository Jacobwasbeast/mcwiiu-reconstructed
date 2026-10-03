#include "game/core/FillTool.hpp"

#include <math.h>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Slot.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Math.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockItem.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/Player.hpp"

FillTool::FillTool(ConsoleSchematicFile* schematic)
    : BlockPos(0, 0, 0),
      mPointB(0, 0, 0),
      mEdgeBlock(NULL),
      mFillBlock(NULL),
      mReplaceOnlyBlock(NULL),
      mEdgeData(0),
      mFillData(0),
      mReplaceOnlyData(0),
      mReplaceOnlyDataMask(-1),
      mFillDataMask(-1),
      mDimension(0),
      mReplaceOnly(false),
      mHasPointB(false),
      mHandleDataType(HANDLE_DATA_NONE),
      mShape(1),
      mMode(1),
      mLiquidType(0),
      mField0x50(0),
      mBrushStrength(2.0f),
      mSurfaceBrush(false),
      mField0x6C(false),
      mSchematic(schematic),
      mActive(false)
{
    mPointB.x = 3;
    mPointB.y = 3;
    mPointB.z = 3;
    mOperation.mBounds = AABB::newPermanent(0.0, 0.0, 0.0, 3.0, 3.0, 3.0);
}

FillTool::~FillTool()
{
    delete mOperation.mBounds;
}

void FillTool::Reset()
{
    mHandleDataType = HANDLE_DATA_NONE;
    mReplaceOnlyData = 0;
    mFillDataMask = -1;
    mReplaceOnlyDataMask = 0x00ffffff;
    mReplaceOnlyBlock = NULL;
}

bool FillTool::IsWaterBlock(const BlockState* state)
{
    unsigned int id = Block::getId(state->getBlock());
    return id > 7 && id < 10;
}

bool FillTool::IsLavaBlock(const BlockState* state)
{
    unsigned int id = Block::getId(state->getBlock());
    return id > 9 && id < 12;
}

bool FillTool::IsValidSurfaceBlock(const BlockState* state)
{
    unsigned int id = Block::getId(state->getBlock());
    if (id < 0x6a)
        return id != 0 && (id < 8 || (id > 0xb && (id < 0x11 || id > 0x12)));
    if (id < 0x102)
        return id != 0x6a && id != 0x7f && (id < 0xa1 || id > 0xa2);
    if (id == 0x102)
        return false;
    return id < 0x108 || (id > 0x10c && (id < 0x10e || id > 0x110));
}

void FillTool::tick()
{
    if (!mActive)
        return;

    if (mOperation.mMode == 4)
    {
        AABB* bounds = mOperation.mBounds;
        unsigned int width = (unsigned int)(bounds->mMaxX - bounds->mMinX);
        unsigned int depth = (unsigned int)(bounds->mMaxZ - bounds->mMinZ);
        int baseZ = (int)bounds->mMinZ;
        int baseX = (int)bounds->mMinX;
        float halfWidth = (float)width * 0.5f;
        float halfDepth = (float)depth * 0.5f;
        unsigned int area = width * depth;
        float centreZ = ((float)bounds->mMinZ + halfDepth) - 0.5f;
        float radius = halfDepth;
        if (halfWidth - halfDepth < 0.0f)
            radius = halfWidth;
        float centreX = ((float)bounds->mMinX + halfWidth) - 0.5f;

        int* surface = new int[area];
        for (unsigned int i = 0; i < area; ++i)
            surface[i] = 0;

        unsigned int index = 0;
        int top = -1;
        for (unsigned int z = 0; z < depth; ++z)
        {
            for (unsigned int x = 0; x < width; ++x)
            {
                int height = mOperation.mLevel->getHeightmapHeight(baseX + (int)x, baseZ + (int)z);
                surface[index] = height - 1;
                const BlockState* state = mOperation.mLevel->getBlockState(
                    BlockPos(baseX + (int)x, height - 1, baseZ + (int)z));
                if (state != NULL)
                {
                    if ((mLiquidType == 0 && IsWaterBlock(state)) ||
                        (mLiquidType == 1 && IsLavaBlock(state)))
                    {
                        if (top < surface[index])
                            top = surface[index];
                    }
                }
                ++index;
            }
        }

        if (top > 0)
        {
            const BlockState* liquid = Blocks::water->defaultBlockState();
            if (mLiquidType == 0)
                liquid = Blocks::water->defaultBlockState();
            else if (mLiquidType == 1)
                liquid = Blocks::lava->defaultBlockState();

            unsigned int row = 0;
            for (unsigned int z = 0; z < depth; ++z)
            {
                for (unsigned int x = 0; x < width; ++x)
                {
                    float worldX = (float)x + (float)bounds->mMinX;
                    float worldZ = (float)z + (float)bounds->mMinZ;
                    if ((mOperation.mShape != 1 && mOperation.mShape != 2) ||
                        sqrtf((worldX - centreX) * (worldX - centreX) +
                              (worldZ - centreZ) * (worldZ - centreZ)) < radius)
                    {
                        MutableBlockPos pos((int)worldX, top, (int)worldZ);
                        const BlockState* state = mOperation.mLevel->getBlockState(pos);
                        while (surface[row + x] < pos.y &&
                               (state == NULL || state->getBlock() == Blocks::air))
                        {
                            mOperation.mLevel->setBlock(pos, liquid, Block::UPDATE_CLIENTS, false);
                            pos = MutableBlockPos(pos.below());
                            state = mOperation.mLevel->getBlockState(pos);
                        }
                    }
                }
                row += width;
            }
        }
    }
    else if (mOperation.mMode == 3)
    {
        AABB* bounds = mOperation.mBounds;
        unsigned int width = (unsigned int)(bounds->mMaxX - bounds->mMinX);
        unsigned int depth = (unsigned int)(bounds->mMaxZ - bounds->mMinZ);
        float halfWidth = (float)width * 0.5f;
        float halfDepth = (float)depth * 0.5f;
        float radius = halfDepth;
        if (halfWidth - halfDepth < 0.0f)
            radius = halfWidth;
        float centreZ = ((float)bounds->mMinZ + halfDepth) - 0.5f;
        float centreX = ((float)bounds->mMinX + halfWidth) - 0.5f;

        for (unsigned int z = 0; z < depth; ++z)
        {
            for (unsigned int x = 0; x < width; ++x)
            {
                float worldX = (float)x + (float)bounds->mMinX;
                float worldZ = (float)z + (float)bounds->mMinZ;
                if ((mOperation.mShape != 1 && mOperation.mShape != 2) ||
                    sqrtf((worldX - centreX) * (worldX - centreX) +
                          (worldZ - centreZ) * (worldZ - centreZ)) < radius)
                {
                    MutableBlockPos pos(
                        mOperation.mLevel->getHeightmapPos(BlockPos((int)worldX, 0, (int)worldZ)).below());
                    while (pos.y > 0)
                    {
                        const BlockState* state = mOperation.mLevel->getBlockState(pos);
                        if (IsValidSurfaceBlock(state))
                            break;
                        pos = MutableBlockPos(pos.below());
                    }
                    std::vector<std::pair<const BlockState*, int> >::const_iterator layer =
                        mOperation.mBrushBlocks.begin();
                    while (layer != mOperation.mBrushBlocks.end() && pos.y > 0)
                    {
                        const BlockState* brush = layer->first;
                        int thickness = layer->second;
                        for (int i = 0; i < thickness && pos.y > 0; ++i)
                        {
                            const BlockState* state = mOperation.mLevel->getBlockState(pos);
                            if (IsValidSurfaceBlock(state))
                                mOperation.mLevel->setBlock(pos, brush, Block::UPDATE_CLIENTS, false);
                            pos = MutableBlockPos(pos.below());
                        }
                        ++layer;
                    }
                }
            }
        }
    }
    else if (mOperation.mMode == 2)
    {
        AABB* bounds = mOperation.mBounds;
        int width = (int)(bounds->mMaxX - bounds->mMinX);
        double strength = (double)mBrushStrength;
        int depth = (int)(bounds->mMaxZ - bounds->mMinZ);
        int radius = (int)ceil(strength * 2.57);
        int baseX = (int)bounds->mMinX;
        int baseZ = (int)bounds->mMinZ;
        int stride = width + radius * 2 + 1;
        unsigned int cells = (unsigned int)(stride * (depth + radius * 2 + 1));

        int* heights = new int[cells];
        for (unsigned int i = 0; i < cells; ++i)
            heights[i] = 0;

        int cursor = 0;
        for (int sampleZ = -radius; sampleZ < depth + radius + 1; ++sampleZ)
        {
            for (int sampleX = -radius; sampleX < width + radius + 1; ++sampleX)
            {
                MutableBlockPos pos(
                    mOperation.mLevel->getHeightmapPos(BlockPos(baseX + sampleX, 0, baseZ + sampleZ)).below());
                if (mOperation.mSkipSurfaceWater)
                {
                    while (pos.y > 0)
                    {
                        const BlockState* state = mOperation.mLevel->getBlockState(pos);
                        if (!IsWaterBlock(state))
                        {
                            const BlockState* extra = mOperation.mLevel->getExtraBlockState(pos);
                            if (!IsWaterBlock(extra))
                                break;
                        }
                        pos = MutableBlockPos(pos.below());
                    }
                }
                heights[cursor] = pos.y;
                ++cursor;
            }
        }

        int row = radius * stride + radius;
        for (int z = 0; z < depth; ++z)
        {
            for (int x = 0; x < width; ++x)
            {
                int centre = row + x;
                MutableBlockPos pos((int)bounds->mMinX + x, heights[centre], (int)bounds->mMinZ + z);
                float weightTotal = 0.0f;
                float heightTotal = 0.0f;
                for (int kernelZ = z - radius; kernelZ < z + radius + 1; ++kernelZ)
                {
                    for (int kernelX = x - radius; kernelX < x + radius + 1; ++kernelX)
                    {
                        int offsetX = kernelX - x;
                        int offsetZ = kernelZ - z;
                        float weight;
                        if (mOperation.mSmoothType == 0)
                        {
                            weight = expf((float)(-(offsetX * offsetX + offsetZ * offsetZ)) /
                                          (float)((double)(float)(strength + strength) * strength));
                            weight = weight / (float)((double)(float)(6.2831855f * strength) * strength);
                        }
                        else
                        {
                            weight = 1.0f;
                        }
                        weightTotal = weightTotal + weight;
                        heightTotal = (float)heights[(kernelZ + radius) * stride + kernelX + radius] * weight +
                                      heightTotal;
                    }
                }
                int target = Math::round(heightTotal / weightTotal);
                const BlockState* state = mOperation.mLevel->getBlockState(pos);
                while (target < pos.y)
                {
                    mOperation.mLevel->removeBlock(pos);
                    pos.setY(pos.y - 1);
                }
                mOperation.mLevel->setBlock(pos, state, Block::UPDATE_CLIENTS, false);
                pos.setY(pos.y - 1);
                while (heights[centre] < pos.y)
                {
                    mOperation.mLevel->setBlock(pos, state, Block::UPDATE_CLIENTS, false);
                    pos.setY(pos.y - 1);
                }
            }
            row += stride;
        }
    }
    else if (mOperation.mChunkX <= mOperation.mChunkMaxX && mOperation.mChunkZ <= mOperation.mChunkMaxZ)
    {
        LevelChunk* chunk = mOperation.mLevel->getChunk(mOperation.mChunkX, mOperation.mChunkZ);
        if (!chunk->isEmpty())
        {
            AABB* bounds = mOperation.mBounds;

            double minX = floor(bounds->mMinX);
            double chunkMinX = (double)chunk->x * 16.0;
            if (minX < chunkMinX)
                minX = chunkMinX;
            int startX = (int)minX;
            double maxX = floor(bounds->mMaxX);
            double chunkMaxX = (double)((startX >> 4) << 4) + 16.0;
            if (chunkMaxX < maxX)
                maxX = chunkMaxX;
            int endX = (int)maxX;

            int startY = (int)floor(bounds->mMinY);
            int endY = (int)floor(bounds->mMaxY);
            if (startY < 0)
                startY = 0;
            if (startY > 255)
                startY = 255;
            if (endY < 0)
                endY = 0;
            if (endY > 255)
                endY = 255;

            int startZ = (int)floor(bounds->mMinZ);
            if (startZ < chunk->z * 16)
                startZ = chunk->z * 16;
            int endZ = (int)floor(bounds->mMaxZ);
            if ((startZ >> 4) * 16 + 16 < endZ)
                endZ = (startZ >> 4) * 16 + 16;

            float halfHeight = (float)(endY - startY) * 0.5f;
            float halfDepth = (float)(bounds->mMaxZ - bounds->mMinZ) * 0.5f;
            float halfWidth = (float)(bounds->mMaxX - bounds->mMinX) * 0.5f;
            float radius = halfHeight;
            if (halfDepth < halfHeight)
                radius = halfDepth;
            float centreX = ((float)bounds->mMinX + halfWidth) - 0.5f;
            if (halfWidth - radius < 0.0f)
                radius = halfWidth;
            float centreZ = ((float)bounds->mMinZ + halfDepth) - 0.5f;
            float centreY = ((float)startY + halfHeight) - 0.5f;

            int doubleSlabId = -1;
            if (mOperation.mReplaceOnlyBlock != NULL && HalfSlabBlock::isHalfSlab(mOperation.mReplaceOnlyBlock))
                doubleSlabId = Block::getId(
                    HalfSlabBlock::GetDoubleSlabBlockFromSlab(mOperation.mReplaceOnlyBlock));

            MutableBlockPos pos(0, 0, 0);
            for (int y = startY; y < endY; ++y)
            {
                pos.setY(y);
                for (int x = startX; x < endX; ++x)
                {
                    pos.setX(x);
                    for (int z = startZ; z < endZ; ++z)
                    {
                        pos.setZ(z);
                        bool edge = false;
                        if (mOperation.mShape == 1)
                        {
                            float extent = radius * radius;
                            float deltaX = (float)x - centreX;
                            float deltaY = (float)y - centreY;
                            float deltaZ = (float)z - centreZ;
                            float distance = (deltaX * deltaX) / extent + (deltaY * deltaY) / extent +
                                             (deltaZ * deltaZ) / extent;
                            if (distance >= 1.0f)
                            {
                                if (distance > 1.0f)
                                    continue;
                                edge = true;
                            }
                        }
                        else if (mOperation.mShape == 2)
                        {
                            float deltaX = (float)x - centreX;
                            float deltaY = (float)y - centreY;
                            float deltaZ = (float)z - centreZ;
                            float distance = (deltaX * deltaX) / (halfWidth * halfWidth) +
                                             (deltaY * deltaY) / (halfHeight * halfHeight) +
                                             (deltaZ * deltaZ) / (halfDepth * halfDepth);
                            if (distance >= 1.0f)
                            {
                                if (distance > 1.0f)
                                    continue;
                                edge = true;
                            }
                        }
                        else if (y == startY || y == endY - 1 || x == startX || x == endX - 1 ||
                                 z == startZ || z == endZ - 1)
                        {
                            edge = true;
                        }

                        const BlockState* state = mOperation.mLevel->getBlockState(BlockPos(x, y, z));
                        Block* block = state->getBlock();
                        int id = Block::getId(block);
                        int data = block->convertBlockStateToLegacyData(state);
                        if ((mOperation.mIgnoreAir && id == 0) ||
                            (mOperation.mIgnoreWater && (id == 9 || id == 8)))
                            continue;
                        int mask = mOperation.mFillDataMask;
                        if (mOperation.mHandleDataType == 1)
                        {
                            if (mOperation.mReplaceOnlyBlock != NULL && id == mOperation.mReplaceOnlyBlockId)
                            {
                                mOperation.mLevel->setExtraBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                                mOperation.mLevel->setBlock(
                                    pos, mOperation.mFillBlock->getBlockState(data & mask), Block::UPDATE_CLIENTS, false);
                            }
                            continue;
                        }
                        if (mOperation.mHandleDataType == 2)
                        {
                            if (mOperation.mReplaceOnlyBlock != NULL &&
                                (HalfSlabBlock::isHalfSlab(block) || HalfSlabBlock::isFullSlab(block)) &&
                                (mask & (mOperation.mReplaceOnlyData ^ data)) == 0)
                            {
                                if (id == mOperation.mReplaceOnlyBlockId)
                                {
                                    mOperation.mLevel->setExtraBlock(
                                        pos, Blocks::air->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                                    mOperation.mLevel->setBlock(
                                        pos,
                                        mOperation.mFillBlock->getBlockState(
                                            (data & ~mask) | (mOperation.mFillData & mask)),
                                        Block::UPDATE_CLIENTS, false);
                                }
                                else if (id == doubleSlabId)
                                {
                                    Block* doubleSlab =
                                        HalfSlabBlock::GetDoubleSlabBlockFromSlab(mOperation.mFillBlock);
                                    if (doubleSlab != NULL)
                                    {
                                        mOperation.mLevel->setExtraBlock(
                                            pos, Blocks::air->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                                        mOperation.mLevel->setBlock(
                                            pos,
                                            doubleSlab->getBlockState(
                                                (data & ~mask) | (mOperation.mFillData & mask)),
                                            Block::UPDATE_CLIENTS, false);
                                    }
                                }
                            }
                            continue;
                        }
                        if (mOperation.mReplaceOnlyBlock != NULL &&
                            (id != mOperation.mReplaceOnlyBlockId ||
                             (mOperation.mReplaceOnlyDataMask &
                              (mOperation.mLevel->getBlockData(x, y, z) ^ mOperation.mReplaceOnlyData)) != 0))
                            continue;
                        if (edge)
                        {
                            mOperation.mLevel->setExtraBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                            mOperation.mLevel->setBlock(
                                pos, mOperation.mEdgeBlock->getBlockState(mOperation.mEdgeData & mask), Block::UPDATE_CLIENTS, false);
                        }
                        else
                        {
                            mOperation.mLevel->setExtraBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                            mOperation.mLevel->setBlock(
                                pos, mOperation.mFillBlock->getBlockState(mOperation.mFillData & mask), Block::UPDATE_CLIENTS, false);
                        }
                    }
                }
            }
        }

        int nextZ = mOperation.mChunkZ + 1;
        if (nextZ > mOperation.mChunkMaxZ)
        {
            mOperation.mChunkX = mOperation.mChunkX + 1;
            mOperation.mChunkZ = mOperation.mChunkMinZ;
        }
        else
        {
            mOperation.mChunkZ = nextZ;
        }
        return;
    }

    mActive = false;
}

void FillTool::PopulateSurfaceBrushBlock(boost::shared_ptr<Player> player)
{
    mSurfaceBlocks.clear();
    boost::shared_ptr<Inventory> inventory = player->mInventory;
    for (int i = 0; i < 9; ++i)
    {
        Slot* slot = player->mInventoryMenu->getSlot(36 + i);
        not_null_ptr<ItemInstance> item = slot->getItem();
        if (item->isEmpty())
            break;
        BlockItem* blockItem = dynamic_cast<BlockItem*>(item->getItem());
        if (blockItem == NULL)
            break;
        Block* block = blockItem->getBlock();
        const BlockState* state =
            block->getBlockState(blockItem->getLevelDataForAuxValue(item->getAuxValue()));
        mSurfaceBlocks.push_back(std::make_pair(state, item->getCount()));
    }
}

void FillTool::Serialise(DataOutputStream* output)
{
    output->writeBlockPos(*this);
    output->writeBlockPos(mPointB);
    output->writeInt(Block::getId(mEdgeBlock));
    output->writeInt(mEdgeData);
    output->writeInt(Block::getId(mFillBlock));
    output->writeInt(mFillData);
    if (mReplaceOnlyBlock == NULL)
    {
        output->writeBoolean(false);
    }
    else
    {
        output->writeBoolean(true);
        output->writeInt(Block::getId(mReplaceOnlyBlock));
        output->writeInt(mReplaceOnlyData);
        output->writeInt(mReplaceOnlyDataMask);
    }
    output->writeInt(mFillDataMask);
    output->writeInt(mDimension);
    output->writeBoolean(mReplaceOnly);
    output->writeBoolean(mHasPointB);
    output->writeInt(mHandleDataType);
    output->writeInt(mShape);
    output->writeInt(mMode);
    output->writeInt(mLiquidType);
    output->writeInt(mField0x50);
    output->writeFloat(mBrushStrength);
    output->writeBoolean(mSurfaceBrush);
    if (mMode == 3)
    {
        output->writeInt((int)mSurfaceBlocks.size());
        std::vector<std::pair<const BlockState*, int> >::const_iterator entry;
        for (entry = mSurfaceBlocks.begin(); entry != mSurfaceBlocks.end(); ++entry)
        {
            output->writeUnsignedShort((unsigned short)Block::getElementId(entry->first));
            output->writeInt(entry->second);
        }
    }
}

void FillTool::Deserialise(DataInputStream* input)
{
    Reset();
    *static_cast<BlockPos*>(this) = MutableBlockPos(input->readBlockPos());
    mPointB = MutableBlockPos(input->readBlockPos());
    mEdgeBlock = Block::byId(input->readInt());
    mEdgeData = input->readInt();
    mFillBlock = Block::byId(input->readInt());
    mFillData = input->readInt();
    if (input->readBoolean())
    {
        mReplaceOnlyBlock = Block::byId(input->readInt());
        mReplaceOnlyData = input->readInt();
        mReplaceOnlyDataMask = input->readInt();
    }
    mFillDataMask = input->readInt();
    mDimension = input->readInt();
    mReplaceOnly = input->readBoolean();
    mHasPointB = input->readBoolean();
    mHandleDataType = input->readInt();
    mShape = input->readInt();
    mMode = input->readInt();
    mLiquidType = input->readInt();
    mField0x50 = input->readInt();
    mBrushStrength = input->readFloat();
    mSurfaceBrush = input->readBoolean();
    if (mMode == 3)
    {
        mSurfaceBlocks.clear();
        int count = input->readInt();
        for (int i = 0; i < count; ++i)
        {
            unsigned short elementId = input->readUnsignedShort();
            int thickness = input->readInt();
            mSurfaceBlocks.push_back(std::make_pair(Block::getStateByElementId(elementId), thickness));
        }
    }
}
