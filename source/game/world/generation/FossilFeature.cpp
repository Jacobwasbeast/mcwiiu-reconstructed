#include "game/world/generation/FossilFeature.hpp"

#include "game/core/BoundingBox.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"
#include "game/world/generation/StructureTemplate.hpp"

FossilFeature::~FossilFeature()
{
}

bool FossilFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    Random placementRandom = level->getRandomFor(0, 0, 0x3ad8025f);
    Rotation* rotation = Rotation::getRandom(placementRandom);

    Minecraft* minecraft = Minecraft::GetInstance();
    StructureManager* structureManager = minecraft->getStructureManager();
    const StructureTemplate* skull = structureManager->get(
        static_cast<eMinecraftStructureTemplate>(placementRandom.nextInt(8) + 3));
    const StructureTemplate* spine = structureManager->get(
        static_cast<eMinecraftStructureTemplate>(placementRandom.nextInt(8) + 7));

    ChunkPos chunkPos(origin);
    BoundingBox boundingBox(chunkPos.getMinBlockX(), 0, chunkPos.getMinBlockZ(),
                            chunkPos.getMaxBlockX(), 256, chunkPos.getMaxBlockZ());
    StructurePlaceSettings settings;
    settings.setRotation(rotation);
    settings.setBoundingBox(&boundingBox);
    settings.setRandom(&placementRandom);

    const BlockPos size = skull->getSize(rotation);
    const int xOffset = placementRandom.nextInt(16 - size.x);
    const int zOffset = placementRandom.nextInt(16 - size.z);
    int height = 256;
    for (int x = 0; x < size.x; ++x)
    {
        for (int z = 0; z < size.x; ++z)
        {
            const int columnHeight = level->getHeightmapHeight(origin.x + xOffset + x,
                                                                origin.z + zOffset + z);
            if (columnHeight < height)
            {
                height = columnHeight;
            }
        }
    }

    int yOffset = height - 15 - placementRandom.nextInt(10);
    if (yOffset < 10)
    {
        yOffset = 10;
    }

    const BlockPos position = origin.offset(xOffset, yOffset, zOffset);
    const BlockPos transformed = skull->getZeroPositionWithTransform(position, Mirror::NONE, rotation);

    settings.setIntegrity(0.9f);
    skull->placeInWorld(level, transformed, &settings, 20);
    settings.setIntegrity(0.1f);
    spine->placeInWorld(level, transformed, &settings, 20);

    return true;
}
