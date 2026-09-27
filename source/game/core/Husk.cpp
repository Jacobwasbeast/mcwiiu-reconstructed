#include "game/core/Husk.hpp"
#include "game/core/EntityType.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

Husk::Husk(Level* level)
    : Zombie(level, false)
{
    fjDerivedCtorCalls();
}

Husk::~Husk()
{
}

void Husk::create(Level* level)
{
    new Husk(level);
}

void Husk::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::HUSK);
}

bool Husk::checkSpawnRules()
{
    if (!Monster::checkSpawnRules())
        return false;
    BlockPos pos(this);
    return mLevel->canSeeSky(pos);
}

bool Husk::isSunSensitive() const
{
    return false;
}

const SoundEvent* Husk::getAmbientSound()
{
    return SoundEvents::HUSK_AMBIENT;
}

const SoundEvent* Husk::getHurtSound(const DamageSource*)
{
    return SoundEvents::HUSK_HURT;
}

const SoundEvent* Husk::getDeathSound()
{
    return SoundEvents::HUSK_DEATH;
}

const SoundEvent* Husk::getStepSound()
{
    return SoundEvents::HUSK_STEP;
}

ResourceLocation* Husk::getDefaultLootTable()
{
    return &BuiltInLootTables::HUSK;
}

bool Husk::doHurtTarget(boost::shared_ptr<Entity> target)
{
    bool hurt = Zombie::doHurtTarget(target);
    if (!hurt)
        return false;
    not_null_ptr<ItemInstance> item = getMainHandItem();
    if (item->isEmpty() && target &&
        (target->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
    {
        boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(target);
        BlockPos pos(this);
        DifficultyInstance difficulty = mLevel->getCurrentDifficultyAt(pos);
        int duration = (int)difficulty.getEffectiveDifficulty() * 140;
        living->addEffect(new MobEffectInstance(MobEffects::HUNGER, duration, 0),
                          boost::shared_ptr<Entity>());
    }
    return true;
}

not_null_ptr<ItemInstance> Husk::getSkull()
{
    return ItemInstance::EMPTY;
}

void Husk::finishConversion()
{
    boost::shared_ptr<Zombie> zombie(new Zombie(mLevel));
    zombie->copyPosition(shared_from_this());

    BlockPos pos(zombie);
    DifficultyInstance difficulty = mLevel->getCurrentDifficultyAt(pos);
    if (isBaby())
    {
        Zombie::ZombieGroupData* group = new Zombie::ZombieGroupData(true, isJockey(), false);
        zombie->finalizeMobSpawn(difficulty, group, std::wstring());
    }
    else
    {
        Zombie::ZombieGroupData* group = new Zombie::ZombieGroupData(false, false, false);
        zombie->finalizeMobSpawn(difficulty, group, std::wstring());
    }

    playSound(SoundEvents::HUSK_CONVERTED_TO_ZOMBIE, 1.0f, 1.0f);
    mLevel->removeEntity(shared_from_this());
    dropEquipment(true, 0);
    zombie->setNoAi(isNoAi());
    zombie->setCanPickUpLoot(canPickUpLoot());
    zombie->setCanBreakDoors(canBreakDoors());
    mLevel->addEntity(zombie);
}

unsigned long long Husk::GetType()
{
    return EntityType::HUSK;
}
