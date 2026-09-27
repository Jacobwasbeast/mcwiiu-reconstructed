#include "game/core/PortalForcer.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/Mth.hpp"
#include "game/core/Direction.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPattern.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/PortalBlock.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/server/ServerLevel.hpp"

PortalForcer::PortalPosition::PortalPosition(const BlockPos& pos, long long updateTime)
    : BlockPos(pos), lastUpdate(updateTime)
{
}

PortalForcer::PortalForcer(ServerLevel* newLevel)
    : level(newLevel), random(new Random(newLevel->getSeed()))
{
}

PortalForcer::~PortalForcer()
{
    delete random;
    boost::unordered_map<ChunkPos, PortalPosition*, ChunkPosKeyHash, ChunkPosKeyEq>::iterator it;
    for (it = portalCache.begin(); it != portalCache.end(); ++it)
    {
        delete it->second;
    }
}

bool PortalForcer::HasCachedPortal(const ChunkPos& chunkPos, BlockPos& portalPos)
{
    boost::unordered_map<ChunkPos, PortalPosition*, ChunkPosKeyHash, ChunkPosKeyEq>::iterator it = portalCache.find(chunkPos);
    if (it == portalCache.end())
    {
        return false;
    }

    PortalPosition* cached = it->second;
    LevelChunk* chunk = level->getChunkLoadedOrUnloadedAt(cached->x, cached->z);
    if (chunk == NULL || chunk->isEmpty() ||
        chunk->getBlockId(cached->x & 15, cached->y, cached->z & 15) == BlockId::PORTAL)
    {
        portalPos = *cached;
        cached->lastUpdate = level->getGameTime();
        return true;
    }

    portalPositions.erase(std::find(portalPositions.begin(), portalPositions.end(), chunkPos));
    delete cached;
    portalCache.erase(it);
    return false;
}

void PortalForcer::uncacheLeastUsedPortal()
{
    if (portalPositions.empty())
    {
        return;
    }

    std::vector<ChunkPos>::iterator least = portalPositions.begin();
    long long leastTime = portalCache[*least]->lastUpdate;
    for (std::vector<ChunkPos>::iterator pos = portalPositions.begin(); pos != portalPositions.end(); ++pos)
    {
        long long updateTime = portalCache[*pos]->lastUpdate;
        if (updateTime < leastTime)
        {
            leastTime = updateTime;
            least = pos;
        }
    }

    ChunkPos chunkPos = *least;
    delete portalCache[chunkPos];
    portalPositions.erase(least);
    portalCache.erase(chunkPos);
}

bool PortalForcer::findPortal(boost::shared_ptr<Entity> entity, float rotation, int chunkX, int chunkZ)
{
    ChunkPos chunkPos(chunkX, chunkZ);
    Level* world = static_cast<Level*>(level);
    BlockPos portalPos;
    double bestDistance = -1.0;
    if (!HasCachedPortal(chunkPos, portalPos))
    {
        int radius = world->mDimension->getType() == DimensionType::NETHER ? 48 : 128;
        int originX = Mth::floor(entity->mX);
        int originY = Mth::floor(entity->mY);
        int originZ = Mth::floor(entity->mZ);

        for (int x = originX - radius; x <= originX + radius; ++x)
        {
            double xDistance = (double)x + 0.5 - entity->mX;
            for (int z = originZ - radius; z <= originZ + radius; ++z)
            {
                double zDistance = (double)z + 0.5 - entity->mZ;
                LevelChunk* chunk = world->getChunkLoadedOrUnloadedAt(x, z);
                if (chunk == NULL || chunk->isEmpty())
                {
                    continue;
                }

                for (int y = world->getHeight() - 1; y >= 0; --y)
                {
                    if (chunk->getBlockId(x & 15, y, z & 15) != BlockId::PORTAL)
                    {
                        continue;
                    }
                    while (y > 0 && chunk->getBlockId(x & 15, y - 1, z & 15) == BlockId::PORTAL)
                    {
                        --y;
                    }
                    double yDistance = (double)y - entity->mY;
                    double distance = xDistance * xDistance + yDistance * yDistance + zDistance * zDistance;
                    if (bestDistance < 0.0 || distance < bestDistance)
                    {
                        bestDistance = distance;
                        portalPos = BlockPos(x, y, z);
                    }
                }
            }
        }

        if (bestDistance < 0.0)
        {
            return false;
        }

        if (portalCache.size() + 1 > 30)
        {
            uncacheLeastUsedPortal();
        }
        PortalPosition* position = new PortalPosition(portalPos, world->getGameTime());
        portalCache[chunkPos] = position;
        portalPositions.push_back(chunkPos);
    }

    BlockPattern::BlockPatternMatch* portal = static_cast<PortalBlock*>(Blocks::portal)->getPortalShape(world, portalPos);
    const Direction* forwards = portal->getForwards();
    bool negative = forwards->getClockWise()->getAxisDirection() == Direction::AxisDirection::NEGATIVE;
    BlockPos frontTopLeft = portal->getFrontTopLeft();
    Vec3* entranceOffset = entity->getPortalEntranceOffset();
    const Direction* entranceForwards = entity->getPortalEntranceForwards();
    double x = (double)portalPos.x + 0.5;
    double z = (double)portalPos.z + 0.5;
    double horizontal = forwards->getAxis() == Direction::Axis::X
                            ? (double)frontTopLeft.z
                            : (double)frontTopLeft.x;
    double y = (double)(frontTopLeft.y + 1) - entranceOffset->y * (double)portal->getHeight();
    if (negative)
    {
        horizontal += 1.0;
    }
    if (forwards->getAxis() == Direction::Axis::X)
    {
        z = (0.5 - entranceOffset->x) * (double)portal->getWidth() *
                (double)forwards->getClockWise()->getAxisDirection()->getStep() +
            horizontal;
    }
    else
    {
        x = (0.5 - entranceOffset->x) * (double)portal->getWidth() *
                (double)forwards->getClockWise()->getAxisDirection()->getStep() +
            horizontal;
    }

    double diagonal = 1.0;
    double xFromZ = 0.0;
    double zFromX = 0.0;
    const Direction* destinationBack = forwards->getOpposite();
    if (destinationBack != entranceForwards)
    {
        if (destinationBack == entranceForwards->getOpposite())
        {
            diagonal = -1.0;
        }
        else if (destinationBack == entranceForwards->getClockWise())
        {
            diagonal = 0.0;
            xFromZ = -1.0;
            zFromX = 1.0;
        }
        else
        {
            diagonal = 0.0;
            xFromZ = 1.0;
            zFromX = -1.0;
        }
    }

    double oldXd = entity->mXd;
    double oldZd = entity->mZd;
    entity->mXd = oldXd * diagonal + oldZd * xFromZ;
    entity->mZd = oldXd * zFromX + oldZd * diagonal;
    entity->mYRot = rotation - (float)(entranceForwards->getOpposite()->get2DDataValue() * 90) +
                    (float)(forwards->get2DDataValue() * 90);

    if (entity->GetType() == EntityType::SERVER_PLAYER)
    {
        boost::shared_ptr<ServerPlayer> player = boost::static_pointer_cast<ServerPlayer>(entity);
        player->mConnection->teleport(x, y, z, entity->mYRot, entity->mXRot, true, true);
    }
    else
    {
        entity->moveTo(x, y, z, entity->mYRot, entity->mXRot);
    }
    return true;
}

bool PortalForcer::createPortal(boost::shared_ptr<Entity> entity)
{
    Level* world = static_cast<Level*>(level);
    int originX = Mth::floor(entity->mX);
    int originY = Mth::floor(entity->mY);
    int originZ = Mth::floor(entity->mZ);
    int halfSize = (world->mDimension->getXZSize() << 4) >> 1;
    int limit = halfSize - 4;
    if (originX - 48 < -limit)
    {
        originX = -limit + 48;
    }
    else if (originX + 48 >= limit)
    {
        originX = limit - 48;
    }
    if (originZ - 48 < -limit)
    {
        originZ = -limit + 48;
    }
    else if (originZ + 48 >= limit)
    {
        originZ = limit - 48;
    }

    int bestX = originX;
    int bestY = originY;
    int bestZ = originZ;
    int bestOrientation = 0;
    int orientation = random->nextInt(4);
    double bestDistance = -1.0;
    int minimumX = originX - 48;
    int maximumX = originX + 48;
    int minimumZ = originZ - 48;
    int maximumZ = originZ + 48;

    for (int x = minimumX; x <= maximumX; ++x)
    {
        double xDistance = (double)x + 0.5 - entity->mX;
        for (int z = minimumZ; z <= maximumZ; ++z)
        {
            double zDistance = (double)z + 0.5 - entity->mZ;
            for (int y = world->getHeight() - 1; y >= 0; --y)
            {
                BlockPos pos(x, y, z);
                if (!world->isEmptyBlock(pos))
                {
                    continue;
                }
                while (y > 0 && world->isEmptyBlock(BlockPos(x, y - 1, z)))
                {
                    --y;
                }

                for (int attempt = 0; attempt < 4; ++attempt)
                {
                    int candidateOrientation = orientation + attempt;
                    int xDirection = candidateOrientation % 2;
                    int zDirection = 1 - xDirection;
                    if (candidateOrientation % 4 > 1)
                    {
                        xDirection = -xDirection;
                        zDirection = -zDirection;
                    }

                    bool clear = true;
                    for (int depth = 0; depth < 3 && clear; ++depth)
                    {
                        for (int width = 0; width < 4 && clear; ++width)
                        {
                            int testX = x + (width - 1) * xDirection + depth * zDirection;
                            int testZ = z + (width - 1) * zDirection - depth * xDirection;
                            if (testX < -limit || testX >= limit || testZ < -limit || testZ >= limit)
                            {
                                clear = false;
                                break;
                            }
                            for (int yOffset = -1; yOffset < 4; ++yOffset)
                            {
                                const BlockState* state = world->getBlockState(BlockPos(testX, y + yOffset, testZ));
                                if ((yOffset < 0 && !state->getMaterial()->isSolid()) ||
                                    (yOffset >= 0 && !world->isEmptyBlock(BlockPos(testX, y + yOffset, testZ))))
                                {
                                    clear = false;
                                    break;
                                }
                            }
                        }
                    }

                    if (clear)
                    {
                        double yDistance = (double)y - entity->mY;
                        double distance = xDistance * xDistance + yDistance * yDistance + zDistance * zDistance;
                        if (bestDistance < 0.0 || distance < bestDistance)
                        {
                            bestDistance = distance;
                            bestX = x;
                            bestY = y;
                            bestZ = z;
                            bestOrientation = candidateOrientation % 4;
                        }
                    }
                }
            }
        }
    }

    if (bestDistance < 0.0)
    {
        for (int x = minimumX; x <= maximumX; ++x)
        {
            double xDistance = (double)x + 0.5 - entity->mX;
            for (int z = minimumZ; z <= maximumZ; ++z)
            {
                double zDistance = (double)z + 0.5 - entity->mZ;
                for (int y = world->getHeight() - 1; y >= 0; --y)
                {
                    if (!world->isEmptyBlock(BlockPos(x, y, z)))
                    {
                        continue;
                    }
                    while (y > 0 && world->isEmptyBlock(BlockPos(x, y - 1, z)))
                    {
                        --y;
                    }

                    for (int attempt = 0; attempt < 2; ++attempt)
                    {
                        int xDirection = (orientation + attempt) % 2;
                        int zDirection = 1 - xDirection;
                        bool clear = true;
                        for (int width = 0; width < 4 && clear; ++width)
                        {
                            int testX = x + (width - 1) * xDirection;
                            int testZ = z + (width - 1) * zDirection;
                            if (testX < -limit || testX >= limit || testZ < -limit || testZ >= limit)
                            {
                                clear = false;
                                break;
                            }
                            for (int yOffset = -1; yOffset < 4; ++yOffset)
                            {
                                const BlockState* state = world->getBlockState(BlockPos(testX, y + yOffset, testZ));
                                if ((yOffset < 0 && !state->getMaterial()->isSolid()) ||
                                    (yOffset >= 0 && !world->isEmptyBlock(BlockPos(testX, y + yOffset, testZ))))
                                {
                                    clear = false;
                                    break;
                                }
                            }
                        }

                        if (clear)
                        {
                            double yDistance = (double)y - entity->mY;
                            double distance = xDistance * xDistance + yDistance * yDistance + zDistance * zDistance;
                            if (bestDistance < 0.0 || distance < bestDistance)
                            {
                                bestDistance = distance;
                                bestX = x;
                                bestY = y;
                                bestZ = z;
                                bestOrientation = (orientation + attempt) % 2;
                            }
                        }
                    }
                }
            }
        }
    }

    int xDirection = bestOrientation % 2;
    int zDirection = 1 - xDirection;
    if (bestOrientation % 4 > 1)
    {
        xDirection = -xDirection;
        zDirection = -zDirection;
    }

    if (bestDistance < 0.0)
    {
        bestY = Mth::clamp(bestY, 70, world->getHeight() - 10);
        for (int depth = 0; depth < 3; ++depth)
        {
            for (int width = 0; width < 2; ++width)
            {
                int x = bestX - zDirection + width * xDirection + depth * zDirection;
                int z = bestZ + xDirection + width * zDirection - depth * xDirection;
                world->setBlockAndUpdate(BlockPos(x, bestY - 1, z), Blocks::obsidian->defaultBlockState());
                world->setBlockAndUpdate(BlockPos(x, bestY, z), Blocks::air->defaultBlockState());
                world->setBlockAndUpdate(BlockPos(x, bestY + 1, z), Blocks::air->defaultBlockState());
                world->setBlockAndUpdate(BlockPos(x, bestY + 2, z), Blocks::air->defaultBlockState());
            }
        }
    }

    Direction::Axis* portalAxis = xDirection == 0 ? Direction::Axis::Z : Direction::Axis::X;
    const BlockState* portalState =
        Blocks::portal->getBlockState(PortalBlock::getDataForAxis(portalAxis));
    for (int pass = 0; pass < 4; ++pass)
    {
        for (int width = 0; width < 4; ++width)
        {
            int x = bestX - xDirection + width * xDirection;
            int z = bestZ - zDirection + width * zDirection;
            for (int yOffset = -1; yOffset < 4; ++yOffset)
            {
                bool frame = width == 0 || width == 3 || yOffset == -1 || yOffset == 3;
                world->setBlock(BlockPos(x, bestY + yOffset, z),
                                frame ? Blocks::obsidian->defaultBlockState() : portalState, Block::UPDATE_CLIENTS, false);
            }
        }
        for (int width = 0; width < 4; ++width)
        {
            int x = bestX - xDirection + width * xDirection;
            int z = bestZ - zDirection + width * zDirection;
            for (int yOffset = -1; yOffset < 4; ++yOffset)
            {
                BlockPos pos(x, bestY + yOffset, z);
                world->updateNeighborsAt(pos, world->getBlockState(pos)->getBlock(), false);
            }
        }
    }
    return true;
}

void PortalForcer::force(boost::shared_ptr<Entity> entity, float rotation, int chunkX, int chunkZ)
{
    Level* world = static_cast<Level*>(level);
    if (world->mDimension->getType() == DimensionType::THE_END)
    {
        int centerX = Mth::floor(entity->mX);
        int centerY = Mth::floor(entity->mY);
        int centerZ = Mth::floor(entity->mZ);
        for (int z = centerZ + 2; z >= centerZ - 2; --z)
        {
            for (int x = centerX - 2; x <= centerX + 2; ++x)
            {
                world->setBlockAndUpdate(BlockPos(x, centerY - 2, z), Blocks::obsidian->defaultBlockState());
                world->setBlockAndUpdate(BlockPos(x, centerY - 1, z), Blocks::air->defaultBlockState());
                world->setBlockAndUpdate(BlockPos(x, centerY, z), Blocks::air->defaultBlockState());
                world->setBlockAndUpdate(BlockPos(x, centerY + 1, z), Blocks::air->defaultBlockState());
            }
        }
        entity->moveTo((double)centerX, (double)(centerY - 1), (double)centerZ, entity->mYRot, 0.0f);
        entity->mXd = 0.0;
        entity->mYd = 0.0;
        entity->mZd = 0.0;
        return;
    }

    if (!findPortal(entity, rotation, chunkX, chunkZ))
    {
        createPortal(entity);
        findPortal(entity, rotation, chunkX, chunkZ);
    }
}

bool PortalForcer::isBlocked(const BlockPos& pos)
{
    Level* world = static_cast<Level*>(level);
    return !world->isEmptyBlock(pos) || !world->isEmptyBlock(pos.above());
}

void PortalForcer::tick(long long gameTime)
{
    (void)gameTime;
}
