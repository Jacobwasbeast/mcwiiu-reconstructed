#include "game/world/chunk/ChunkInfoMapManager.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/LevelRenderer.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Minecraft.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerList.hpp"

static Level* sLevel;
static ChunkSource* sServerChunkSource;
static ChunkSource* sClientChunkSource;
static LevelRenderer* sLevelRenderer;

static const int CHUNK_INFO_MAP_SPLIT_SCREEN_SIZE = 64;
static const int CHUNK_INFO_MAP_SECTION_COUNT = 16;

static const unsigned char CHUNK_FLAG_BUILT = 1;

static const unsigned int CHUNK_STATUS_COLOUR_EMPTY = 0xFF404040;
static const unsigned int CHUNK_STATUS_COLOUR_MISSING = 0xFFFF0000;
static const unsigned int CHUNK_STATUS_COLOUR_NOT_BUILT = 0xFF808000;
static const unsigned int CHUNK_STATUS_COLOUR_BUILT = 0xFF00FF00;

void ChunkInfoMapManager::Render(float width, float height)
{
}

void ChunkInfoMapManager::renderStatusMap(BufferBuilder* bufferBuilder, float width, float height)
{
    int xzSize = sLevel->mDimension->getXZSize();
    int halfXZSize = xzSize / 2;

    PlayerList* players = NULL;
    if (gMinecraftServer != NULL)
    {
        players = gMinecraftServer->getPlayers();
    }

    int size;
    int centreChunkX;
    int centreChunkZ;
    if (players->getPlayerCount() < 2 && xzSize >= CHUNK_INFO_MAP_SPLIT_SCREEN_SIZE)
    {
        boost::shared_ptr<Player> player = Minecraft::GetInstance()->mPlayer;
        centreChunkX = Mth::clamp(player->mChunkX, CHUNK_INFO_MAP_SPLIT_SCREEN_SIZE / 2 - halfXZSize,
                                  halfXZSize - CHUNK_INFO_MAP_SPLIT_SCREEN_SIZE / 2);
        centreChunkZ = Mth::clamp(player->mChunkZ, CHUNK_INFO_MAP_SPLIT_SCREEN_SIZE / 2 - halfXZSize,
                                  halfXZSize - CHUNK_INFO_MAP_SPLIT_SCREEN_SIZE / 2);
        size = CHUNK_INFO_MAP_SPLIT_SCREEN_SIZE;
    }
    else
    {
        centreChunkX = 0;
        centreChunkZ = 0;
        size = xzSize;
    }

    int originChunkX = size / 2 - centreChunkX;
    int originChunkZ = size / 2 - centreChunkZ;

    float cellWidth = width / (float)size;
    float cellHeight = height / (float)size;

    for (int column = 0; column < size; column++)
    {
        bufferBuilder->begin(3);

        int chunkX = column - originChunkX;

        for (int row = 0; row < size; row++)
        {
            int chunkZ = row - originChunkZ;

            LevelChunk* chunk = sClientChunkSource->getChunk(chunkX, chunkZ);
            if ((!chunk->isReady() || chunk->isEmpty()) &&
                !sServerChunkSource->reallyHasChunk(chunkX, chunkZ))
            {
                RenderQuad((float)column * cellWidth, (float)row * cellHeight, cellWidth, cellHeight,
                           chunk->isEmpty() ? CHUNK_STATUS_COLOUR_EMPTY : CHUNK_STATUS_COLOUR_MISSING,
                           bufferBuilder);
            }

            int blockX = chunkX * 16;
            int blockZ = chunkZ * 16;

            unsigned int colour = CHUNK_STATUS_COLOUR_NOT_BUILT;
            for (int section = 0; section < CHUNK_INFO_MAP_SECTION_COUNT; section++)
            {
                if ((sLevelRenderer->getGlobalChunkFlags(blockX, section * 16, blockZ, sLevel) &
                     CHUNK_FLAG_BUILT) != 0)
                {
                    colour = CHUNK_STATUS_COLOUR_BUILT;
                    break;
                }
            }

            RenderQuad((float)column * cellWidth, (float)row * cellHeight, cellWidth, cellHeight,
                       colour, bufferBuilder);
        }

        bufferBuilder->end();
    }
}

void ChunkInfoMapManager::renderHeatMap(BufferBuilder* bufferBuilder, float width, float height)
{
    int xzSize = sLevel->mDimension->getXZSize();
    int halfXZSize = xzSize / 2;

    int maxMemoryAllocated = 0;
    for (int column = 0; column < xzSize; column++)
    {
        int chunkX = column - halfXZSize;

        for (int row = 0; row < xzSize; row++)
        {
            int memoryAllocated =
                sClientChunkSource->getChunk(chunkX, row - halfXZSize)->getSparseBlockAndDataMemoryAllocated();
            if (memoryAllocated > maxMemoryAllocated)
            {
                maxMemoryAllocated = memoryAllocated;
            }
        }
    }

    float cellWidth = width / (float)xzSize;
    float cellHeight = height / (float)xzSize;

    for (int column = 0; column < xzSize; column++)
    {
        bufferBuilder->begin(3);

        int chunkX = column - halfXZSize;

        for (int row = 0; row < xzSize; row++)
        {
            int memoryAllocated =
                sClientChunkSource->getChunk(chunkX, row - halfXZSize)->getSparseBlockAndDataMemoryAllocated();

            unsigned int heat = 0;
            if (maxMemoryAllocated > 0)
            {
                heat = (unsigned int)(memoryAllocated * 255 / maxMemoryAllocated);
            }

            RenderQuad((float)column * cellWidth, (float)row * cellHeight, cellWidth, cellHeight,
                       0xFF000000 | (heat << 16) | ((255 - heat) << 8), bufferBuilder);
        }

        bufferBuilder->end();
    }
}
