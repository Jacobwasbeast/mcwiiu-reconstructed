#include "game/core/TrackingEmitter.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/arrayWithLength.hpp"

TrackingEmitter::TrackingEmitter(Level* level, boost::shared_ptr<Entity> entity,
                                 const ParticleType* particleType, int lifetime)
    : Particle(level, entity->mX, entity->mBoundingBox->mMinY + entity->mBbHeight * 0.5f,
               entity->mZ, entity->mXd, entity->mYd, entity->mZd),
      mEntity(entity), mParticleType(particleType)
{
    this->lifetime = lifetime;
    tick();
}

TrackingEmitter::~TrackingEmitter()
{
}

void TrackingEmitter::tick()
{
    for (int i = 0x10; i != 0; --i)
    {
        double x = static_cast<double>(random.nextFloat() * 2.0f - 1.0f);
        double y = static_cast<double>(random.nextFloat() * 2.0f - 1.0f);
        double z = static_cast<double>(random.nextFloat() * 2.0f - 1.0f);

        if (x * x + y * y + z * z <= 1.0)
        {
            level->addParticle(mParticleType,
                                x * static_cast<double>(mEntity->mBbWidth) * 0.25 + mEntity->mX,
                                y * static_cast<double>(mEntity->mBbHeight) * 0.25 +
                                    static_cast<double>(mEntity->mBbHeight) * 0.5 +
                                    mEntity->mBoundingBox->mMinY,
                                z * static_cast<double>(mEntity->mBbWidth) * 0.25 + mEntity->mZ,
                                x, y + 0.2, z, arrayWithLength<int>());
        }
    }

    if (++age >= lifetime)
    {
        remove();
    }
}

void TrackingEmitter::render(BufferBuilder*, const boost::shared_ptr<Entity>&,
                             float, float, float, float, float, float)
{
}

int TrackingEmitter::getParticleTexture()
{
    return 3;
}

uint64_t TrackingEmitter::GetType()
{
    return 0x0008022400000000LL;
}
