#ifndef GAME_CORE_FILLTOOL_HPP
#define GAME_CORE_FILLTOOL_HPP

#include <utility>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/world/block/BlockPos.hpp"

class AABB;
class Block;
class BlockState;
class ConsoleSchematicFile;
class DataInputStream;
class DataOutputStream;
class Level;
class Player;

class FillTool : public BlockPos
{
public:
    enum EHandleDataType { HANDLE_DATA_NONE = 0, HANDLE_DATA_EDGE = 1, HANDLE_DATA_FILL = 2 };
    FillTool(ConsoleSchematicFile* schematic);
    ~FillTool();
    void Reset();
    bool IsWaterBlock(const BlockState* state);
    bool IsLavaBlock(const BlockState* state);
    bool IsValidSurfaceBlock(const BlockState* state);
    void tick();
    void PopulateSurfaceBrushBlock(boost::shared_ptr<Player> player);
    void Serialise(DataOutputStream* output);
    void Deserialise(DataInputStream* input);
    static Vec3i minVec(const Vec3i& first, const Vec3i& second);
    static Vec3i maxVec(const Vec3i& first, const Vec3i& second);
    AABB* GetAABB();
    void TakeSchematicForUndo(Level* level);
    void BeginFill(Level* level);
    void UndoFill(Level* level);
    void SetPointA(double x, double y, double z);
    void SetPointB(double x, double y, double z);
    void SetEdgeBlock(Block* block);
    void SetFillBlock(Block* block);
    void SetReplaceOnlyBlock(Block* block);
    void SetHandleDataType(EHandleDataType type);
    void SetEdgeData(int data);
    void SetFillData(int data);
    void SetFillDataMask(int mask);
    void SetReplaceOnlyData(int data, int mask);
    void SetDimension(int dimension);
    void SetXDiameter(int diameter);
    void SetYDiameter(int diameter);
    void SetZDiameter(int diameter);
    void SetDiameter(int diameter);
    int GetXDiameter();
    int GetYDiameter();
    int GetZDiameter();
    int GetDiameter();
    int GetDimension();
    BlockPos GetPointA() const;
    BlockPos GetPointB() const;
    Block* GetEdgeBlock() const;
    Block* GetFillBlock() const;
    int GetEdgeData() const;
    int GetFillData() const;
    int GetMode() const;

private:
    BlockPos mPointB;
    int mXDiameter;
    int mYDiameter;
    Block* mEdgeBlock;
    Block* mFillBlock;
    Block* mReplaceOnlyBlock;
    int mEdgeData;
    int mFillData;
    int mReplaceOnlyData;
    int mReplaceOnlyDataMask;
    int mFillDataMask;
    int mDimension;
    bool mReplaceOnly;
    bool mHasPointB;
    int mZDiameter;
    float mBrushStrength;
    bool mEnabled;
    std::vector<std::pair<const BlockState*, int> > mSurfaceBlocks;
    std::vector<std::pair<const BlockState*, int> > mBrushBlocks;
    bool mSurfaceBrush;
    unsigned char mReserved70[8];
    Level* mLevel;
    unsigned char mReserved78[40];
    ConsoleSchematicFile* mSchematic;
    AABB* mBounds;
    int mMode;
    unsigned char mReservedAC[12];
    Block* mBrushBlock;
    int mBrushData;
    bool mUseBrush;
    bool mUseSurface;
    Block* mFallbackBlock;
    int mX0;
    int mY0;
    int mZ0;
    int mX1;
    int mY1;
    int mZ1;
    int mFlags;
    int mMaxBlocks;
    int mMinY;
    unsigned char mReservedF8[4];
    int mProgress;
    unsigned char mReserved100[4];
    bool mFinished;
    unsigned char mReserved105[7];
    int mUndo;
    int mRedo;
    unsigned char mReserved114[4];
    bool mActive;
    unsigned char mReserved119[7];
};

#endif
