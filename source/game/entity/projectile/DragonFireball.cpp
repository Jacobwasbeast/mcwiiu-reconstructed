#include "game/entity/projectile/DragonFireball.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/client/render/ParticleType.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/HitResult.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/AreaEffectCloud.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/data/DataFixerUpper.hpp"

DragonFireball::DragonFireball(Level* level)
    : Fireball(level)
{
    fjDerivedCtorCalls();
    setSize(1.0f, 0.4f);
}

DragonFireball::DragonFireball(Level* level, boost::shared_ptr<LivingEntity> owner,
                               double xd, double yd, double zd)
    : Fireball(level, owner, xd, yd, zd)
{
    fjDerivedCtorCalls();
    setSize(1.0f, 0.4f);
}

DragonFireball::DragonFireball(Level* level, double x, double y, double z,
                               double xd, double yd, double zd)
    : Fireball(level, x, y, z, xd, yd, zd)
{
    fjDerivedCtorCalls();
    setSize(1.0f, 0.4f);
}

DragonFireball::~DragonFireball() = default;

void DragonFireball::addDataWalkers(DataFixerUpper* fixer)
{
    Fireball::addDataWalkers(fixer, EntityType::DRAGON_FIREBALL);
}

void DragonFireball::onHit(const HitResult* result)
{
    if ((result->mEntity.get() == NULL
         || result->mEntity.owner_before(mOwner) != mOwner.owner_before(result->mEntity))
        && !mLevel->mIsClientSide)
    {
        AABB* box = mBoundingBox->grow(2.0, 2.0, 2.0);
        std::vector<boost::shared_ptr<Entity> >* entities =
            mLevel->getEntitiesOfClass(typeid(Entity), box);

        boost::shared_ptr<AreaEffectCloud> cloud(
            new AreaEffectCloud(mLevel, mX, mY, mZ));
        cloud->setOwner(mOwner);
        cloud->setParticle(PARTICLE_DRAGON_BREATH);
        cloud->setRadius(3.0f);
        cloud->setParticleSpawnRateOnTick(10);
        cloud->setDuration(200);
        cloud->setRadiusPerTick((7.0f - cloud->getRadius()) /
                                static_cast<float>(cloud->getDuration() - 1));
        cloud->addEffect(new MobEffectInstance(MobEffects::HARM, 1, 1));
        mLevel->addEntity(boost::static_pointer_cast<Entity>(cloud));

        if (entities != NULL)
        {
            for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
                 it != entities->end(); ++it)
            {
                boost::shared_ptr<Entity> entity = *it;
                if (entity && distanceToSqr(entity) < 16.0)
                {
                    boost::shared_ptr<DamageSource> source =
                        DamageSource::CreateIndirectMagic(shared_from_this(), entity, NULL);
                    entity->hurt(source.get(), 6.0f);
                }
            }
            delete entities;
        }

        mLevel->levelEvent(LevelEvent::PARTICLES_DRAGON_FIREBALL_SPLASH, BlockPos(mX, mY, mZ), 0);
        remove();
    }
}

bool DragonFireball::isPickable()
{
    return false;
}

bool DragonFireball::hurt(const DamageSource*, float)
{
    return false;
}

const ParticleType* DragonFireball::getTrailParticleType()
{
    return PARTICLE_DRAGON_BREATH;
}

bool DragonFireball::shouldBurn()
{
    return false;
}

unsigned long long DragonFireball::GetType()
{
    return EntityType::DRAGON_FIREBALL;
}

void DragonFireball::create(Level* level)
{
    new DragonFireball(level);
}
