#include "game/network/PathNavigationListener.hpp"

#include "game/core/Path.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/util/AABB.hpp"

class Mob : public Entity
{
public:
    PathNavigation* getNavigation();
};

PathNavigationListener::PathNavigationListener()
    : mPathNavigations(), mReserved(0)
{
    InitializeCriticalSection(&mMutex);
}

PathNavigationListener::~PathNavigationListener()
{
    DeleteCriticalSection(&mMutex);
}

bool PathNavigationListener::didBlockChange(Level* level, const BlockPos& pos, const BlockState* oldState, const BlockState* newState)
{
    const AABB* oldShape = oldState->getShape((LevelSource*)level, pos);
    const AABB* newShape = newState->getShape((LevelSource*)level, pos);
    if (oldShape == newShape)
        return false;
    return oldShape == NULL || !oldShape->equals(newShape);
}

void PathNavigationListener::blockChanged(Level* level, const BlockPos& pos, const BlockState* oldState, const BlockState* newState, int, bool)
{
    if (!didBlockChange(level, pos, oldState, newState))
        return;
    EnterCriticalSection(&mMutex);
    for (std::vector<PathNavigation*>::iterator it = mPathNavigations.begin(); it != mPathNavigations.end(); ++it)
    {
        PathNavigation* navigation = *it;
        if (navigation == NULL)
            continue;
        if (!navigation->hasDelayedRecomputation() && navigation->getPath() != NULL && !navigation->getPath()->isDone() && navigation->getPath()->getSize() != 0)
        {
            Node* last = navigation->getPath()->last();
            double distance = pos.distSqr((last->x + navigation->mMob->mX - 0.5) * 0.5,
                                          (last->y + navigation->mMob->mY - 0.5) * 0.5,
                                          (last->z + navigation->mMob->mZ - 0.5) * 0.5);
            int remaining = navigation->getPath()->getSize() - navigation->getPath()->getIndex();
            if (distance < static_cast<double>(remaining * remaining))
                navigation->recomputePath();
        }
    }
    LeaveCriticalSection(&mMutex);
}

void PathNavigationListener::entityAdded(boost::shared_ptr<Entity> entity)
{
    if (entity && (entity->GetType() & ENTITY_TYPE_MOB) == ENTITY_TYPE_MOB)
    {
        EnterCriticalSection(&mMutex);
        Mob* mob = static_cast<Mob*>(entity.get());
        PathNavigation* navigation = mob->getNavigation();
        if (navigation != NULL)
            mPathNavigations.push_back(navigation);
        LeaveCriticalSection(&mMutex);
    }
}

void PathNavigationListener::entityRemoved(boost::shared_ptr<Entity> entity)
{
    if (entity && (entity->GetType() & ENTITY_TYPE_MOB) == ENTITY_TYPE_MOB)
    {
        EnterCriticalSection(&mMutex);
        PathNavigation* navigation = static_cast<Mob*>(entity.get())->getNavigation();
        std::vector<PathNavigation*>::iterator it = mPathNavigations.begin();
        while (it != mPathNavigations.end() && *it != navigation)
            ++it;
        if (it != mPathNavigations.end())
            mPathNavigations.erase(it);
        LeaveCriticalSection(&mMutex);
    }
}

void PathNavigationListener::blockLightChanged(const BlockPos&) {}
void PathNavigationListener::setBlocksDirty(int, int, int, int, int, int, Level*) {}
void PathNavigationListener::allChanged() {}
void PathNavigationListener::playStreamingMusic(const SoundEvent*, const BlockPos&, bool) {}
void PathNavigationListener::playSoundToAllPlayers(const SoundEvent*, SoundSource::ESoundSource, double, double, double, float, float, float) {}
void PathNavigationListener::playSound(boost::shared_ptr<Player>, const SoundEvent*, SoundSource::ESoundSource, double, double, double, float, float, float) {}
void PathNavigationListener::addParticle(Level*, ePARTICLE_TYPE, bool, double, double, double, double, double, double, arrayWithLength<int>) {}
void PathNavigationListener::addParticle(Level*, ePARTICLE_TYPE, bool, bool, double, double, double, double, double, double, arrayWithLength<int>) {}
void PathNavigationListener::playerRemoved(boost::shared_ptr<Entity>) {}
void PathNavigationListener::globalLevelEvent(int, const BlockPos&, int) {}
void PathNavigationListener::levelEvent(boost::shared_ptr<Player>, int, const BlockPos&, int) {}
void PathNavigationListener::destroyBlockProgress(int, const BlockPos&, int) {}
void PathNavigationListener::invalidateChunksOutsideArea(int, int, int, int) {}
