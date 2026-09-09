#include "game/core/OceanRuin.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/BoundingBox.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/RandomizableLockableContainerBlockEntity.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/StructurePiece.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"
#include "game/world/generation/StructureTemplate.hpp"

OceanRuin::OceanRuin()
    : ScatteredFeaturePieces::ScatteredFeaturePiece(), mBlockState(Blocks::AIR->defaultBlockState()),
      mIsLarge(false), mIsCold(false)
{
}

OceanRuin::OceanRuin(Level* level, Random& random, int x, int z, int width, int height, int depth,
    bool isLarge, bool isCold)
    : ScatteredFeaturePieces::ScatteredFeaturePiece(level, random, x, 0, z, width, height, depth),
      mBlockState(Blocks::AIR->defaultBlockState()), mIsLarge(isLarge), mIsCold(isCold)
{
    setOrientation(NULL);
    BoundingBox box(x, 0, z, x + width - 1, height - 1, z + depth - 1);
    setBoundingBox(&box);
    setRotation(Rotation::getRandom(random));
}

OceanRuin::~OceanRuin()
{
}

unsigned int OceanRuin::getBigWarmRuin(Random& random)
{
    static const unsigned int ruins[4] = { 0x92, 0x93, 0x94, 0x95 };
    return ruins[random.nextInt(4)];
}

unsigned int OceanRuin::getSmallWarmRuin(Random& random)
{
    static const unsigned int ruins[3] = { 0x96, 0x97, 0x98 };
    return ruins[random.nextInt(3)];
}

unsigned int OceanRuin::getRuin(int index, bool large, ERuinType type)
{
    static const unsigned int cold[] = { 0x86, 0x87, 0x88, 0x89, 0x99, 0x9c, 0x9f, 0xa2, 0xa5, 0xa8, 0xab, 0xae };
    static const unsigned int warm[] = { 0x8e, 0x8f, 0x90, 0x91, 0x9a, 0x9d, 0xa0, 0xa3, 0xa6, 0xa9, 0xac, 0xaf };
    static const unsigned int brick[] = { 0x8a, 0x8b, 0x8c, 0x8d, 0x9b, 0x9e, 0xa1, 0xa4, 0xa7, 0xaa, 0xad, 0xb0 };
    const unsigned int* ruins = type == eRuinType_Cold ? cold : type == eRuinType_Warm ? warm : brick;
    return ruins[large ? index : index + 4];
}

void OceanRuin::updateChestReplacer(eMinecraftStructureTemplate structureTemplate)
{
    if (mIsCold)
    {
        mBlockState = Blocks::sandstone->defaultBlockState();
    }
    else if (structureTemplate == static_cast<eMinecraftStructureTemplate>(0x9f))
    {
        mBlockState = Blocks::stonebrick->defaultBlockState();
    }
    else
    {
        mBlockState = Blocks::AIR->defaultBlockState();
        if (structureTemplate == static_cast<eMinecraftStructureTemplate>(0x86)
            || structureTemplate == static_cast<eMinecraftStructureTemplate>(0x89)
            || structureTemplate == static_cast<eMinecraftStructureTemplate>(0x87))
            mBlockState = Blocks::stone->defaultBlockState();
    }
}

const StructureTemplate* OceanRuin::placeRuin(Random& random, StructureManager* structureManager, Level* level,
    const BlockPos& pos, StructurePlaceSettings& settings)
{
    float integrity = mIsLarge ? 0.8f : 0.9f;
    if (!mIsCold)
    {
        int index = random.nextInt(mIsLarge ? 4 : 8);
        unsigned int large = getRuin(index, mIsLarge, eRuinType_Cold);
        unsigned int small = getRuin(index, mIsLarge, eRuinType_Warm);
        unsigned int brick = getRuin(index, mIsLarge, eRuinType_ColdBrick);
        updateChestReplacer(static_cast<eMinecraftStructureTemplate>(large));
        const StructureTemplate* result = structureManager->getOrCreate(static_cast<eMinecraftStructureTemplate>(large));
        settings.setIntegrity(integrity);
        result->placeInWorld(level, pos, &settings);
        settings.setIntegrity(0.5f);
        const StructureTemplate* middle = structureManager->getOrCreate(static_cast<eMinecraftStructureTemplate>(small));
        middle->placeInWorld(level, pos, &settings);
        settings.setIntegrity(0.7f);
        const StructureTemplate* last = structureManager->getOrCreate(static_cast<eMinecraftStructureTemplate>(brick));
        last->placeInWorld(level, pos, &settings);
        return result;
    }
    unsigned int ruin = mIsLarge ? getBigWarmRuin(random) : getSmallWarmRuin(random);
    updateChestReplacer(static_cast<eMinecraftStructureTemplate>(ruin));
    const StructureTemplate* result = structureManager->getOrCreate(static_cast<eMinecraftStructureTemplate>(ruin));
    settings.setIntegrity(integrity);
    result->placeInWorld(level, pos, &settings);
    return result;
}

void OceanRuin::adaptPosition(const BlockPos& pos, Mirror* mirror, Rotation* rotation, Level* level)
{
    BlockPos size = StructureTemplate::transform(BlockPos(mWidth - 1, 0, mDepth - 1), mirror, rotation);
    BlockPos min = pos.offset(size.x < pos.x ? size.x : pos.x, size.y < pos.y ? size.y : pos.y, size.z < pos.z ? size.z : pos.z);
    BlockPos max(size.x < pos.x ? pos.x : size.x, size.y < pos.y ? pos.y : size.y, size.z < pos.z ? pos.z : size.z);
    int lowest = 0x200;
    int count = 0;
    for (int x = min.x; x <= max.x; ++x)
        for (int z = min.z; z <= max.z; ++z)
            for (int y = max.y; y >= min.y && y > 1; --y)
            {
                BlockPos p(x, y, z);
                if (level->getBlockState(p) != NULL && level->getBlockState(p)->getBlock() != Blocks::AIR)
                    break;
                if (y < lowest) lowest = y;
                if (y < pos.y - 3) ++count;
            }
    if (pos.y - lowest > 2 && count > mWidth + 4)
        mBoundingBox->move(0, lowest + 3 - mBoundingBox->y0, 0);
}

void OceanRuin::fillBelow(const BlockPos& pos, Mirror* mirror, Rotation* rotation, Level* level)
{
    BlockPos size = StructureTemplate::transform(BlockPos(mWidth - 1, 0, mDepth - 1), mirror, rotation);
    BlockPos min = pos.offset(size.x < pos.x ? size.x : pos.x, size.y < pos.y ? size.y : pos.y, size.z < pos.z ? size.z : pos.z);
    BlockPos max(size.x < pos.x ? pos.x : size.x, size.y < pos.y ? pos.y : size.y, size.z < pos.z ? pos.z : size.z);
    for (int x = min.x; x <= max.x; ++x)
        for (int z = min.z; z <= max.z; ++z)
            for (int y = max.y; y > 1; --y)
            {
                BlockPos p(x, y, z);
                if (level->getBlockState(p) != NULL && level->getBlockState(p)->getBlock() != Blocks::AIR)
                    break;
                for (int fill = y - 1; fill >= 0; --fill)
                    if (level->getRandomFor(x, fill, z)->rollPercentageChance(0x52))
                        level->setBlock(BlockPos(x, fill, z), Blocks::stone->defaultBlockState(), 2, false);
            }
}

bool OceanRuin::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (!updateAverageGroundHeight(level, box, -2, false) || mBoundingBox == NULL)
        return true;
    BlockPos pos(mBoundingBox->x0, mBoundingBox->y0, mBoundingBox->z0);
    if (!box->isInside(pos))
        return true;
    Rotation* rotation = getRotation();
    Mirror* mirror = Mirror::NONE;
    adaptPosition(pos, mirror, rotation, level);
    mBoundingBox->move(0, mGroundHeight - mBoundingBox->y0, 0);
    StructurePlaceSettings settings;
    settings.setMirror(mirror);
    settings.setRotation(rotation);
    settings.setIgnoreBlock(Blocks::AIR);
    settings.setBoundingBox(mBoundingBox);
    fillBelow(pos, mirror, rotation, level);
    const StructureTemplate* structure = placeRuin(random, level->getLevelStorage()->getStructureManager(), level, pos, settings);
    StructureTemplate::MarkerMap markers = structure->getMarkers(pos, &settings);
    StructureTemplate::MarkerMap::iterator it;
    for (it = markers.begin(); it != markers.end(); ++it)
    {
        if (it->second == L"chest")
        {
            level->setBlock(it->first, mBlockState, 3, false);
            BlockPos below = it->first.below();
            level->setBlock(below, Blocks::AIR->defaultBlockState(), 3, false);
            boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(below);
            if (entity && entity->GetType() == 0x1e00201)
            {
                boost::shared_ptr<RandomizableLockableContainerBlockEntity> chest =
                    boost::static_pointer_cast<RandomizableLockableContainerBlockEntity>(entity);
                chest->setLootTable(mIsLarge ? &BuiltInLootTables::UNDERWATER_RUIN_BIG : &BuiltInLootTables::UNDERWATER_RUIN_SMALL,
                    random.nextLong());
            }
        }
    }
    return true;
}

void OceanRuin::addAdditionalSaveData(DataOutputStream* output)
{
    output->writeInt(Block::getId(mBlockState->getBlock()));
    output->writeBoolean(mIsLarge);
    output->writeBoolean(mIsCold);
}

void OceanRuin::readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager)
{
    mBlockState = Block::byId(input->readInt())->defaultBlockState();
    mIsLarge = input->readBoolean();
    mIsCold = input->readBoolean();
}

EStructurePiece OceanRuin::GetType()
{
    return eStructurePiece_OceanRuin;
}

StructurePiece* OceanRuin::Create()
{
    return new OceanRuin();
}
