#include "game/world/generation/NetherBridgeFeature.hpp"

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
    mBridgeEnemies.push_back(new Biome::MobSpawnerData(10, 0x5D540045ULL, 2, 3));
    mBridgeEnemies.push_back(new Biome::MobSpawnerData(5, 0x5D550041ULL, 4, 4));
    mBridgeEnemies.push_back(new Biome::MobSpawnerData(8, 0x5D540442ULL, 5, 5));
    mBridgeEnemies.push_back(new Biome::MobSpawnerData(2, 0x5D540441ULL, 5, 5));
    mBridgeEnemies.push_back(new Biome::MobSpawnerData(3, 0x5D108001ULL, 4, 4));
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
        int offset = random.nextInt(0x31) % 7;
        mCachedChunk = new ChunkPos(0, offset);
        mCachedBuildings = true;
    }

    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if ((options == NULL || !options->isFeatureChunk(chunkX, chunkZ,
                                                      eFeatureType_NetherBridge, NULL)) &&
        (mCachedChunk == NULL || chunkX != mCachedChunk->x || chunkZ != mCachedChunk->z))
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
    level = levelIn;
    int centerX = pos.x >> 4;
    int centerZ = pos.z >> 4;
    int min = 0;
    int max = 0;

    for (int radius = 0; radius < 1001; ++radius)
    {
        for (int x = min; x <= max; ++x)
        {
            int chunkX = centerX + x;
            int chunkZ = centerZ + min;
            if (isFeatureChunk(chunkX, chunkZ, false) &&
                (!skipExistingChunks || !levelIn->hasChunkAt(BlockPos(chunkX * 16, 0, chunkZ * 16))))
            {
                return new BlockPos(chunkX * 16 + 8, 0x40, chunkZ * 16 + 8);
            }

            if (max != min)
            {
                chunkZ = centerZ + max;
                if (isFeatureChunk(chunkX, chunkZ, false) &&
                    (!skipExistingChunks || !levelIn->hasChunkAt(BlockPos(chunkX * 16, 0, chunkZ * 16))))
                {
                    return new BlockPos(chunkX * 16 + 8, 0x40, chunkZ * 16 + 8);
                }
            }

            for (int z = min + 1; z < max; ++z)
            {
                if (x != min && x != max)
                {
                    continue;
                }
                chunkZ = centerZ + z;
                if (isFeatureChunk(chunkX, chunkZ, false) &&
                    (!skipExistingChunks || !levelIn->hasChunkAt(BlockPos(chunkX * 16, 0, chunkZ * 16))))
                {
                    return new BlockPos(chunkX * 16 + 8, 0x40, chunkZ * 16 + 8);
                }
            }
        }

        --min;
        ++max;
    }
    return NULL;
}

NetherBridgeFeature::NetherBridgeStart::NetherBridgeStart(Level* levelIn, Random& random,
                                                           int chunkX, int chunkZ)
    : StructureStart(chunkX, chunkZ)
{
    NetherBridgePieces::StartPiece* startPiece = new NetherBridgePieces::StartPiece(
        random, chunkX * 16 + 2, chunkZ * 16 + 2, levelIn);
    push_back(startPiece);
    startPiece->addChildren(startPiece, this, random);

    while (!startPiece->mPendingPieces.empty())
    {
        std::vector<StructurePiece*>::iterator it = startPiece->mPendingPieces.begin();
        std::advance(it, random.nextInt((int)startPiece->mPendingPieces.size()));
        StructurePiece* piece = *it;
        startPiece->mPendingPieces.erase(it);
        piece->addChildren(piece, this, random);
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
