#include "game/client/render/HugeExplosionSeedParticle.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"

HugeExplosionSeedParticle::HugeExplosionSeedParticle(Level* level, double x, double y, double z,
                                                     double xa, double ya, double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    lifetime = 8;
}

HugeExplosionSeedParticle::~HugeExplosionSeedParticle()
{
}

void HugeExplosionSeedParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera,
                                       float partialTicks, float xa, float ya, float za,
                                       float xa2, float za2)
{
}

void HugeExplosionSeedParticle::tick()
{
    Minecraft::GetInstance()->mLevel = (MultiPlayerLevel*)level;

    for (int i = 0; i < 3; ++i)
    {
        level->addParticle(PARTICLE_LARGE_EXPLODE,
                           x + (random.nextDouble() - random.nextDouble()) * 4.0,
                           y + (random.nextDouble() - random.nextDouble()) * 4.0,
                           z + (random.nextDouble() - random.nextDouble()) * 4.0,
                           (double)age / (double)lifetime, 0.0, 0.0,
                           arrayWithLength<int>());
    }

    Minecraft::GetInstance()->mLevel = NULL;
    ++age;
    if (age == lifetime)
    {
        remove();
    }
}

int HugeExplosionSeedParticle::getParticleTexture()
{
    return 1;
}

uint64_t HugeExplosionSeedParticle::GetType()
{
    return 0x0000000000080217ULL;
}

Particle* HugeExplosionSeedParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                               double x, double y, double z,
                                                               double xa, double ya, double za,
                                                               arrayWithLength<int> params)
{
    return new HugeExplosionSeedParticle(level, x, y, z, xa, ya, za);
}
