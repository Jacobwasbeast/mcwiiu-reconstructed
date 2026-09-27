#include "game/core/ShipwreckPiece.hpp"
#include "game/core/EntityType.hpp"

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
#include "game/world/biome/Biome.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/StructureTemplate.hpp"

static const eMinecraftStructureTemplate SHIPWRECK_TYPES[20] =
{
    eMinecraftStructureTemplate_Shipwreck_WithMast,
    eMinecraftStructureTemplate_Shipwreck_UpsideDownFull,
    eMinecraftStructureTemplate_Shipwreck_UpsideDownFrontHalf,
    eMinecraftStructureTemplate_Shipwreck_UpsideDownBackHalf,
    eMinecraftStructureTemplate_Shipwreck_SidewaysFull,
    eMinecraftStructureTemplate_Shipwreck_SidewaysFrontHalf,
    eMinecraftStructureTemplate_Shipwreck_SidewaysBackHalf,
    eMinecraftStructureTemplate_Shipwreck_RightSideUpFull,
    eMinecraftStructureTemplate_Shipwreck_RightSideUpFrontHalf,
    eMinecraftStructureTemplate_Shipwreck_RightSideUpBackHalf,
    eMinecraftStructureTemplate_Shipwreck_WithMastDegraded,
    eMinecraftStructureTemplate_Shipwreck_UpsideDownFullDegraded,
    eMinecraftStructureTemplate_Shipwreck_UpsideDownFrontHalfDegraded,
    eMinecraftStructureTemplate_Shipwreck_UpsideDownBackHalfDegraded,
    eMinecraftStructureTemplate_Shipwreck_SidewaysFullDegraded,
    eMinecraftStructureTemplate_Shipwreck_SidewaysFrontHalfDegraded,
    eMinecraftStructureTemplate_Shipwreck_SidewaysBackHalfDegraded,
    eMinecraftStructureTemplate_Shipwreck_RightSideUpFullDegraded,
    eMinecraftStructureTemplate_Shipwreck_RightSideUpFrontHalfDegraded,
    eMinecraftStructureTemplate_Shipwreck_RightSideUpBackHalfDegraded
};

static const int SHIPWRECK_AUX_VALUES[9][2] = {
    { 0, 1 }, { 3, 1 }, { 5, 3 }, { 5, 1 }, { 1, 0 },
    { 1, 3 }, { 1, 5 }, { 0, 2 }, { 1, 4 }
};

int ShipwreckPiece::GetNonUpsideDownShipwreckType(Random& random)
{
    const eMinecraftStructureTemplate types[14] =
    {
        eMinecraftStructureTemplate_Shipwreck_WithMast,
        eMinecraftStructureTemplate_Shipwreck_SidewaysFull,
        eMinecraftStructureTemplate_Shipwreck_SidewaysFrontHalf,
        eMinecraftStructureTemplate_Shipwreck_SidewaysBackHalf,
        eMinecraftStructureTemplate_Shipwreck_RightSideUpFull,
        eMinecraftStructureTemplate_Shipwreck_RightSideUpFrontHalf,
        eMinecraftStructureTemplate_Shipwreck_RightSideUpBackHalf,
        eMinecraftStructureTemplate_Shipwreck_WithMastDegraded,
        eMinecraftStructureTemplate_Shipwreck_SidewaysFullDegraded,
        eMinecraftStructureTemplate_Shipwreck_SidewaysFrontHalfDegraded,
        eMinecraftStructureTemplate_Shipwreck_SidewaysBackHalfDegraded,
        eMinecraftStructureTemplate_Shipwreck_RightSideUpFullDegraded,
        eMinecraftStructureTemplate_Shipwreck_RightSideUpFrontHalfDegraded,
        eMinecraftStructureTemplate_Shipwreck_RightSideUpBackHalfDegraded
    };
    eMinecraftStructureTemplate selected = types[random.nextInt(14)];
    for (int i = 0; i < 20; ++i)
    {
        if (SHIPWRECK_TYPES[i] == selected)
            return i;
    }
    return eMinecraftStructureTemplate_Shipwreck_WithMast;
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
        SHIPWRECK_TYPES[mType]);
    const StructureTemplate* alternate = structureManager->getOrCreate(
        SHIPWRECK_TYPES[mNonUpsideDownType]);
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
    eMinecraftStructureTemplate value = SHIPWRECK_TYPES[type];
    return value == eMinecraftStructureTemplate_Shipwreck_UpsideDownFull
        || value == eMinecraftStructureTemplate_Shipwreck_UpsideDownFrontHalf
        || value == eMinecraftStructureTemplate_Shipwreck_UpsideDownBackHalf
        || value == eMinecraftStructureTemplate_Shipwreck_UpsideDownFullDegraded
        || value == eMinecraftStructureTemplate_Shipwreck_UpsideDownFrontHalfDegraded
        || value == eMinecraftStructureTemplate_Shipwreck_UpsideDownBackHalfDegraded;
}

bool ShipwreckPiece::postProcess(Level* level, Random& random, BoundingBox* box)
{
    BlockPos origin(getWorldX(0, 0), getWorldY(1), getWorldZ(0, 0));
    Biome* biome = level->getBiome(origin);
    bool beached = biome == Biome::BEACH || biome == Biome::COLD_BEACH;
    StructureManager* structureManager = level->getLevelStorage()->getStructureManager();
    int templateType;
    bool upsideDown;
    if (beached)
    {
        templateType = mNonUpsideDownType;
        upsideDown = false;
    }
    else
    {
        templateType = mType;
        upsideDown = IsUpsideDownShipwreckType(templateType);
    }

    StructurePlaceSettings settings;
    settings.setBoundingBox(box);
    settings.setIntegrity(1.0f);
    settings.addSwapAuxValue(0, SHIPWRECK_AUX_VALUES[mWoodType][0]);
    settings.addSwapAuxValue(5, SHIPWRECK_AUX_VALUES[mWoodType][1]);
    settings.setRotation(mRotation);
    settings.setMirror(Mirror::NONE);
    settings.placeWaterBelowSeaLevel(true);

    int yOffset = upsideDown || beached ? -4 : -2;
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
        SHIPWRECK_TYPES[templateType]);
    structure->placeInWorldChunk(level, place, &settings);
    StructureTemplate::MarkerMap markers = structure->getMarkers(place, &settings);
    StructureTemplate::MarkerMap::const_iterator it;
    for (it = markers.begin(); it != markers.end(); ++it)
    {
        if (it->second == L"mapChest")
        {
            boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(it->first.below());
            if (blockEntity && blockEntity->GetType() == BlockEntityType::CHEST)
            {
                boost::shared_ptr<ChestBlockEntity> chest =
                    boost::static_pointer_cast<ChestBlockEntity>(blockEntity);
                chest->setLootTable(&BuiltInLootTables::SHIPWRECK, random.nextLong());
            }
        }
        else if (it->second == L"supplyChest")
        {
            boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(it->first.below());
            if (blockEntity && blockEntity->GetType() == BlockEntityType::CHEST)
            {
                boost::shared_ptr<ChestBlockEntity> chest =
                    boost::static_pointer_cast<ChestBlockEntity>(blockEntity);
                chest->setLootTable(&BuiltInLootTables::SHIPWRECKSUPPLY, random.nextLong());
            }
        }
        else if (it->second == L"treasureChest")
        {
            boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(it->first.below());
            if (blockEntity && blockEntity->GetType() == BlockEntityType::CHEST)
            {
                boost::shared_ptr<ChestBlockEntity> chest =
                    boost::static_pointer_cast<ChestBlockEntity>(blockEntity);
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
