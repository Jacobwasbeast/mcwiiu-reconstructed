#include "game/entity/ai/PlayWithItemsGoal.hpp"

#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/Dolphin.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"

PlayWithItemsGoal::~PlayWithItemsGoal()
{
}

bool PlayWithItemsGoal::canUse()
{
    if (!mDolphin->isInWater())
        return false;

    AABB* searchBox = mDolphin->mBoundingBox->grow(8.0, 8.0, 8.0);
    std::vector<boost::shared_ptr<Entity> >* entities =
        mDolphin->mLevel->getEntitiesOfClass(typeid(Entity), searchBox);
    bool result = true;
    if (entities->empty())
    {
        result = !mDolphin->getMainHandItem()->isEmpty();
    }
    delete entities;
    return result;
}

bool PlayWithItemsGoal::canContinueToUse()
{
    return mDolphin->isInWater();
}

void PlayWithItemsGoal::start()
{
    AABB* searchBox = mDolphin->mBoundingBox->grow(8.0, 8.0, 8.0);
    std::vector<boost::shared_ptr<Entity> >* entities =
        mDolphin->mLevel->getEntitiesOfClass(typeid(Entity), searchBox);
    if (!entities->empty())
    {
        mDolphin->getNavigation()->moveTo(entities->at(0), 1.2);
    }
    delete entities;
}

void PlayWithItemsGoal::stop()
{
    not_null_ptr<ItemInstance> item = mDolphin->getMainHandItem();
    if (!item->isEmpty())
    {
        mDolphin->drop(item);
        mDolphin->mLevel->broadcastEntityEvent(mDolphin->shared_from_this(), 0, 0);
    }
}

void PlayWithItemsGoal::tick()
{
    not_null_ptr<ItemInstance> item = mDolphin->getMainHandItem();
    if (!item->isEmpty())
    {
        mDolphin->drop(item);
        mDolphin->mLevel->broadcastEntityEvent(mDolphin->shared_from_this(), 0, 0);
        return;
    }

    AABB* searchBox = mDolphin->mBoundingBox->grow(8.0, 8.0, 8.0);
    std::vector<boost::shared_ptr<Entity> >* entities =
        mDolphin->mLevel->getEntitiesOfClass(typeid(Entity), searchBox);
    if (!entities->empty())
    {
        mDolphin->getNavigation()->moveTo(entities->at(0), 1.2);
    }
    delete entities;
}

std::wstring PlayWithItemsGoal::getName()
{
    return std::wstring(L"Play with Item Goal!!");
}
