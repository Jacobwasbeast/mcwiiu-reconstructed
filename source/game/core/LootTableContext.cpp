#include "game/core/LootTableContext.hpp"

#include "game/core/DamageSource.hpp"
#include "game/world/generation/Player.hpp"

LootTableContext::LootTableContext(float luck, ServerLevel* level, LootTables* lootTables,
                                   boost::shared_ptr<Entity> thisEntity,
                                   boost::shared_ptr<Player> killerEntity,
                                   const DamageSource* damageSource)
{
    mLuck = luck;
    mLevel = level;
    mLootTables = lootTables;
    mThisEntity = thisEntity;
    mKillerEntity = killerEntity;
    mDamageSource = damageSource;
}

bool LootTableContext::addVisitedTable(LootTable* lootTable)
{
    for (std::list<LootTable*>::iterator it = mVisitedTables.begin();
         it != mVisitedTables.end();
         ++it)
    {
        if (*it == lootTable)
        {
            return false;
        }
    }

    mVisitedTables.push_back(lootTable);
    return true;
}

void LootTableContext::removeVisitedTable(LootTable* lootTable)
{
    for (std::list<LootTable*>::iterator it = mVisitedTables.begin();
         it != mVisitedTables.end();
         ++it)
    {
        if (*it == lootTable)
        {
            mVisitedTables.erase(it);
            return;
        }
    }
}

boost::shared_ptr<Entity> LootTableContext::getEntity(EntityTarget target)
{
    if (target == THIS)
    {
        return mThisEntity.lock();
    }

    if (target == DIRECT_KILLER)
    {
        return mDamageSource == NULL ? boost::shared_ptr<Entity>() : mDamageSource->getEntity();
    }

    if (target == KILLER)
    {
        return mKillerEntity.lock();
    }

    return boost::shared_ptr<Entity>();
}
