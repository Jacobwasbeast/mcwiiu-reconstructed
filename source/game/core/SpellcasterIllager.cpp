#include "game/core/SpellcasterIllager.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/world/Level.hpp"

const EntityDataAccessor<unsigned char>* SpellcasterIllager::DATA_ID = NULL;

SpellcasterIllager::IllagerSpell* gSpell0 = new SpellcasterIllager::IllagerSpell(0, 0.0, 0.0, 0.0);
SpellcasterIllager::IllagerSpell* gSpell1 = new SpellcasterIllager::IllagerSpell(1, 0.7, 0.7, 0.8);
SpellcasterIllager::IllagerSpell* gSpell2 = new SpellcasterIllager::IllagerSpell(2, 0.4, 0.3, 0.35);
SpellcasterIllager::IllagerSpell* gSpell3 = new SpellcasterIllager::IllagerSpell(3, 0.7, 0.5, 0.2);
SpellcasterIllager::IllagerSpell* gSpell4 = new SpellcasterIllager::IllagerSpell(4, 0.3, 0.3, 0.8);
SpellcasterIllager::IllagerSpell* gSpell5 = new SpellcasterIllager::IllagerSpell(5, 0.1, 0.1, 0.2);

SpellcasterIllager::SpellcasterIllager(Level* level)
    : AbstractIllager(level), mSpellTicks(0), mCurrentSpell(gSpell0)
{
}

SpellcasterIllager::~SpellcasterIllager()
{
}

void SpellcasterIllager::staticCtor()
{
    DATA_ID = SynchedEntityData::defineId<unsigned char>(0x000000004D541840ULL,
                                                          0x000000004D541040ULL,
                                                          EntityDataSerializers::BYTE);
}

void SpellcasterIllager::defineSynchedData()
{
    AbstractIllager::defineSynchedData();
    getEntityData()->createDateItem(DATA_ID, (unsigned char)0);
}

void SpellcasterIllager::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    mSpellTicks = tag->getInt(L"SpellTicks");
}

void SpellcasterIllager::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putInt(L"SpellTicks", mSpellTicks);
}

bool SpellcasterIllager::isCastingSpell()
{
    if (mLevel->mIsClientSide)
    {
        return getEntityData()->get(DATA_ID) != 0;
    }
    return mSpellTicks > 0;
}

int SpellcasterIllager::getArmPose()
{
    return isCastingSpell() ? ILLAGER_ARM_POSE_SPELLCASTING : ILLAGER_ARM_POSE_CROSSED;
}

void SpellcasterIllager::setIsCastingSpell(const IllagerSpell* spell)
{
    mCurrentSpell = spell;
    getEntityData()->set(DATA_ID, (unsigned char)spell->mId, true);
}

const SpellcasterIllager::IllagerSpell* SpellcasterIllager::getCurrentSpell()
{
    if (!mLevel->mIsClientSide)
    {
        return mCurrentSpell;
    }
    return IllagerSpell::byId(getEntityData()->get(DATA_ID));
}

void SpellcasterIllager::customServerAiStep()
{
    Mob::customServerAiStep();
}

void SpellcasterIllager::tick()
{
    Monster::tick();
    if (mLevel->mIsClientSide && isCastingSpell())
    {
        const IllagerSpell* spell = getCurrentSpell();
        float angle = mYBodyRot * 0.017453292f
                      + Mth::cos((float)mTickCount * 0.6662f) * 0.25f;
        double xOffset = (double)Mth::cos(angle) * 0.6;
        double zOffset = (double)Mth::sin(angle) * 0.6;
        mLevel->addParticle(PARTICLE_MOB_SPELL, mX + xOffset, mY + 1.8, mZ + zOffset,
                            spell->mX, spell->mY, spell->mZ, arrayWithLength<int>());
        mLevel->addParticle(PARTICLE_MOB_SPELL, mX - xOffset, mY + 1.8, mZ - zOffset,
                            spell->mX, spell->mY, spell->mZ, arrayWithLength<int>());
    }
}

int SpellcasterIllager::getSpellCastingTime()
{
    return mSpellTicks;
}

SpellcasterIllager::IllagerSpell::IllagerSpell(int id, double x, double y, double z)
    : mId(id), mX(x), mY(y), mZ(z)
{
}

SpellcasterIllager::IllagerSpell* SpellcasterIllager::IllagerSpell::byId(int id)
{
    if (id == 1) return gSpell1;
    if (id == 2) return gSpell2;
    if (id == 3) return gSpell3;
    if (id == 4) return gSpell4;
    if (id == 5) return gSpell5;
    return gSpell0;
}

SpellcasterIllager::SpellcasterUseSpellGoal::SpellcasterUseSpellGoal(SpellcasterIllager* spellcaster)
    : Goal(), mSpellcaster(spellcaster), mSpellWarmup(0), mNextAttackTickCount(0)
{
    setRequiredControlFlags(3);
}

SpellcasterIllager::SpellcasterUseSpellGoal::~SpellcasterUseSpellGoal()
{
}

bool SpellcasterIllager::SpellcasterUseSpellGoal::canUse()
{
    return mSpellcaster->getTarget() != NULL && !mSpellcaster->isCastingSpell()
        && mNextAttackTickCount <= mSpellcaster->mTickCount;
}

bool SpellcasterIllager::SpellcasterUseSpellGoal::canContinueToUse()
{
    boost::shared_ptr<LivingEntity> target = mSpellcaster->getTarget();
    return target != NULL && mSpellcaster->isCastingSpell();
}

bool SpellcasterIllager::SpellcasterUseSpellGoal::canInterrupt()
{
    return false;
}

void SpellcasterIllager::SpellcasterUseSpellGoal::start()
{
    mSpellWarmup = getCastWarmupTime();
    mSpellcaster->mSpellTicks = getCastWarmupTime() + getCastingTime();
    mNextAttackTickCount = mSpellcaster->mTickCount + getCastingInterval();
    mSpellcaster->getNavigation()->stop();
}

void SpellcasterIllager::SpellcasterUseSpellGoal::stop()
{
    Goal::stop();
    mSpellcaster->mSpellTicks = 0;
    mSpellcaster->setIsCastingSpell(gSpell0);
}

void SpellcasterIllager::SpellcasterUseSpellGoal::tick()
{
    if (mSpellcaster->getTarget() != NULL)
    {
        mSpellcaster->getLookControl()->setLookAt(mSpellcaster->getTarget(),
                                                  (float)mSpellcaster->getMaxHeadYRot(),
                                                  (float)mSpellcaster->getMaxHeadXRot());
    }

    if (mSpellWarmup > 0 && --mSpellWarmup == 0)
    {
        mSpellcaster->setIsCastingSpell(getSpell());
        const SoundEvent* sound = getSpellPrepareSound();
        if (sound != NULL)
        {
            mSpellcaster->playSound(sound, 1.0f, 1.0f);
        }
    }

    if (mSpellcaster->mSpellTicks > 0 && --mSpellcaster->mSpellTicks == 0)
    {
        performSpellCasting();
        mSpellcaster->playSound(mSpellcaster->getCastingSoundEvent(), 1.0f, 1.0f);
    }
}

int SpellcasterIllager::SpellcasterUseSpellGoal::getCastWarmupTime()
{
    return 20;
}

std::wstring SpellcasterIllager::SpellcasterUseSpellGoal::getName()
{
    return L"SpellcasterUseSpellGoal";
}
