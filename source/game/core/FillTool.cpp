#include "game/core/FillTool.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/util/AABB.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"

FillTool::FillTool(ConsoleSchematicFile* schematic)
    : BlockPos(0, 0, 0), mPointB(0, 0, 0), mXDiameter(3), mYDiameter(3),
      mEdgeBlock(NULL), mFillBlock(NULL), mReplaceOnlyBlock(NULL), mEdgeData(0),
      mFillData(0), mReplaceOnlyData(0), mReplaceOnlyDataMask(-1), mFillDataMask(0),
      mDimension(0), mReplaceOnly(false), mHasPointB(false), mZDiameter(3),
      mBrushStrength(2.0f), mEnabled(false), mSurfaceBrush(false), mLevel(NULL),
      mSchematic(schematic), mBounds(AABB::newPermanent(0.0, 0.0, 0.0, 3.0, 6.0, 2.0)),
      mMode(1), mBrushBlock(NULL), mBrushData(0), mUseBrush(false), mUseSurface(false),
      mFallbackBlock(NULL), mX0(0), mY0(0), mZ0(0), mX1(0), mY1(0), mZ1(0),
      mFlags(0), mMaxBlocks(0), mMinY(0), mProgress(0), mFinished(false),
      mUndo(0), mRedo(0), mActive(false)
{
}

FillTool::~FillTool()
{
    delete mBounds;
}

void FillTool::Reset()
{
    mBrushData = 0;
    mReplaceOnlyData = 0;
    mReplaceOnlyDataMask = -1;
    mFillDataMask = 0x00ffffff;
    mReplaceOnlyBlock = NULL;
    mFillBlock = NULL;
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
    if (mSurfaceBrush)
    {
        mBrushBlocks.clear();
        mBrushBlocks.insert(mBrushBlocks.end(), mSurfaceBlocks.begin(), mSurfaceBlocks.end());
    }
    mProgress = mProgress + 1;
    if (mProgress >= mMaxBlocks && mMaxBlocks != 0)
        mActive = false;
}

void FillTool::PopulateSurfaceBrushBlock(boost::shared_ptr<Player> player)
{
    mBrushBlocks.clear();
    if (player)
        mBrushBlocks.insert(mBrushBlocks.end(), mSurfaceBlocks.begin(), mSurfaceBlocks.end());
}

void FillTool::Serialise(DataOutputStream* output)
{
    output->writeBlockPos(*this);
    output->writeBlockPos(mPointB);
    output->writeShort((short)Block::getId(mEdgeBlock));
    output->writeShort((short)mEdgeData);
    output->writeShort((short)Block::getId(mFillBlock));
    output->writeShort((short)mFillData);
    output->writeBoolean(mReplaceOnlyBlock != NULL);
    if (mReplaceOnlyBlock != NULL)
    {
        output->writeShort((short)Block::getId(mReplaceOnlyBlock));
        output->writeShort((short)mReplaceOnlyData);
        output->writeShort((short)mReplaceOnlyDataMask);
    }
    output->writeShort((short)mFillDataMask);
    output->writeShort((short)mDimension);
    output->writeBoolean(mReplaceOnly);
    output->writeBoolean(mHasPointB);
    output->writeShort((short)mBrushData);
    output->writeShort((short)mXDiameter);
    output->writeShort((short)mYDiameter);
    output->writeShort((short)mZDiameter);
    output->writeFloat(mBrushStrength);
    output->writeBoolean(mSurfaceBrush);
    if (mMode == 3)
    {
        output->writeInt((int)mSurfaceBlocks.size());
        std::vector<std::pair<const BlockState*, int> >::const_iterator it;
        for (it = mSurfaceBlocks.begin(); it != mSurfaceBlocks.end(); ++it)
        {
            output->writeUnsignedShort((unsigned short)Block::getElementId(it->first));
            output->writeShort((short)it->second);
        }
    }
}

void FillTool::Deserialise(DataInputStream* input)
{
    Reset();
    *static_cast<BlockPos*>(this) = input->readBlockPos();
    mPointB = input->readBlockPos();
    mEdgeBlock = Block::byId(input->readShort());
    mEdgeData = input->readShort();
    mFillBlock = Block::byId(input->readShort());
    mFillData = input->readShort();
    if (input->readBoolean())
    {
        mReplaceOnlyBlock = Block::byId(input->readShort());
        mReplaceOnlyData = input->readShort();
        mReplaceOnlyDataMask = input->readShort();
    }
    mFillDataMask = input->readShort();
    mDimension = input->readShort();
    mReplaceOnly = input->readBoolean();
    mHasPointB = input->readBoolean();
    mBrushData = input->readShort();
    mXDiameter = input->readShort();
    mYDiameter = input->readShort();
    mZDiameter = input->readShort();
    mBrushStrength = input->readFloat();
    mSurfaceBrush = input->readBoolean();
    mSurfaceBlocks.clear();
    if (mMode == 3)
    {
        int count = input->readInt();
        for (int i = 0; i < count; ++i)
            mSurfaceBlocks.push_back(std::make_pair(Block::getStateByElementId((unsigned short)input->readUnsignedShort()), input->readShort()));
    }
}
