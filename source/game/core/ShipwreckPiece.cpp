#include "game/core/ShipwreckPiece.hpp"

#include <utility>

#include <boost/shared_ptr.hpp>

#include "game/core/BoundingBox.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/util/Random.hpp"
#include "game/world/GenericOverworldLevelSource.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/RandomizableLockableContainerBlockEntity.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/StructureTemplate.hpp"

static const int SHIPWRECK_TYPES[20] = {
    0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b,
    0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85
};

static const int SHIPWRECK_AUX_VALUES[9][2] = {
    { 0, 1 }, { 3, 1 }, { 5, 3 }, { 5, 1 }, { 1, 0 },
    { 1, 3 }, { 1, 5 }, { 0, 2 }, { 1, 4 }
};

int ShipwreckPiece::GetNonUpsideDownShipwreckType(Random& random)
{
    static const int types[14] = { 0x72, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b,
        0x7c, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85 };
    static const int groups[2][10] = {
        { 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b },
        { 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85 }
    };
    int selected = types[random.nextInt(14)];
    int offset = 0;
    int group;
    for (group = 0; group < 2; ++group)
    {
        int index;
        for (index = 0; index < 10; ++index)
        {
            if (groups[group][index] == selected)
            {
                return offset + index;
            }
        }
        offset += 10;
    }
    return 0x72;
}

ShipwreckPiece::ShipwreckPiece(Level* level, GenericOverworldLevelSource* levelSource,
                               int x, int z, Random& random, const Direction* orientation)
    : StructurePiece(20), mRotation(NULL), mType(0), mNonUpsideDownType(0), mWoodType(0), mHeight(0)
{
    mWoodType = random.nextInt(9);
    setOrientation(orientation);
    mType = random.nextInt(20);
    mNonUpsideDownType = GetNonUpsideDownShipwreckType(random);

    StructureManager* structureManager = level->getLevelStorage()->getStructureManager();
    const StructureTemplate* normal = structureManager->getOrCreate(
        static_cast<eMinecraftStructureTemplate>(SHIPWRECK_TYPES[mType]));
    const StructureTemplate* alternate = structureManager->getOrCreate(
        static_cast<eMinecraftStructureTemplate>(SHIPWRECK_TYPES[mNonUpsideDownType]));
    BlockPos normalSize = normal->getSize();
    BlockPos alternateSize = alternate->getSize();
    int sizeX = normalSize.x < alternateSize.x ? alternateSize.x : normalSize.x;
    int sizeY = normalSize.y < alternateSize.y ? alternateSize.y : normalSize.y;
    int sizeZ = normalSize.z < alternateSize.z ? alternateSize.z : normalSize.z;
    BlockPos size(sizeX, sizeY, sizeZ);
    int maxSize = size.x;
    if (maxSize < size.z) maxSize = size.z;

    mRotation = Rotation::VALUES[random.nextInt(4)];
    setBoundingBox(new BoundingBox(x, 0, z, x + maxSize - 1, size.y - 1, z + maxSize - 1));

    BlockPos origin(getWorldX(0, 0), getWorldY(1), getWorldZ(0, 0));
    arrayWithLength<unsigned short> elements(0x10000, true);
    ChunkPrimer primer(true, elements);
    ChunkPos chunkPos(origin);
    levelSource->prepareHeights(chunkPos.x, chunkPos.z, &primer);
    int heights[4] = { primer.getHighestSolidPos(7, 7), primer.getHighestSolidPos(7, 12),
        primer.getHighestSolidPos(12, 7), primer.getHighestSolidPos(12, 12) };
    mHeight = heights[0];
    int i;
    for (i = 1; i < 4; ++i)
    {
        if (heights[i] < mHeight) mHeight = heights[i];
    }
}

ShipwreckPiece::~ShipwreckPiece()
{
}

bool ShipwreckPiece::IsUpsideDownShipwreckType(int type)
{
    int value = 0x72 + type;
    return value == 0x73 || value == 0x74 || value == 0x75 || value == 0x7d
        || value == 0x7e || value == 0x7f;
}

bool ShipwreckPiece::postProcess(Level* level, Random& random, BoundingBox* box)
{
    BlockPos origin(getWorldX(0, 0), getWorldY(1), getWorldZ(0, 0));
    Block* block = level->getBlock(origin);
    bool inWater = block == Blocks::WATER || block == Blocks::FLOWING_WATER;
    StructureManager* structureManager = level->getLevelStorage()->getStructureManager();
    int templateType = inWater ? mNonUpsideDownType : mType;
    bool upsideDown = inWater ? false : IsUpsideDownShipwreckType(mType);

    StructurePlaceSettings settings;
    settings.setBoundingBox(box);
    settings.setIntegrity(1.0f);
    settings.addSwapAuxValue(0, SHIPWRECK_AUX_VALUES[mWoodType][0]);
    settings.addSwapAuxValue(5, SHIPWRECK_AUX_VALUES[mWoodType][1]);
    settings.setRotation(mRotation);
    settings.setMirror(Mirror::NONE);
    settings.placeWaterBelowSeaLevel(true);

    int yOffset = upsideDown || inWater ? -4 : -2;
    BlockPos place = origin;
    if (mRotation->getType() == Rotation::eTYPE_Clockwise90)
    {
        place = origin.offset(getBoundingBox()->getXSpan(), mHeight + yOffset, 0);
    }
    else if (mRotation->getType() == Rotation::eTYPE_Clockwise180)
    {
        place = origin.offset(getBoundingBox()->getXSpan(), mHeight + yOffset,
                              getBoundingBox()->getZSpan());
    }
    else
    {
        place = origin.offset(0, mHeight + yOffset, 0);
    }

    const StructureTemplate* structure = structureManager->getOrCreate(
        static_cast<eMinecraftStructureTemplate>(SHIPWRECK_TYPES[templateType]));
    structure->placeInWorldChunk(level, place, &settings);
    StructureTemplate::MarkerMap markers = structure->getMarkers(place, &settings);
    StructureTemplate::MarkerMap::const_iterator it;
    for (it = markers.begin(); it != markers.end(); ++it)
    {
        if (it->second == L"mapChest")
        {
            boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(it->first.below());
            if (blockEntity && blockEntity->GetType() == 0x1e00201ULL)
            {
                boost::shared_ptr<RandomizableLockableContainerBlockEntity> chest =
                    boost::static_pointer_cast<RandomizableLockableContainerBlockEntity>(blockEntity);
                chest->setLootTable(&BuiltInLootTables::SHIPWRECK, random.nextLong());
            }
        }
        else if (it->second == L"supplyChest")
        {
            boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(it->first.below());
            if (blockEntity && blockEntity->GetType() == 0x1e00201ULL)
            {
                boost::shared_ptr<RandomizableLockableContainerBlockEntity> chest =
                    boost::static_pointer_cast<RandomizableLockableContainerBlockEntity>(blockEntity);
                chest->setLootTable(&BuiltInLootTables::SHIPWRECKSUPPLY, random.nextLong());
            }
        }
        else if (it->second == L"treasureChest")
        {
            boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(it->first.below());
            if (blockEntity && blockEntity->GetType() == 0x1e00201ULL)
            {
                boost::shared_ptr<RandomizableLockableContainerBlockEntity> chest =
                    boost::static_pointer_cast<RandomizableLockableContainerBlockEntity>(blockEntity);
                chest->setLootTable(&BuiltInLootTables::SHIPWRECKTREASURE, random.nextLong());
            }
        }
    }
    return true;
}

void ShipwreckPiece::addAdditionalSaveData(DataOutputStream* output)
{
    output->writeInt(mType);
    output->writeInt(mNonUpsideDownType);
    output->writeInt(mWoodType);
    output->writeInt(mHeight);
}

void ShipwreckPiece::readAdditionalSaveData(DataInputStream* input, StructureManager*)
{
    mType = input->readInt();
    mNonUpsideDownType = input->readInt();
    mWoodType = input->readInt();
    mHeight = input->readInt();
}

EStructurePiece ShipwreckPiece::GetType()
{
    return eStructurePiece_ShipwreckPiece;
}
