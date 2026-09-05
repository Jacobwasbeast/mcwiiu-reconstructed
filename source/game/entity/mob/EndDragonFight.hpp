#ifndef GAME_ENTITY_MOB_ENDDRAGONFIGHT_HPP
#define GAME_ENTITY_MOB_ENDDRAGONFIGHT_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/DamageSource.hpp"
#include "game/core/EnderCrystal.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EnderDragon.hpp"
#include "game/server/ServerBossEvent.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/block/BlockPos.hpp"

class DragonRespawnAnimation;
class Level;

class EndDragonFight
{
public:
    EndDragonFight(ServerLevel* level, CompoundTag* data);
    ~EndDragonFight();

    int getCrystalsAlive();
    void resetSpikeCrystals();
    void spawnExitPortal(bool active);
    void updateCrystalCount();
    void onCrystalDestroyed(boost::shared_ptr<EnderCrystal> crystal, const DamageSource* source);
    bool findExitPortal();
    void fjFindCrystals(std::vector<boost::shared_ptr<EnderCrystal> >& crystals, const BlockPos& pos);
    void respawnDragon(const std::vector<boost::shared_ptr<EnderCrystal> >& crystals);
    bool tryRespawn();
    CompoundTag* saveData();
    static BlockPos getGatewayPosition(int index);
    bool hasExitPortal();
    void updatePlayers();
    void ensureArenaLoaded();
    void createNewDragon();
    void tick();
    void setRespawnStage(DragonRespawnAnimation* stage);
    void spawnNewGateway(const BlockPos& pos, int index);
    void spawnNewGateway();
    void setDragonKilled(boost::shared_ptr<EnderDragon> dragon);
    void updateDragon(boost::shared_ptr<EnderDragon> dragon);
    bool hasPreviouslyKilledDragon();

private:
    ServerBossEvent* dragonEvent;
    Level* level;
    std::vector<int> gateways;
    int ticksSinceDragonSeen;
    int crystalsAlive;
    bool dragonKilled;
    bool previouslyKilledDragon;
    std::wstring dragonUUID;
    bool exitPortalFound;
    BlockPos* exitPortalLocation;
    DragonRespawnAnimation* respawnStage;
    int respawnTime;
    std::vector<boost::shared_ptr<EnderCrystal> > crystals;
};

#endif
