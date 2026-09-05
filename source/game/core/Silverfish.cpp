#include "game/core/Silverfish.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MonsterEggBlock.hpp"

Silverfish::Silverfish(Level* level)
    : Monster(level), mWakeUpFriendsGoal(this)
{
    mCategories |= 0x2000;
    setSize(0.4f, 0.3f);
}

Silverfish::~Silverfish()
{
}

void Silverfish::create(Level* level)
{
    new Silverfish(level);
}

void Silverfish::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer);
}

void Silverfish::registerGoals()
{
    Mob::registerGoals();
}

double Silverfish::getRidingHeight()
{
    return 0.1;
}

double Silverfish::getEyeHeight()
{
    return 0.1;
}

void Silverfish::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(8.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(1.0);
}

bool Silverfish::makeStepSound()
{
    return true;
}

const SoundEvent* Silverfish::getAmbientSound()
{
    return SoundEvents::SILVERFISH_AMBIENT;
}

const SoundEvent* Silverfish::getHurtSound(const DamageSource* source)
{
    (void)source;
    return SoundEvents::SILVERFISH_HURT;
}

const SoundEvent* Silverfish::getDeathSound()
{
    return SoundEvents::SILVERFISH_DEATH;
}

void Silverfish::playStepSound(const BlockPos& pos, Block* block)
{
    playSound(SoundEvents::SILVERFISH_STEP, 1.0f, 0.8f);
}

bool Silverfish::hurt(const DamageSource* source, float damage)
{
    if (source != NULL)
    {
        boost::shared_ptr<Entity> attacker = source->getEntity();
        if (attacker && (attacker->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
        {
            mWakeUpFriendsGoal.notifyHurt();
        }
    }
    return Monster::hurt(source, damage);
}

const SoundEvent* Silverfish::getDeathLoot()
{
    return NULL;
}

void Silverfish::tick()
{
    mYBodyRotO = mYBodyRot;
    Monster::tick();
}

void Silverfish::setYBodyRot(float yBodyRot)
{
    mYBodyRot = yBodyRot;
    LivingEntity::setYBodyRot(yBodyRot);
}

double Silverfish::getWalkTargetValue(const BlockPos& pos)
{
    if (mLevel->getBlockState(pos.below())->getBlock() == Blocks::stone)
    {
        return 10.0;
    }
    return Monster::getWalkTargetValue(pos);
}

bool Silverfish::isDarkEnoughToSpawn()
{
    return true;
}

bool Silverfish::checkSpawnRules()
{
    if (!Monster::checkSpawnRules())
    {
        return false;
    }
    return !mLevel->getNearestPlayer(boost::static_pointer_cast<Entity>(shared_from_this()), 5.0, 1.0);
}

EMobType Silverfish::getMobType()
{
    return MOB_TYPE_ARTHROPOD;
}

unsigned long long Silverfish::GetType()
{
    return 0x5D540044ULL;
}

Silverfish::SilverfishMergeWithStoneGoal::SilverfishMergeWithStoneGoal(Silverfish* silverfish)
    : RandomStrollGoal(silverfish, 1.0, 10), mDirection(0), mShouldMerge(false)
{
    setRequiredControlFlags(1);
}

Silverfish::SilverfishMergeWithStoneGoal::~SilverfishMergeWithStoneGoal()
{
}

bool Silverfish::SilverfishMergeWithStoneGoal::canUse()
{
    Silverfish* silverfish = static_cast<Silverfish*>(mMob);
    if (silverfish->getNavigation()->isDone()
        && silverfish->mLevel->getGameRules()->getBoolean(1)
        && silverfish->mRandom->nextInt(10) == 0)
    {
        const Direction* direction = Direction::getRandomDirection(*silverfish->mRandom);
        BlockPos pos((int)silverfish->mX, (int)(silverfish->mY + 0.5), (int)silverfish->mZ);
        mDirection = direction->get3DDataValue();
        if (MonsterEggBlock::isCompatibleHostBlock(silverfish->mLevel->getBlockState(pos.relative(direction))))
        {
            mShouldMerge = true;
            return true;
        }
    }
    mShouldMerge = false;
    return RandomStrollGoal::canUse();
}

bool Silverfish::SilverfishMergeWithStoneGoal::canContinueToUse()
{
    return !mShouldMerge && RandomStrollGoal::canContinueToUse();
}

void Silverfish::SilverfishMergeWithStoneGoal::start()
{
    if (!mShouldMerge)
    {
        RandomStrollGoal::start();
        return;
    }
    Silverfish* silverfish = static_cast<Silverfish*>(mMob);
    const Direction* direction = Direction::from3DDataValue(mDirection);
    BlockPos pos((int)silverfish->mX, (int)(silverfish->mY + 0.5), (int)silverfish->mZ);
    const BlockState* host = silverfish->mLevel->getBlockState(pos.relative(direction));
    if (MonsterEggBlock::isCompatibleHostBlock(host))
    {
        MonsterEggBlock::Variant* variant = MonsterEggBlock::Variant::byHostBlock(host);
        silverfish->mLevel->setBlock(pos.relative(direction), Blocks::monster_egg->getBlockState(variant->getData()), 3, false);
        silverfish->remove();
        silverfish->spawnAnim();
    }
}

std::wstring Silverfish::SilverfishMergeWithStoneGoal::getName()
{
    return std::wstring(L"SilverfishMergeWithStone");
}

Silverfish::SilverfishWakeUpFriendsGoal::SilverfishWakeUpFriendsGoal(Silverfish* silverfish)
    : Goal(), mSilverfish(silverfish), mTicksLeft(0)
{
}

Silverfish::SilverfishWakeUpFriendsGoal::~SilverfishWakeUpFriendsGoal()
{
}

void Silverfish::SilverfishWakeUpFriendsGoal::notifyHurt()
{
    if (mTicksLeft == 0)
    {
        mTicksLeft = 20;
    }
}

bool Silverfish::SilverfishWakeUpFriendsGoal::canUse()
{
    return mTicksLeft == 0;
}

void Silverfish::SilverfishWakeUpFriendsGoal::tick()
{
    --mTicksLeft;
    if (mTicksLeft >= 0)
    {
        return;
    }
    BlockPos center(boost::static_pointer_cast<Entity>(mSilverfish->shared_from_this()));
    for (int x = -10; x <= 10; ++x)
    {
        for (int y = -5; y <= 5; ++y)
        {
            for (int z = -10; z <= 10; ++z)
            {
                BlockPos pos = center.offset(x, y, z);
                const BlockState* state = mSilverfish->mLevel->getBlockState(pos);
                if (MonsterEggBlock::isCompatibleHostBlock(state))
                {
                    if (!mSilverfish->mLevel->getGameRules()->getBoolean(1))
                    {
                        MonsterEggBlock::Variant* variant = MonsterEggBlock::Variant::byHostBlock(state);
                        mSilverfish->mLevel->setBlock(pos, Blocks::monster_egg->getBlockState(variant->getData()), 3, false);
                    }
                    else
                    {
                        mSilverfish->mLevel->setBlock(pos, Blocks::AIR->defaultBlockState(), 3, false);
                    }
                    if (mSilverfish->mRandom->nextBoolean())
                    {
                        return;
                    }
                }
            }
        }
    }
}

std::wstring Silverfish::SilverfishWakeUpFriendsGoal::getName()
{
    return std::wstring(L"SilverfishWakeUpFriends");
}
