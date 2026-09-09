#include "game/entity/mob/EndDragonFight.hpp"

#include "game/server/ServerPlayer.hpp"

#include <cmath>

#include "game/util/AABB.hpp"
#include "game/core/Direction.hpp"
#include "game/data/IntTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/util/NbtUtils.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/EndGatewayFeature.hpp"
#include "game/world/generation/EndPodiumFeature.hpp"
#include "game/world/generation/SpikeFeature.hpp"
#include "game/world/biome/TheEndBiomeDecorator.hpp"

EndDragonFight::EndDragonFight(ServerLevel* levelValue, CompoundTag* data)
    : dragonEvent(new ServerBossEvent(0x5157A755, L"Dragon", BossEvent::PINK, BossEvent::PROGRESS)),
      level(levelValue),
      gateways(),
      ticksSinceDragonSeen(0),
      crystalsAlive(0),
      dragonKilled(false),
      previouslyKilledDragon(false),
      dragonUUID(),
      exitPortalFound(false),
      exitPortalLocation(0),
      respawnStage(0),
      respawnTime(0),
      crystals()
{
    dragonEvent->setPlayBossMusic(true);
    dragonEvent->setCreateWorldFog(true);
    if (data == 0)
    {
        previouslyKilledDragon = true;
        dragonKilled = true;
        gateways.push_back(0);
        return;
    }
    if (data->contains(L"DragonUUID"))
    {
        dragonUUID = data->getString(L"DragonUUID");
    }
    dragonKilled = data->getBoolean(L"DragonKilled");
    previouslyKilledDragon = data->getBoolean(L"PreviouslyKilled");
    if (data->getBoolean(L"IsRespawning"))
    {
        respawnTime = 1;
    }
    if (data->contains(L"ExitPortalLocation", 10))
    {
        exitPortalLocation = new BlockPos(NbtUtils::readBlockPos(data->getCompound(L"ExitPortalLocation")));
        exitPortalFound = true;
    }
    if (data->contains(L"Gateways", 9))
    {
        ListTag* list = data->getList(L"Gateways", 3);
        int i;
        for (i = 0; i < list->size(); ++i)
        {
            gateways.push_back(list->getInt(i));
        }
    }
    else
    {
        gateways.push_back(0);
    }
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
    std::vector<SpikeFeature::EndSpike>* spikes = TheEndBiomeDecorator::getSpikesForLevel(level);
    if (spikes == 0)
    {
        return;
    }
    std::vector<SpikeFeature::EndSpike>::iterator spike;
    for (spike = spikes->begin(); spike != spikes->end(); ++spike)
    {
        AABB* box = spike->getTopBoundingBox();
        std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(EnderCrystal), box);
        std::vector<boost::shared_ptr<Entity> >::iterator entity;
        for (entity = entities->begin(); entity != entities->end(); ++entity)
        {
            boost::shared_ptr<EnderCrystal> crystal = boost::static_pointer_cast<EnderCrystal>(*entity);
            if (crystal)
            {
                crystal->setBeamTarget(0);
            }
        }
        delete entities;
    }
    delete spikes;
}

void EndDragonFight::spawnExitPortal(bool active)
{
    EndPodiumFeature podium(active);
    if (exitPortalLocation == 0)
    {
        exitPortalLocation = new BlockPos(0, level->getSeaLevel(), 0);
        while (exitPortalLocation->y > level->getSeaLevel() &&
               level->getBlockState(*exitPortalLocation) == 0)
        {
            *exitPortalLocation = exitPortalLocation->below();
        }
    }
    Random random;
    podium.place(level, random, *exitPortalLocation);
    exitPortalFound = true;
}

void EndDragonFight::updateCrystalCount()
{
    crystalsAlive = 0;
    std::vector<SpikeFeature::EndSpike>* spikes = TheEndBiomeDecorator::getSpikesForLevel(level);
    if (spikes == 0)
    {
        return;
    }
    std::vector<SpikeFeature::EndSpike>::iterator spike;
    for (spike = spikes->begin(); spike != spikes->end(); ++spike)
    {
        AABB* box = spike->getTopBoundingBox();
        std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(EnderCrystal), box);
        crystalsAlive += static_cast<int>(entities->size());
        delete entities;
    }
    delete spikes;
}

void EndDragonFight::onCrystalDestroyed(boost::shared_ptr<EnderCrystal> crystal, const DamageSource* source)
{
    if (crystal)
    {
        updateCrystalCount();
        if (respawnStage != 0)
        {
            resetSpikeCrystals();
        }
    }
    if (dragonKilled && crystalsAlive == 0)
    {
        spawnExitPortal(true);
    }
}

bool EndDragonFight::findExitPortal()
{
    return false;
}

void EndDragonFight::fjFindCrystals(std::vector<boost::shared_ptr<EnderCrystal> >& result, const BlockPos& pos)
{
    AABB box(pos.x - 10.0, pos.y - 10.0, pos.z - 10.0,
             pos.x + 10.0, pos.y + 10.0, pos.z + 10.0);
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(EnderCrystal), &box);
    std::vector<boost::shared_ptr<Entity> >::iterator entity;
    for (entity = entities->begin(); entity != entities->end(); ++entity)
    {
        result.push_back(boost::static_pointer_cast<EnderCrystal>(*entity));
    }
    delete entities;
}

void EndDragonFight::respawnDragon(const std::vector<boost::shared_ptr<EnderCrystal> >& result)
{
    resetSpikeCrystals();
    spawnExitPortal(true);
    respawnTime = 1;
}

bool EndDragonFight::tryRespawn()
{
    if (!dragonKilled || respawnStage != 0)
    {
        return false;
    }
    std::vector<boost::shared_ptr<EnderCrystal> > found;
    fjFindCrystals(found, BlockPos(0, 0, 0));
    if (found.size() != 4)
    {
        return false;
    }
    respawnDragon(found);
    return true;
}

CompoundTag* EndDragonFight::saveData()
{
    if (!previouslyKilledDragon && dragonUUID.empty())
    {
        return new CompoundTag();
    }
    CompoundTag* data = new CompoundTag();
    if (!dragonUUID.empty())
    {
        data->putString(L"DragonUUID", dragonUUID);
    }
    data->putBoolean(L"DragonKilled", dragonKilled);
    data->putBoolean(L"PreviouslyKilled", previouslyKilledDragon);
    if (exitPortalLocation != 0)
    {
        data->put(L"ExitPortalLocation", NbtUtils::writeBlockPos(*exitPortalLocation));
    }
    if (respawnStage != 0)
    {
        data->putBoolean(L"IsRespawning", true);
    }
    ListTag* list = new ListTag(3);
    std::vector<int>::iterator gateway;
    for (gateway = gateways.begin(); gateway != gateways.end(); ++gateway)
    {
        list->add(new IntTag(*gateway));
    }
    data->put(L"Gateways", list);
    return data;
}

BlockPos EndDragonFight::getGatewayPosition(int index)
{
    const float angle = static_cast<float>(index) * 3.14159265358979323846f * 2.0f / 20.0f;
    return BlockPos(static_cast<int>(65.0f * std::cos(angle)), 75,
                    static_cast<int>(65.0f * std::sin(angle)));
}

bool EndDragonFight::hasExitPortal()
{
    return exitPortalFound && exitPortalLocation != 0;
}

void EndDragonFight::updatePlayers()
{
    std::vector<boost::shared_ptr<Player> >* players = level->getPlayers(0, 0);
    std::vector<boost::shared_ptr<Player> >::iterator player;
    for (player = players->begin(); player != players->end(); ++player)
    {
        if (*player)
        {
            dragonEvent->addPlayer(boost::static_pointer_cast<ServerPlayer>(*player), 0);
        }
    }
    delete players;
}

void EndDragonFight::ensureArenaLoaded()
{
    level->getChunkAt(BlockPos(0, 75, 0));
}

void EndDragonFight::createNewDragon()
{
    boost::shared_ptr<EnderDragon> dragon = EnderDragon::create(level);
    if (dragon)
    {
        dragon->AddParts();
        level->addEntity(dragon);
        updateDragon(dragon);
    }
}

void EndDragonFight::tick()
{
    ensureArenaLoaded();
    updatePlayers();
    if (respawnStage != 0)
    {
        ++respawnTime;
    }
    else if (!dragonKilled)
    {
        ++ticksSinceDragonSeen;
        updateCrystalCount();
    }
}

void EndDragonFight::setRespawnStage(DragonRespawnAnimation* stage)
{
    respawnStage = stage;
    respawnTime = 0;
    if (stage == 0)
    {
        createNewDragon();
    }
}

void EndDragonFight::spawnNewGateway(const BlockPos& pos, int index)
{
    level->levelEvent(3000, pos, 0);
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
    dragonKilled = true;
    previouslyKilledDragon = true;
    dragonEvent->setVisible(false);
    spawnExitPortal(true);
    spawnNewGateway();
}

void EndDragonFight::updateDragon(boost::shared_ptr<EnderDragon> dragon)
{
    if (!dragon)
    {
        return;
    }
    dragonEvent->setPercent(dragon->getHealth() / dragon->getMaxHealth());
    if (!dragon->isAlive())
    {
        setDragonKilled(dragon);
    }
}

bool EndDragonFight::hasPreviouslyKilledDragon()
{
    return previouslyKilledDragon;
}
