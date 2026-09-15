#include "game/core/Phantom.hpp"

#include <cmath>
#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/ai/CircleAroundAnchorGoal.hpp"
#include "game/entity/ai/FlyingPathNavigation.hpp"
#include "game/entity/ai/GlideMoveControl.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/SwoopAttackGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/InsomniaComponent.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

static const unsigned long long ENTITY_TYPE_PHANTOM = 0x5D540049ULL;

Phantom::Phantom(Level* level) : Monster(level)
{
    fjDerivedCtorCalls();
    if (mMoveControl != NULL)
        delete mMoveControl;
    mMoveControl = new GlideMoveControl(this, 0.1f, 0.2f);
    setSize(0.9f, 0.5f);
    setNoGravity(true);
}

Phantom::~Phantom() { }

void Phantom::create(Level* level) { new Phantom(level); }

void Phantom::tick()
{
    if (mLevel->mIsClientSide)
    {
        int phase = getId() * 3 + mTickCount;
        float first = Mth::cos((float)(phase - 176) * 0.13f);
        float second = Mth::cos((float)(phase - 175) * 0.13f);
        if (first > 0.0f && second <= 0.0f)
            mLevel->addParticle(PARTICLE_CLOUD, mX, mY, mZ, 0.0, 0.0, 0.0,
                                arrayWithLength<int>());
        float angle = mYRot * 0.01745329238474369f;
        float wingAngle = (first + 0.5f) * 0.13f * -50.0f * 0.01745329238474369f;
        float xOffset = Mth::cos(angle) * 1.3f * Mth::cos(wingAngle);
        float zOffset = Mth::sin(angle) * 1.3f * Mth::cos(wingAngle);
        float yOffset = Mth::sin(wingAngle);
        mLevel->addParticle(PARTICLE_CLOUD, mX + xOffset, mY + yOffset + 0.3,
                            mZ + zOffset, 0.0, 0.0, 0.0, arrayWithLength<int>());
        mLevel->addParticle(PARTICLE_CLOUD, mX - xOffset, mY + yOffset + 0.3,
                            mZ - zOffset, 0.0, 0.0, 0.0, arrayWithLength<int>());
    }
    Monster::tick();
}

bool Phantom::checkSpawnRules()
{
    bool hasSkyLight = mLevel->mDimension->isHasSkyLight() != 0;
    PlayerSelector selector(this);
    std::vector<boost::shared_ptr<Player> >* players =
        mLevel->getPlayers(0xC800000ULL, &selector);
    std::vector<boost::shared_ptr<Player> >::iterator it;
    for (it = players->begin(); it != players->end(); ++it)
    {
        boost::shared_ptr<Player> player = *it;
        BlockPos playerPos(player);
        InsomniaComponent* insomnia = player->getInsomniaComponent();
        if (insomnia != NULL && hasSkyLight && mLevel->canSeeSky(playerPos))
        {
            int timer = insomnia->getInsomniaTimerTicks();
            int until = insomnia->getTicksUntilInsomnia();
            if (timer < 1)
                timer = 1;
            if (until <= mRandom->nextInt(timer))
            {
                BlockPos offset(mRandom->nextInt(21) - 10,
                                mRandom->nextInt(15) + 20,
                                mRandom->nextInt(21) - 10);
                BlockPos spawnPos(playerPos + offset);
                setPos((double)spawnPos.x, (double)spawnPos.y, (double)spawnPos.z);
                bool result = Monster::checkSpawnRules();
                delete players;
                return result;
            }
        }
    }
    delete players;
    return false;
}

void Phantom::aiStep()
{
    Monster::aiStep();
    if (!mLevel->mIsClientSide && mLevel->isDay() && getBrightness() > 0.5f &&
        mRandom->nextFloat() * 30.0f < (getBrightness() - 0.4f) * 2.0f)
    {
        BlockPos pos(this);
        if (mLevel->canSeeSky(pos))
            setSecondsOnFire(8);
    }
}

EMobType Phantom::getMobType() { return MOB_TYPE_UNDEAD; }
bool Phantom::canFly() { return true; }

void Phantom::registerGoals()
{
    Goal::FloatRange distance = { 6.0f, 20.0f };
    mGoalSelector.addGoal(1, new SwoopAttackGoal(this, 1.0f, distance), true);
    Goal::FloatRange anchor = { 12.0f, -4.0f };
    Goal::FloatRange height = { 18.0f, 5.0f };
    Goal::FloatRange vertical = { 1.8f, 30.0f };
    mGoalSelector.addGoal(2, new CircleAroundAnchorGoal(this, 1.0f, anchor, 250,
                                                         height, vertical, 350, 1.0f), true);
    mTargetSelector.addGoal(1, new NearestAttackableTargetGoal(this, typeid(Player), false,
                                                                false, false), true);
}

void Phantom::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(6.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(1.8);
    getAttribute(SharedMonsterAttributes::FLYING_SPEED)->setBaseValue(0.4);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(64.0);
}

PathNavigation* Phantom::createNavigation(Level* level)
{
    FlyingPathNavigation* navigation = new FlyingPathNavigation(this, level);
    navigation->setCanOpenDoors(false);
    return navigation;
}

const SoundEvent* Phantom::getAmbientSound() { return SoundEvents::PHANTOM_AMBIENT; }
const SoundEvent* Phantom::getHurtSound(const DamageSource*) { return SoundEvents::PHANTOM_HURT; }
const SoundEvent* Phantom::getDeathSound() { return SoundEvents::PHANTOM_DEATH; }
ResourceLocation* Phantom::getDefaultLootTable() { return &BuiltInLootTables::PHANTOM; }
unsigned long long Phantom::GetType() { return ENTITY_TYPE_PHANTOM; }

bool Phantom::PlayerSelector::apply(boost::shared_ptr<Player> player) const
{
    return mPhantom->distanceToSqr(player) < 10000.0;
}

Phantom::PlayerSelector::~PlayerSelector() { }
