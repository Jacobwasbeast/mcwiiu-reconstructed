#ifndef GAME_CORE_LOOTTABLECONTEXT_HPP
#define GAME_CORE_LOOTTABLECONTEXT_HPP

#include <list>

#include <boost/shared_ptr.hpp>

class DamageSource;
class Entity;
class LootTable;
class LootTables;
class ServerLevel;
class Player;

class LootTableContext
{
public:
    enum EntityTarget
    {
        THIS = 0,
        DIRECT_KILLER = 1,
        KILLER = 2
    };

    LootTableContext(float luck, ServerLevel* level, LootTables* lootTables,
                     boost::shared_ptr<Entity> thisEntity,
                     boost::shared_ptr<Player> killerEntity,
                     const DamageSource* damageSource);

    bool addVisitedTable(LootTable* lootTable);
    void removeVisitedTable(LootTable* lootTable);
    boost::shared_ptr<Entity> getEntity(EntityTarget target);

    float mLuck;
    ServerLevel* mLevel;
    LootTables* mLootTables;
    boost::shared_ptr<Entity> mThisEntity;
    boost::shared_ptr<Player> mKillerEntity;
    const DamageSource* mDamageSource;
    std::list<LootTable*> mVisitedTables;
};

#endif
