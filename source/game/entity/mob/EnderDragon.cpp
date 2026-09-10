#include "game/entity/mob/EnderDragon.hpp"
#include "game/world/generation/Player.hpp"

#include <cmath>

#include "game/core/BinaryHeap.hpp"
#include "game/core/Node.hpp"
#include "game/core/Path.hpp"
#include "game/entity/mob/MultiEntityMobPart.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/entity/mob/EndDragonFight.hpp"

EnderDragon::EnderDragon(Level* level)
    : Mob(level), mOpenSet(NULL), mPath(NULL), mDragonFight(NULL), mRandom(new Random()),
      mDragonPhase(0), mTicksSinceDeath(0), mTarget(), mNearbyCrystal(0),
      mDragonKilled(false), mPreviouslyKilled(false), mHasTarget(false), mWasHurt(false),
      mHealth(200), mTargetX(0.0), mTargetY(0.0), mTargetZ(0.0),
      mFlapTime(0.0f), mOldFlapTime(0.0f), mDeathTime(0), mBeamTarget()
{
    _init();
}

EnderDragon::~EnderDragon()
{
    delete mOpenSet;
    delete mPath;
    delete mRandom;
}

std::wstring EnderDragon::getDisplayName()
{
    return L"entity.EnderDragon.name";
}

float EnderDragon::getMaxHealth() { return 200.0f; }
float EnderDragon::getHealth() { return LivingEntity::getHealth(); }
unsigned int EnderDragon::getDimension() { return 1; }
Level* EnderDragon::getLevel() { return Entity::getCommandSenderWorld(); }

unsigned int EnderDragon::getSynchedAction()
{
    return static_cast<unsigned int>(mDragonPhase);
}

int EnderDragon::setSynchedAction(EEnderdragonAction action, bool force)
{
    if (!force && action == static_cast<EEnderdragonAction>(mDragonPhase))
        return 0;
    mDragonPhase = static_cast<int>(action);
    return 1;
}

unsigned int EnderDragon::findClosestNode(double, double, double) { return 0; }
unsigned int EnderDragon::findClosestNode() { return 0; }
double EnderDragon::getHeadYRotDiff(float) { return 0.0; }
Vec3* EnderDragon::getHeadLookVector(float) { return NULL; }

Path* EnderDragon::reconstruct_path(Node*, Node*) { return NULL; }
Path* EnderDragon::findPath(int, int, Node*) { return NULL; }

void EnderDragon::navigateToNextPathNode()
{
    if (mPath != NULL && !mPath->isDone())
        mPath->next();
}

void EnderDragon::strafeAttackTarget() {}
unsigned int EnderDragon::findNewTarget()
{
    if (mLevel == NULL)
        return 0;

    boost::shared_ptr<Player> target = mLevel->getNearestPlayer(mX, mY, mZ, 64.0, true, -1.0);
    mTarget = boost::static_pointer_cast<Entity>(target);
    mHasTarget = mTarget != 0;
    if (mHasTarget)
        setSynchedAction(ACTION_1, false);
    else
        setSynchedAction(ACTION_NONE, false);
    return mHasTarget ? 1U : 0U;
}
bool EnderDragon::hurt(boost::shared_ptr<MultiEntityMobPart>, const DamageSource*, float) { return false; }
void EnderDragon::GetInteractTooltip(boost::shared_ptr<MultiEntityMobPart>, const EntityTooltipDataHolder&) {}
void EnderDragon::AddParts()
{
    boost::shared_ptr<MultiEntityMob> parent =
        boost::dynamic_pointer_cast<MultiEntityMob>(shared_from_this());
    const std::wstring headName = L"head";
    const std::wstring neckName = L"neck";
    const std::wstring bodyName = L"body";
    const std::wstring tailName = L"tail";
    boost::shared_ptr<MultiEntityMobPart> head(new MultiEntityMobPart(parent, headName, 6.0f, 6.0f));
    boost::shared_ptr<MultiEntityMobPart> neck(new MultiEntityMobPart(parent, neckName, 6.0f, 6.0f));
    boost::shared_ptr<MultiEntityMobPart> body(new MultiEntityMobPart(parent, bodyName, 8.0f, 8.0f));
    mParts.push_back(boost::static_pointer_cast<Entity>(head));
    mParts.push_back(boost::static_pointer_cast<Entity>(neck));
    mParts.push_back(boost::static_pointer_cast<Entity>(body));
    for (int i = 0; i < 5; ++i)
    {
        boost::shared_ptr<MultiEntityMobPart> tail(new MultiEntityMobPart(parent, tailName, 4.0f, 4.0f));
        mParts.push_back(boost::static_pointer_cast<Entity>(tail));
    }
}
void EnderDragon::onCrystalDestroyed(boost::shared_ptr<EnderCrystal> crystal, const BlockPos& position, const DamageSource* source)
{
    boost::shared_ptr<Player> player;
    if (source != NULL)
    {
        boost::shared_ptr<Entity> attacker = source->getEntity();
        if (Entity::instanceof(attacker, ENTITY_TYPE_PLAYER))
            player = boost::dynamic_pointer_cast<Player>(attacker);
    }
    if (player == 0 && mLevel != NULL)
        player = mLevel->getNearestAttackablePlayer(position, 64.0, 16.0);

    if (crystal != 0 && crystal == mBeamTarget)
    {
        mBeamTarget.reset();
        if (player != 0)
        {
            boost::shared_ptr<LivingEntity> sourceEntity =
                boost::static_pointer_cast<LivingEntity>(player);
            boost::shared_ptr<EntityDamageSource> explosion =
                DamageSource::CreateExplosion(sourceEntity);
            hurt(explosion.get(), 10.0f);
        }
    }

    if (mDragonFight != NULL)
    {
        if (mDragonFight->getCrystalsAlive() == 0)
        {
            if (setSynchedAction(ACTION_2, false) == 0)
            {
                if (mTarget != 0 && Entity::instanceof(mTarget, ENTITY_TYPE_PLAYER))
                {
                    mTarget = boost::static_pointer_cast<Entity>(
                        boost::dynamic_pointer_cast<Player>(mTarget));
                    setSynchedAction(ACTION_1, false);
                    strafeAttackTarget();
                }
                while (mPath != NULL && !mPath->isDone())
                    mPath->next();
            }
        }
    }
}

void EnderDragon::_init()
{
    int i;
    int j;
    int k;
    for (i = 0; i < 64; ++i)
        for (j = 0; j < 3; ++j)
            mPosHistory[i][j] = 0.0;
    for (k = 0; k < 8; ++k)
        mDragonPhase = 0;
    mDeathTime = 0;
    mBeamTarget.reset();
}

void EnderDragon::addDataWalkers(DataFixerUpper*) {}
void EnderDragon::staticCtor() {}
void EnderDragon::registerAttributes() { Mob::registerAttributes(); }
void EnderDragon::defineSynchedData() { Mob::defineSynchedData(); }
bool EnderDragon::canRide(boost::shared_ptr<Entity>) { return false; }

void EnderDragon::getLatencyPos(arrayWithLength<double> positions, int index, float)
{
    if (index < 0 || index >= 64 || positions.data == NULL)
        return;
    positions.data[0] = mPosHistory[index][0];
    positions.data[1] = mPosHistory[index][1];
    positions.data[2] = mPosHistory[index][2];
}

double EnderDragon::getTilt(float partialTicks)
{
    return static_cast<double>(partialTicks) * 0.0;
}

double EnderDragon::getHeadYOffset(float partialTicks)
{
    return getHeadPartYOffset(0, arrayWithLength<double>(), arrayWithLength<double>()) + partialTicks * 0.0;
}

double EnderDragon::rotWrap(double angle)
{
    while (angle >= 180.0) angle -= 360.0;
    while (angle < -180.0) angle += 360.0;
    return angle;
}

bool EnderDragon::checkCrystals() { return mNearbyCrystal != 0; }
void EnderDragon::knockBack(std::vector<boost::shared_ptr<Entity> >*) {}
bool EnderDragon::hurt(std::vector<boost::shared_ptr<Entity> >*) { return false; }
bool EnderDragon::checkWalls(AABB*) { return false; }
void EnderDragon::aiStep() { Mob::aiStep(); }
ResourceLocation* EnderDragon::getDefaultLootTable() { return NULL; }
bool EnderDragon::hurt(const DamageSource* source, float amount) { return LivingEntity::hurt(source, amount); }
void EnderDragon::reallyHurt(const DamageSource* source, float amount) { LivingEntity::actuallyHurt(source, amount); }
void EnderDragon::kill() { LivingEntity::kill(); }
void EnderDragon::dropExperience(int experience) { mTicksSinceDeath += experience > 0 ? 0 : 0; }
void EnderDragon::tickDeath() { ++mDeathTime; }
void EnderDragon::checkDespawn() {}
std::vector<boost::shared_ptr<Entity> >* EnderDragon::getSubEntities() { return &mParts; }
bool EnderDragon::isPickable() { return false; }
const SoundEvent* EnderDragon::getAmbientSound() { return NULL; }
const SoundEvent* EnderDragon::getHurtSound(const DamageSource*) { return NULL; }
float EnderDragon::getSoundVolume() { return 5.0f; }

void EnderDragon::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    if (tag != NULL)
        tag->putInt(L"DragonState", getSynchedAction());
}

void EnderDragon::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    if (tag != NULL && tag->contains(L"DragonState"))
        setSynchedAction(static_cast<EEnderdragonAction>(tag->getInt(L"DragonState")), true);
}

double EnderDragon::getHeadPartYOffset(int part, arrayWithLength<double> first, arrayWithLength<double> second)
{
    if (part == 6 || first.data == NULL || second.data == NULL)
        return static_cast<double>(part);
    return second.data[1] - first.data[1];
}

double EnderDragon::getHeadPartYRotDiff(int, arrayWithLength<double> first, arrayWithLength<double> second)
{
    if (first.data == NULL || second.data == NULL)
        return 0.0;
    return rotWrap(second.data[0] - first.data[0]);
}

unsigned int EnderDragon::GetInteractTooltip(const EntityTooltipDataHolder&) { return 0xffffffffU; }
unsigned long long EnderDragon::GetType() { return 0x000000000D000005ULL; }

boost::shared_ptr<EnderDragon> EnderDragon::create(Level* level)
{
    return boost::shared_ptr<EnderDragon>(new EnderDragon(level));
}
