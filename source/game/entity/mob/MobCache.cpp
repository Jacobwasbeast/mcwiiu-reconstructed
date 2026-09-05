#include "game/entity/mob/MobCache.hpp"

#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/server/ServerLevel.hpp"

MobCache::MobCache()
{
}

boost::shared_ptr<Mob> MobCache::getTestMob(ServerLevel* level, unsigned long long entityType)
{
    iterator found = find(entityType);

    if (found == end())
    {
        boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(EntityIO::newEntity(entityType, level));
        (*this)[entityType] = mob;
    }
    else
    {
        found->second->setLevel(level);
    }

    return find(entityType)->second;
}

void MobCache::removeFromCache(boost::shared_ptr<Mob> mob)
{
    erase(find(mob->GetType()));
}
