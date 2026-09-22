#include "game/core/WitherSkull.hpp"
#include <algorithm>
#include "game/core/HitResult.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/world/block/Block.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/GameRules.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

const unsigned long long WitherSkull::ENTITY_TYPE = 0x8A400002ULL;
const EntityDataAccessor<bool>* WitherSkull::DATA_DANGEROUS = NULL;

WitherSkull::WitherSkull(Level* level) : Fireball(level)
{
    fjDerivedCtorCalls();
    setSize(0.3125f, 0.3125f);
}
WitherSkull::WitherSkull(Level* level, boost::shared_ptr<LivingEntity> owner, double x, double y, double z)
    : Fireball(level, owner, x, y, z)
{
    fjDerivedCtorCalls();
    setSize(0.3125f, 0.3125f);
}
WitherSkull::WitherSkull(Level* level, double x, double y, double z, double xd, double yd, double zd)
    : Fireball(level, x, y, z, xd, yd, zd)
{
    fjDerivedCtorCalls();
    setSize(0.3125f, 0.3125f);
}
WitherSkull::~WitherSkull() = default;
void WitherSkull::create(Level* level)
{
    new WitherSkull(level);
}
void WitherSkull::staticCtor()
{
    DATA_DANGEROUS = SynchedEntityData::defineId<bool>(ENTITY_TYPE, 0x8A400000ULL,
                                                       EntityDataSerializers::BOOLEAN);
}
void WitherSkull::addDataWalkers(DataFixerUpper* fixer) { Fireball::addDataWalkers(fixer, ENTITY_TYPE); }
bool WitherSkull::isOnFire() { return false; }
float WitherSkull::getBlockExplosionResistance(Explosion* explosion, Level* level, const BlockPos& pos)
{
    const float resistance = Entity::getBlockExplosionResistance(explosion, level, pos);
    Block* block = level->getBlockState(pos)->getBlock();
    if (isDangerous() && WitherBoss::canDestroy(block) && resistance > 0.8f)
        return 0.8f;
    return resistance;
}
bool WitherSkull::isPickable() { return isDangerous(); }
bool WitherSkull::isDangerous() const { return mEntityData->get(DATA_DANGEROUS); }
void WitherSkull::setDangerous(bool dangerous) { mEntityData->set(DATA_DANGEROUS, dangerous, true); }
double WitherSkull::getInertia()
{
    const double dangerousInertia = 0.73;
    if (isDangerous())
        return dangerousInertia;
    return Fireball::getInertia();
}
void WitherSkull::onHit(const HitResult* result)
{
    if (mLevel->mIsClientSide)
        return;
    if (result->mEntity)
    {
        if (mOwner)
        {
            EntityDamageSource* source = DamageSource::CreateMobAttack(mOwner);
            if (result->mEntity->hurt(source, 8.0f) && result->mEntity->isAlive())
                doEnchantDamageEffects(mOwner, result->mEntity);
            delete source;
        }
        else
        {
            result->mEntity->hurt(DamageSource::MAGIC, 5.0f);
        }
        if ((result->mEntity->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
        {
            const Difficulty* difficulty = mLevel->getDifficulty();
            if (difficulty == Difficulty::NORMAL || difficulty == Difficulty::HARD)
            {
                const int duration = difficulty == Difficulty::HARD ? 40 : 10;
                boost::shared_ptr<LivingEntity> target = boost::dynamic_pointer_cast<LivingEntity>(result->mEntity);
                target->addEffect(new MobEffectInstance(MobEffects::WITHER, duration * 20, 1),
                                  boost::shared_ptr<Entity>());
            }
        }
    }
    const bool mobGriefing = mLevel->getGameRules()->getBoolean(1);
    mLevel->explode(shared_from_this(), mX, mY, mZ, 1.0f, false, mobGriefing,
                    false, false, false, -1.0f);
    remove();
}
void WitherSkull::defineSynchedData()
{
    Fireball::defineSynchedData();
    mEntityData->createDateItem(DATA_DANGEROUS, false);
}
bool WitherSkull::shouldBurn() { return false; }
std::vector<unsigned long long>* WitherSkull::getEntityTypesToIgnoreCollisionsWith()
{
    std::vector<unsigned long long>* types = new std::vector<unsigned long long>();
    types->push_back(GetType());
    return types;
}
unsigned long long WitherSkull::GetType() { return ENTITY_TYPE; }
