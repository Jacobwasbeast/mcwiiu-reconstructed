#include "game/world/generation/ScatteredFeaturePieces.hpp"

#include "game/core/BoundingBox.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"
#include "game/world/generation/StructureManager.hpp"

ScatteredFeaturePieces::JunglePyramidPiece::MossStoneSelector* ScatteredFeaturePieces::JunglePyramidPiece::mStoneSelector = NULL;

void ScatteredFeaturePieces::loadStatic()
{
    StructureFeatureIO::setPieceId(eStructurePiece_DesertPyramidPiece, &ScatteredFeaturePieces::DesertPyramidPiece::Create, L"TeDP");
    StructureFeatureIO::setPieceId(eStructurePiece_JunglePyramidPiece, &ScatteredFeaturePieces::JunglePyramidPiece::Create, L"TeJP");
    StructureFeatureIO::setPieceId(eStructurePiece_SwamplandHut, &ScatteredFeaturePieces::SwamplandHut::Create, L"TeSH");
    StructureFeatureIO::setPieceId(eStructurePiece_Igloo, &ScatteredFeaturePieces::Igloo::Create, L"Iglu");
}

ScatteredFeaturePieces::ScatteredFeaturePiece::ScatteredFeaturePiece()
    : StructurePiece(), mPlaceSettings(), mWidth(0), mHeight(0), mDepth(0), mGroundHeight(0), mFirstBools(), mSecondBools(), mFirstInts(), mSecondInts()
{
}

ScatteredFeaturePieces::ScatteredFeaturePiece::ScatteredFeaturePiece(Level* level, Random& random, int x, int y, int z, int width, int height, int depth)
    : StructurePiece(), mPlaceSettings(), mWidth(width), mHeight(height), mDepth(depth), mGroundHeight(-1), mFirstBools(), mSecondBools(), mFirstInts(), mSecondInts()
{
    setOrientation(Direction::getRandomDirection(random));
    mBoundingBox = BoundingBox::orientBox(x, y, z, 0, 0, 0, width, height, depth, getOrientation());
}

ScatteredFeaturePieces::ScatteredFeaturePiece::~ScatteredFeaturePiece()
{
}

void ScatteredFeaturePieces::ScatteredFeaturePiece::addAdditionalSaveData(DataOutputStream* output)
{
    output->writeInt(mWidth);
    output->writeInt(mHeight);
    output->writeInt(mDepth);
    output->writeInt(mGroundHeight);
    mPlaceSettings.write(output);
}

void ScatteredFeaturePieces::ScatteredFeaturePiece::readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager)
{
    mWidth = input->readInt();
    mHeight = input->readInt();
    mDepth = input->readInt();
    mGroundHeight = input->readInt();
    if (StructureFeatureIO::getVersion() == 1)
    {
        mFirstBools = readBoolSaveData(input);
        mSecondBools = readBoolSaveData(input);
        mFirstInts = readIntSaveData(input);
        mSecondInts = readIntSaveData(input);
    }
    else if (StructureFeatureIO::getVersion() > 2)
    {
        mPlaceSettings.read(input);
    }
}

bool ScatteredFeaturePieces::ScatteredFeaturePiece::updateAverageGroundHeight(Level* level, BoundingBox* box, int offset, bool useHeightmap)
{
    if (mGroundHeight > -1)
        return true;
    int total = 0;
    int count = 0;
    for (int z = box->z0; z <= box->z1; ++z)
    {
        for (int x = box->x0; x <= box->x1; ++x)
        {
            if (box->isInside(BlockPos(x, 64, z)))
            {
                int height = level->getTopSolidBlockPos(BlockPos(x, 0, z)).y;
                if (!useHeightmap && height < level->getSeaLevel())
                    height = level->getSeaLevel();
                total += height;
                ++count;
            }
        }
    }
    if (count == 0)
        return false;
    mGroundHeight = total / count;
    mBoundingBox->move(0, mGroundHeight - mBoundingBox->y0 + offset, 0);
    return true;
}

ScatteredFeaturePieces::DesertPyramidPiece::DesertPyramidPiece()
    : ScatteredFeaturePiece()
{
    for (int i = 0; i < 4; ++i) mTrap[i] = false;
}

ScatteredFeaturePieces::DesertPyramidPiece::DesertPyramidPiece(Level* level, Random& random, int x, int z)
    : ScatteredFeaturePiece(level, random, x, 64, z, 21, 15, 21)
{
    for (int i = 0; i < 4; ++i) mTrap[i] = false;
}

ScatteredFeaturePieces::DesertPyramidPiece::~DesertPyramidPiece()
{
}

void ScatteredFeaturePieces::DesertPyramidPiece::addAdditionalSaveData(DataOutputStream* output)
{
    ScatteredFeaturePiece::addAdditionalSaveData(output);
    output->writeBoolean(mTrap[0]); output->writeBoolean(mTrap[1]); output->writeBoolean(mTrap[2]); output->writeBoolean(mTrap[3]);
}

void ScatteredFeaturePieces::DesertPyramidPiece::readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager)
{
    ScatteredFeaturePiece::readAdditionalSaveData(input, structureManager);
    if (StructureFeatureIO::getVersion() == 1)
    {
        if (mSecondBools.size() > 3)
        {
            mTrap[0] = mSecondBools[0];
            mTrap[1] = mSecondBools[1];
            mTrap[2] = mSecondBools[2];
            mTrap[3] = mSecondBools[3];
        }
    }
    else
    {
        mTrap[0] = input->readBoolean();
        mTrap[1] = input->readBoolean();
        mTrap[2] = input->readBoolean();
        mTrap[3] = input->readBoolean();
    }
}

bool ScatteredFeaturePieces::DesertPyramidPiece::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (!updateAverageGroundHeight(level, box, 0, false)) return false;
    const BlockState* stone = Blocks::sandstone->defaultBlockState();
    const BlockState* air = Blocks::AIR->defaultBlockState();
    for (int layer = 0; layer < 5; ++layer)
    {
        generateBox(level, box, layer, layer, layer, 20 - layer, layer, 20 - layer, stone, stone, false);
        generateBox(level, box, layer + 1, layer + 1, layer + 1, 19 - layer, layer + 1, 19 - layer, air, air, false);
    }
    generateBox(level, box, 4, 5, 4, 16, 10, 16, air, air, false);
    generateBox(level, box, 8, 0, 8, 12, 0, 12, stone, stone, false);
    if (!mTrap[0]) { mTrap[0] = createChest(level, box, random, 10, 1, 10, &BuiltInLootTables::DESERT_PYRAMID); if (mTrap[0]) setDirty(); }
    return true;
}

StructurePiece* ScatteredFeaturePieces::DesertPyramidPiece::Create() { return new DesertPyramidPiece(); }
EStructurePiece ScatteredFeaturePieces::DesertPyramidPiece::GetType() { return eStructurePiece_DesertPyramidPiece; }

ScatteredFeaturePieces::Igloo::Igloo() : ScatteredFeaturePiece(), mHasBasement(false), mHeightOffset(0) {}

ScatteredFeaturePieces::Igloo::Igloo(Level* level, Random& random, int x, int z)
    : ScatteredFeaturePiece(level, random, x, 64, z, 7, 5, 8), mHasBasement(random.nextDouble() < 0.5), mHeightOffset(random.nextInt(8) + 4)
{
    mPlaceSettings.setIgnoreBlock(Blocks::AIR);
}

ScatteredFeaturePieces::Igloo::~Igloo() {}

bool ScatteredFeaturePieces::Igloo::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (!updateAverageGroundHeight(level, box, 0, false)) return false;
    const BlockState* snow = Blocks::snow->defaultBlockState();
    const BlockState* air = Blocks::AIR->defaultBlockState();
    for (int y = 0; y < 4; ++y)
    {
        int radius = y < 2 ? 3 : 2;
        generateBox(level, box, 3 - radius, y, 3 - radius, 3 + radius, y, 3 + radius, snow, snow, false);
        generateBox(level, box, 3 - radius + 1, y + 1, 3 - radius + 1, 3 + radius - 1, y + 1, 3 + radius - 1, air, air, false);
    }
    generateBox(level, box, 1, 0, 1, 5, 0, 6, snow, snow, false);
    if (mHasBasement)
        generateAirBox(level, box, 2, -mHeightOffset, 2, 4, -1, 5);
    return true;
}

void ScatteredFeaturePieces::Igloo::addAdditionalSaveData(DataOutputStream* output)
{
    ScatteredFeaturePiece::addAdditionalSaveData(output); output->writeBoolean(mHasBasement); output->writeInt(mHeightOffset);
}

void ScatteredFeaturePieces::Igloo::readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager)
{
    ScatteredFeaturePiece::readAdditionalSaveData(input, structureManager);
    if (StructureFeatureIO::getVersion() > 2) { mHasBasement = input->readBoolean(); mHeightOffset = input->readInt(); }
}

StructurePiece* ScatteredFeaturePieces::Igloo::Create() { return new Igloo(); }
EStructurePiece ScatteredFeaturePieces::Igloo::GetType() { return eStructurePiece_Igloo; }

ScatteredFeaturePieces::JunglePyramidPiece::MossStoneSelector::~MossStoneSelector() {}
void ScatteredFeaturePieces::JunglePyramidPiece::MossStoneSelector::next(Random& random, int x, int y, int z, bool edge)
{
    mNext = random.nextFloat() < 0.4f ? Blocks::mossy_cobblestone->defaultBlockState() : Blocks::cobblestone->defaultBlockState();
}

ScatteredFeaturePieces::JunglePyramidPiece::JunglePyramidPiece()
    : ScatteredFeaturePiece()
{
    for (int i = 0; i < 4; ++i) mTrap[i] = false;
}

ScatteredFeaturePieces::JunglePyramidPiece::JunglePyramidPiece(Level* level, Random& random, int x, int z)
    : ScatteredFeaturePiece(level, random, x, 64, z, 12, 10, 15)
{
    for (int i = 0; i < 4; ++i) mTrap[i] = false;
}

ScatteredFeaturePieces::JunglePyramidPiece::~JunglePyramidPiece() {}

void ScatteredFeaturePieces::JunglePyramidPiece::addAdditionalSaveData(DataOutputStream* output)
{
    ScatteredFeaturePiece::addAdditionalSaveData(output);
    for (int i = 0; i < 4; ++i) output->writeBoolean(mTrap[i]);
}

void ScatteredFeaturePieces::JunglePyramidPiece::readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager)
{
    ScatteredFeaturePiece::readAdditionalSaveData(input, structureManager);
    for (int i = 0; i < 4; ++i) mTrap[i] = input->readBoolean();
}

bool ScatteredFeaturePieces::JunglePyramidPiece::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (!updateAverageGroundHeight(level, box, 0, true)) return false;
    const BlockState* stone = Blocks::cobblestone->defaultBlockState();
    const BlockState* moss = Blocks::mossy_cobblestone->defaultBlockState();
    const BlockState* air = Blocks::AIR->defaultBlockState();
    generateBox(level, box, 0, -4, 0, 11, -4, 14, stone, stone, false);
    generateBox(level, box, 1, -3, 1, 10, 6, 13, stone, stone, false);
    generateAirBox(level, box, 2, -2, 2, 9, 5, 12);
    generateBox(level, box, 2, 6, 2, 9, 6, 12, moss, moss, false);
    generateBox(level, box, 3, 7, 3, 8, 7, 11, stone, stone, false);
    generateAirBox(level, box, 4, 7, 4, 7, 7, 10);
    if (!mTrap[0]) { mTrap[0] = createDispenser(level, box, random, 3, 0, 12, Direction::SOUTH, &BuiltInLootTables::JUNGLE_TEMPLE_DISPENSER); if (mTrap[0]) setDirty(); }
    if (!mTrap[1]) { mTrap[1] = createChest(level, box, random, 8, -1, 3, &BuiltInLootTables::JUNGLE_TEMPLE); if (mTrap[1]) setDirty(); }
    return true;
}

void ScatteredFeaturePieces::JunglePyramidPiece::staticCtor()
{
    mStoneSelector = new MossStoneSelector();
}
StructurePiece* ScatteredFeaturePieces::JunglePyramidPiece::Create() { return new JunglePyramidPiece(); }
EStructurePiece ScatteredFeaturePieces::JunglePyramidPiece::GetType() { return eStructurePiece_JunglePyramidPiece; }

ScatteredFeaturePieces::SwamplandHut::SwamplandHut() : ScatteredFeaturePiece(), mHasWitch(false) {}
ScatteredFeaturePieces::SwamplandHut::SwamplandHut(Level* level, Random& random, int x, int z)
    : ScatteredFeaturePiece(level, random, x, 64, z, 7, 7, 9), mHasWitch(false) {}
ScatteredFeaturePieces::SwamplandHut::~SwamplandHut() {}
void ScatteredFeaturePieces::SwamplandHut::addAdditionalSaveData(DataOutputStream* output)
{ ScatteredFeaturePiece::addAdditionalSaveData(output); output->writeBoolean(mHasWitch); }
void ScatteredFeaturePieces::SwamplandHut::readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager)
{ ScatteredFeaturePiece::readAdditionalSaveData(input, structureManager); mHasWitch = input->readBoolean(); }
bool ScatteredFeaturePieces::SwamplandHut::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (!updateAverageGroundHeight(level, box, 0, true)) return false;
    const BlockState* planks = Blocks::planks->defaultBlockState();
    const BlockState* fence = Blocks::jungle_fence->defaultBlockState();
    const BlockState* air = Blocks::AIR->defaultBlockState();
    generateBox(level, box, 1, 0, 1, 5, 4, 7, planks, planks, false);
    generateAirBox(level, box, 2, 1, 2, 4, 3, 6);
    generateBox(level, box, 0, 4, 0, 6, 4, 8, planks, planks, false);
    generateBox(level, box, 1, 5, 1, 5, 5, 7, air, air, false);
    for (int x = 1; x <= 5; x += 4) for (int z = 1; z <= 7; z += 6) generateBox(level, box, x, 0, z, x, 3, z, fence, fence, false);
    return true;
}
StructurePiece* ScatteredFeaturePieces::SwamplandHut::Create() { return new SwamplandHut(); }
EStructurePiece ScatteredFeaturePieces::SwamplandHut::GetType() { return eStructurePiece_SwamplandHut; }
