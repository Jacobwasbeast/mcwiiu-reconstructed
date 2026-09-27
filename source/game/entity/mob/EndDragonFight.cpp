#include "game/entity/mob/EndDragonFight.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/LevelEvent.hpp"

#include "game/server/ServerPlayer.hpp"

#include <algorithm>
#include <cmath>
#include <boost/assert.hpp>

#include "game/util/AABB.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/entity/ai/EntitySelector.hpp"
#include "game/core/Mth.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GenericStats.hpp"
#include "game/data/IntTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/DragonRespawnAnimation.hpp"
#include "game/util/NbtUtils.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/TheEndPortalBlockEntity.hpp"
#include "game/world/generation/EndGatewayFeature.hpp"
#include "game/world/generation/EndPodiumFeature.hpp"
#include "game/world/generation/SpikeFeature.hpp"
#include "game/world/biome/TheEndBiomeDecorator.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "game/core/System.hpp"

struct EndGatewaySorter
{
    bool operator()(int first, int second) const
    {
        return first != 0 && second == 0;
    }
};

Predicate<boost::shared_ptr<Player> >* EndDragonFight::VALID_PLAYER =
    new Predicates<boost::shared_ptr<Player> >::FunctionPredicate(validPlayerPredicate);

EndDragonFight::EndDragonFight(ServerLevel* levelValue, CompoundTag* data)
    : level(levelValue),
      mReserved(0),
      ticksSinceDragonSeen(0),
      crystalsAlive(0),
      ticksSinceCrystalsScanned(0),
      ticksSinceLastPlayerScan(0),
      dragonKilled(false),
      previouslyKilledDragon(false),
      needsStateScanning(true),
      exitPortalLocation(NULL),
      respawnStage(NULL),
      respawnTime(0)
{
    ServerBossEvent* bossEvent = new ServerBossEvent(IDS_ENDERDRAGON, L"", BossEvent::PINK, BossEvent::PROGRESS);
    bossEvent->setPlayBossMusic(true);
    dragonEvent = static_cast<ServerBossEvent*>(bossEvent->setCreateWorldFog(true));

    if (data != NULL && data->contains(L"DragonKilled", Tag::TAG_ANY_NUMERIC) && !gApp.GetResetEnd())
    {
        if (data->contains(L"DragonUUID"))
            dragonUUID = data->getString(L"DragonUUID");
        dragonKilled = data->getBoolean(L"DragonKilled");
        previouslyKilledDragon = data->getBoolean(L"PreviouslyKilled");
        if (data->getBoolean(L"IsRespawning"))
            respawnStage = DragonRespawnAnimation::START;
        if (data->contains(L"ExitPortalLocation", Tag::TAG_COMPOUND))
            exitPortalLocation = new BlockPos(NbtUtils::readBlockPos(data->getCompound(L"ExitPortalLocation")));
    }
    else
    {
        previouslyKilledDragon = true;
        dragonKilled = true;
    }

    if (data != NULL && data->contains(L"Gateways", Tag::TAG_LIST) && !gApp.GetResetEnd())
    {
        ListTag* list = data->getList(L"Gateways", Tag::TAG_INT);
        for (int i = 0; i < list->size(); ++i)
            gateways.push_back(list->getInt(i));
    }
    else
    {
        for (int i = 0; i < 1; ++i)
            gateways.push_back(i);
        Random random(level->getSeed());
        Mth::shuffle(&gateways, random);
    }
    mReserved = 0;
}

EndDragonFight::~EndDragonFight()
{
    delete exitPortalLocation;
    delete dragonEvent;
}

int EndDragonFight::getCrystalsAlive()
{
    return crystalsAlive;
}

void EndDragonFight::resetSpikeCrystals()
{
    arrayWithLength<SpikeFeature::EndSpike> spikes = TheEndBiomeDecorator::getSpikesForLevel(level);
    for (unsigned int i = 0; i < spikes.length; ++i)
    {
        AABB* box = spikes.data[i].getTopBoundingBox();
        std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(EnderCrystal), box);
        for (std::vector<boost::shared_ptr<Entity> >::iterator entity = entities->begin(); entity != entities->end();
             ++entity)
        {
            boost::shared_ptr<EnderCrystal> crystal = boost::static_pointer_cast<EnderCrystal>(*entity);
            crystal->setInvulnerable(false);
            crystal->setBeamTarget(NULL);
        }
        delete entities;
    }
    delete[] spikes.data;
}

void EndDragonFight::spawnExitPortal(bool active)
{
    EndPodiumFeature podium(active);
    if (exitPortalLocation == 0)
    {
        exitPortalLocation = new BlockPos(EndPodiumFeature::END_PODIUM_LOCATION);
        while (level->getBlockState(*exitPortalLocation)->getBlock() == Blocks::bedrock &&
               exitPortalLocation->y > level->getSeaLevel())
        {
            *exitPortalLocation = exitPortalLocation->below();
        }
    }
    Random random;
    podium.place(level, random, *exitPortalLocation);
}

void EndDragonFight::updateCrystalCount()
{
    crystalsAlive = 0;
    arrayWithLength<SpikeFeature::EndSpike> spikes = TheEndBiomeDecorator::getSpikesForLevel(level);
    for (unsigned int i = 0; i < spikes.length; ++i)
    {
        AABB* box = spikes.data[i].getTopBoundingBox();
        std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(EnderCrystal), box);
        crystalsAlive += static_cast<int>(entities->size());
        delete entities;
    }
    delete[] spikes.data;
}

void EndDragonFight::onCrystalDestroyed(boost::shared_ptr<EnderCrystal> crystal, const DamageSource* source)
{
    std::vector<boost::shared_ptr<EnderCrystal> >::iterator tracked = respawnCrystals.begin();
    while (tracked != respawnCrystals.end() && *tracked != crystal)
        ++tracked;

    if (respawnStage != NULL && tracked != respawnCrystals.end())
    {
        for (std::vector<boost::shared_ptr<EnderCrystal> >::iterator it = respawnCrystals.begin();
             it != respawnCrystals.end(); ++it)
        {
            if (*it != crystal)
                (*it)->setBeamTarget(NULL);
        }
        respawnStage = NULL;
        respawnTime = 0;
        resetSpikeCrystals();
        spawnExitPortal(true);
        return;
    }

    updateCrystalCount();
    boost::shared_ptr<Entity> dragon = level->getEntityByUUID(dragonUUID);
    if (Entity::instanceof(dragon, EntityType::ENDER_DRAGON))
        boost::static_pointer_cast<EnderDragon>(dragon)->onCrystalDestroyed(crystal, BlockPos(crystal), source);
}

BlockPattern::BlockPatternMatch* EndDragonFight::findExitPortal()
{
    return NULL;
}

void EndDragonFight::fjFindCrystals(std::vector<boost::shared_ptr<EnderCrystal> >& result, const BlockPos& pos)
{
    result.clear();
    BlockPos centre = pos.above(1);
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        AABB* box = AABB::newTemp(centre.relative(directions.data[i], 2));
        std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(EnderCrystal), box);
        if (entities->empty())
        {
            delete entities;
            return;
        }
        for (std::vector<boost::shared_ptr<Entity> >::iterator entity = entities->begin(); entity != entities->end();
             ++entity)
            result.push_back(boost::static_pointer_cast<EnderCrystal>(*entity));
        delete entities;
    }
}

void EndDragonFight::respawnDragon(const std::vector<boost::shared_ptr<EnderCrystal> >& crystals)
{
    if (!dragonKilled || respawnStage != NULL)
        return;
    if (!level->getLevelData()->getModernEnd())
        return;

    respawnStage = DragonRespawnAnimation::START;
    respawnTime = 0;
    spawnExitPortal(false);
    respawnCrystals = crystals;
}

void EndDragonFight::tryRespawn()
{
    ServerLevel* endLevel = gMinecraftServer->getLevel(1);
    if (!dragonKilled || respawnStage != NULL)
        return;
    if (!endLevel->getLevelData()->getModernEnd())
        return;

    if (exitPortalLocation == NULL && findExitPortal() == NULL)
        spawnExitPortal(true);
    BlockPos portalPos = *exitPortalLocation;

    std::vector<boost::shared_ptr<EnderCrystal> > crystals;
    fjFindCrystals(crystals, portalPos);
    if (crystals.size() == 4)
        respawnDragon(crystals);
}

CompoundTag* EndDragonFight::saveData()
{
    if (!level->getLevelData()->getModernEnd())
        return new CompoundTag();

    CompoundTag* data = new CompoundTag();
    if (!dragonUUID.empty())
        data->putString(L"DragonUUID", dragonUUID);
    data->putBoolean(L"DragonKilled", dragonKilled);
    data->putBoolean(L"PreviouslyKilled", previouslyKilledDragon);
    if (exitPortalLocation != NULL)
        data->put(L"ExitPortalLocation", NbtUtils::writeBlockPos(*exitPortalLocation));
    if (respawnStage != NULL)
        data->putBoolean(L"IsRespawning", true);

    ListTag* list = new ListTag(0);
    for (std::vector<int>::iterator gateway = gateways.begin(); gateway != gateways.end(); ++gateway)
        list->add(new IntTag(*gateway));
    data->put(L"Gateways", list);
    return data;
}

BlockPos EndDragonFight::getGatewayPosition(int index)
{
    const float PI = 3.1415927f;
    const float angle = (PI * static_cast<float>(index) - PI) * 2.0f;
    return BlockPos(static_cast<int>(65.0f * std::cos(angle)), 75, static_cast<int>(65.0f * std::sin(angle)));
}

bool EndDragonFight::hasExitPortal()
{
    for (int chunkX = -8; chunkX <= 8; ++chunkX)
        for (int chunkZ = -8; chunkZ <= 8; ++chunkZ)
        {
            LevelChunk::BlockEntityMap* blockEntities = level->getChunk(chunkX, chunkZ)->getBlockEntities();
            for (LevelChunk::BlockEntityMap::iterator it = blockEntities->begin(); it != blockEntities->end(); ++it)
            {
                if ((it->second->GetType() & BlockEntityType::THE_END_PORTAL) == BlockEntityType::THE_END_PORTAL)
                    return true;
            }
        }
    return false;
}

void EndDragonFight::updatePlayers()
{
    std::vector<boost::shared_ptr<ServerPlayer> > current;
    std::vector<boost::shared_ptr<Player> >* players = level->getPlayers(EntityType::SERVER_PLAYER, VALID_PLAYER);
    for (std::vector<boost::shared_ptr<Player> >::iterator player = players->begin(); player != players->end();
         ++player)
    {
        boost::shared_ptr<ServerPlayer> serverPlayer = boost::static_pointer_cast<ServerPlayer>(*player);
        dragonEvent->addPlayer(serverPlayer, 0);
        current.push_back(serverPlayer);
    }
    delete players;

    std::vector<boost::shared_ptr<ServerPlayer> > departed = dragonEvent->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator player = current.begin(); player != current.end();
         ++player)
    {
        std::vector<boost::shared_ptr<ServerPlayer> >::iterator found =
            std::find(departed.begin(), departed.end(), *player);
        if (found != departed.end())
            departed.erase(found);
    }
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator player = departed.begin(); player != departed.end();
         ++player)
        dragonEvent->removePlayer(*player);
}

void EndDragonFight::ensureArenaLoaded()
{
    System::processTimeInMilliSecs();
    for (int chunkX = -8; chunkX <= 8; ++chunkX)
        for (int chunkZ = -8; chunkZ <= 8; ++chunkZ)
        {
            level->reallyHasChunk(chunkX, chunkZ);
            level->getChunkSource()->getOrCreateChunk(chunkX, chunkZ, false);
        }
}

void EndDragonFight::createNewDragon()
{
    level->getChunkAt(BlockPos(0, 128, 0));
    boost::shared_ptr<EnderDragon> dragon = EnderDragon::create(level);
    dragon->AddParts();
    dragon->moveTo(0.0, 128.0, 0.0, level->mRandom->nextFloat() * 360.0f, 0.0f);
    level->addEntity(dragon);
    dragonUUID = dragon->getUUID();
}

void EndDragonFight::tick()
{
    std::vector<boost::shared_ptr<ServerPlayer> > players = dragonEvent->getPlayers();
    dragonEvent->setVisible(!dragonKilled);

    if (++ticksSinceLastPlayerScan >= 20)
    {
        updatePlayers();
        ticksSinceLastPlayerScan = 0;
        for (int index = 0; index < 1; ++index)
        {
            BlockPos gatewayPos = getGatewayPosition(index);
            std::vector<int>::iterator pending = std::find(gateways.begin(), gateways.end(), index);
            if (!level->hasChunkAt(gatewayPos))
                continue;
            const bool gatewayPresent = level->getBlockState(gatewayPos)->getBlock() == Blocks::end_gateway;
            if (pending == gateways.end())
            {
                if (!gatewayPresent)
                    gateways.push_back(index);
            }
            else if (gatewayPresent)
            {
                gateways.erase(pending);
            }
        }
        std::sort(gateways.begin(), gateways.end(), EndGatewaySorter());
    }

    if (!players.empty())
    {
        const double radius = level->mDimension->getXZSize() * 16.0 * 0.5;
        AABB* arena = AABB::newPermanent(-radius, 0.0, -radius, radius, level->getMaxBuildHeight(), radius);

        if (needsStateScanning)
        {
            ensureArenaLoaded();
            needsStateScanning = false;
            if (respawnStage == NULL)
            {
                const bool portalExists = hasExitPortal();
                if (portalExists)
                {
                    previouslyKilledDragon = true;
                }
                else
                {
                    previouslyKilledDragon = false;
                    spawnExitPortal(false);
                }

                std::vector<boost::shared_ptr<Entity> >* dragons =
                    level->getEntitiesOfClass(typeid(EnderDragon), arena, EntitySelector::ENTITY_STILL_ALIVE, false);
                if (dragons->empty())
                {
                    dragonKilled = true;
                }
                else
                {
                    boost::shared_ptr<EnderDragon> dragon = boost::static_pointer_cast<EnderDragon>(dragons->at(0));
                    dragonUUID = dragon->getUUID();
                    dragonKilled = false;
                    if (portalExists)
                    {
                        dragon->remove();
                        dragonUUID = L"";
                    }
                }
                if (!previouslyKilledDragon && dragonKilled)
                    dragonKilled = false;
                delete dragons;
            }
            if (respawnStage != NULL)
            {
                respawnTime = 0;
                fjFindCrystals(respawnCrystals, EndPodiumFeature::END_PODIUM_LOCATION);
            }
        }

        if (respawnStage != NULL)
            respawnStage->tick(level, this, &respawnCrystals, respawnTime++, BlockPos(0, level->getSeaLevel(), 0));

        if (!dragonKilled && respawnStage == NULL)
        {
            if (dragonUUID.empty() || ++ticksSinceDragonSeen >= 1200)
            {
                ensureArenaLoaded();
                std::vector<boost::shared_ptr<Entity> >* dragons =
                    level->getEntitiesOfClass(typeid(EnderDragon), arena, EntitySelector::ENTITY_STILL_ALIVE, false);
                if (dragons->empty())
                    createNewDragon();
                else
                    dragonUUID = dragons->at(0)->getUUID();
                delete dragons;
                ticksSinceDragonSeen = 0;
            }
            if (++ticksSinceCrystalsScanned >= 100)
            {
                updateCrystalCount();
                ticksSinceCrystalsScanned = 0;
            }
        }
        delete arena;
    }
}

void EndDragonFight::setRespawnStage(DragonRespawnAnimation* stage)
{
    respawnTime = 0;
    if (stage == DragonRespawnAnimation::END)
    {
        dragonKilled = false;
        respawnStage = 0;
        createNewDragon();
        std::vector<boost::shared_ptr<ServerPlayer> > players = dragonEvent->getPlayers();
        std::vector<boost::shared_ptr<ServerPlayer> >::iterator player;
        for (player = players.begin(); player != players.end(); ++player)
        {
            (*player)->awardStat(GenericStats::theEndAgain(), GenericStats::param_theEndAgain());
        }
        return;
    }
    respawnStage = stage;
}

void EndDragonFight::spawnNewGateway(const BlockPos& pos, int index)
{
    level->levelEvent(LevelEvent::ANIMATION_END_GATEWAY_SPAWN, pos, 0);
    Random random;
    EndGatewayFeature feature(index);
    feature.place(level, random, pos);
}

void EndDragonFight::spawnNewGateway()
{
    if (gateways.empty())
    {
        return;
    }
    int index = gateways.back();
    gateways.pop_back();
    spawnNewGateway(getGatewayPosition(index), index);
}

void EndDragonFight::setDragonKilled(boost::shared_ptr<EnderDragon> dragon)
{
    if (dragonUUID == dragon->getUUID())
    {
        dragonEvent->setPercent(0.0f);
        dragonEvent->setVisible(false);
        spawnExitPortal(true);
        if (level->getLevelData()->getModernEnd())
            spawnNewGateway();
        if (!previouslyKilledDragon)
        {
            BlockPos topBlock(EndPodiumFeature::END_PODIUM_LOCATION.x,
                              level->getAboveTopSolidBlock(EndPodiumFeature::END_PODIUM_LOCATION, false, false),
                              EndPodiumFeature::END_PODIUM_LOCATION.z);
            level->setBlockAndUpdate(topBlock, Blocks::dragon_egg->defaultBlockState());
        }
        dragonKilled = true;
        previouslyKilledDragon = true;
    }
}

void EndDragonFight::updateDragon(boost::shared_ptr<EnderDragon> dragon)
{
    if (dragonUUID == dragon->getUUID())
    {
        dragonEvent->setPercent(dragon->getHealth() / dragon->getMaxHealth());
        ticksSinceDragonSeen = 0;
        if (dragon->hasCustomName())
            dragonEvent->setName(dragon->getDisplayName());
    }
}

bool EndDragonFight::hasPreviouslyKilledDragon()
{
    return previouslyKilledDragon;
}
