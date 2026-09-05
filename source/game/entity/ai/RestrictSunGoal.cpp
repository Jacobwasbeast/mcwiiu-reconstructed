#include "game/entity/ai/RestrictSunGoal.hpp"

#include "game/core/EquipmentSlot.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"

class PathfinderMob
{
public:
    Level* mLevel;
    not_null_ptr<ItemInstance> getItemSlot(const EquipmentSlot* slot);
    PathNavigation* getNavigation();
};

RestrictSunGoal::RestrictSunGoal(PathfinderMob* mob)
    : Goal(),
      mMob(mob)
{
}

RestrictSunGoal::~RestrictSunGoal()
{
}

bool RestrictSunGoal::canUse()
{
    if (!mMob->mLevel->isDay())
        return false;

    not_null_ptr<ItemInstance> helmet = mMob->getItemSlot(EquipmentSlot::HEAD);
    if (!helmet->isEmpty())
        return false;

    return dynamic_cast<GroundPathNavigation*>(mMob->getNavigation()) != NULL;
}

void RestrictSunGoal::start()
{
    static_cast<GroundPathNavigation*>(mMob->getNavigation())->setAvoidSun(true);
}

void RestrictSunGoal::stop()
{
    static_cast<GroundPathNavigation*>(mMob->getNavigation())->setAvoidSun(false);
}

std::wstring RestrictSunGoal::getName()
{
    return std::wstring(L"RestrictSun");
}
