#include "game/world/generation/NetherBridgeFeature.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/core/NetherBridgePieces.hpp"

NetherBridgeFeature::NetherBridgeFeature()
    : StructureFeature(), mCachedBuildings(false), mCachedChunk(NULL)
{
    mBridgeEnemies.push_back(new Biome::MobSpawnerData(10, EntityType::BLAZE, 2, 3));
    mBridgeEnemies.push_back(new Biome::MobSpawnerData(5, EntityType::PIG_ZOMBIE, 4, 4));
    mBridgeEnemies.push_back(new Biome::MobSpawnerData(8, EntityType::WITHER_SKELETON, 5, 5));
    mBridgeEnemies.push_back(new Biome::MobSpawnerData(2, EntityType::SKELETON, 5, 5));
    mBridgeEnemies.push_back(new Biome::MobSpawnerData(3, EntityType::MAGMA_CUBE, 4, 4));
}

NetherBridgeFeature::~NetherBridgeFeature()
{
    for (std::vector<Biome::MobSpawnerData*>::iterator it = mBridgeEnemies.begin();
         it != mBridgeEnemies.end(); ++it)
    {
        delete *it;
    }
    delete mCachedChunk;
}

std::wstring NetherBridgeFeature::getFeatureName()
{
    return std::wstring(L"Fortress");
}

std::vector<Biome::MobSpawnerData*>* NetherBridgeFeature::getBridgeEnemies()
{
    return &mBridgeEnemies;
}

bool NetherBridgeFeature::isFeatureChunk(int chunkX, int chunkZ, bool flatWorld)
{
    if (!mCachedBuildings)
    {
        long long seed = level->getSeed();
        random.setSeed(seed);
        random.nextInt();
        int index = random.nextInt(49);
        mCachedChunk = new ChunkPos(index % 7, index / 7);
        mCachedBuildings = true;
    }

    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if ((options == NULL || !options->isFeatureChunk(chunkX, chunkZ,
                                                      eFeatureType_NetherBridge, NULL)) &&
        (chunkX != mCachedChunk->x || chunkZ != mCachedChunk->z))
    {
        return false;
    }
    return true;
}

StructureStart* NetherBridgeFeature::createStructureStart(int chunkX, int chunkZ)
{
    return new NetherBridgeStart(level, random, chunkX, chunkZ);
}

BlockPos* NetherBridgeFeature::getNearestGeneratedFeature(Level* levelIn, const BlockPos& pos,
                                                            bool skipExistingChunks)
{
    int chunkX = pos.x >> 4;
    int chunkZ = pos.z >> 4;
    int remaining = 1001;

    for (int radius = 0; ; ++radius)
    {
        for (int offsetX = -radius; offsetX <= radius; ++offsetX)
        {
            bool edgeX = offsetX == -radius || offsetX == radius;
            for (int offsetZ = -radius; offsetZ <= radius; ++offsetZ)
            {
                bool edgeZ = offsetZ == -radius || offsetZ == radius;
                if (!edgeX && !edgeZ)
                    continue;

                int candidateX = chunkX + offsetX;
                int candidateZ = chunkZ + offsetZ;
                if (isFeatureChunk(candidateX, candidateZ, false) &&
                    (!skipExistingChunks || !levelIn->isChunkGeneratedAt(candidateX, candidateZ)))
                    return new BlockPos(candidateX * 16 + 8, 64, candidateZ * 16 + 8);
            }
        }

        --remaining;
        if (remaining == 0)
            return NULL;
    }
}

NetherBridgeFeature::NetherBridgeStart::NetherBridgeStart(Level* levelIn, Random& random,
                                                           int chunkX, int chunkZ)
    : StructureStart(chunkX, chunkZ)
{
    NetherBridgePieces::StartPiece* startPiece = new NetherBridgePieces::StartPiece(
        random, chunkX * 16 + 2, chunkZ * 16 + 2, levelIn);
    push_back(startPiece);
    startPiece->addChildren(startPiece, this, random);

    while (!startPiece->mPendingChildren.empty())
    {
        std::vector<StructurePiece*>::iterator it = startPiece->mPendingChildren.begin();
        std::advance(it, random.nextInt((int)startPiece->mPendingChildren.size()));
        StructurePiece* piece = *it;
        startPiece->mPendingChildren.erase(it);
        piece->addChildren(startPiece, this, random);
    }

    calculateBoundingBox();
    moveInsideHeights(levelIn, random, 0x30, 0x46);
}

NetherBridgeFeature::NetherBridgeStart::NetherBridgeStart()
    : StructureStart()
{
}

NetherBridgeFeature::NetherBridgeStart::~NetherBridgeStart()
{
}

EStructureStart NetherBridgeFeature::NetherBridgeStart::GetType()
{
    return eStructureStart_NetherBridge;
}

StructureStart* NetherBridgeFeature::NetherBridgeStart::Create()
{
    return new NetherBridgeStart();
}
