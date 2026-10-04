#include "game/client/render/SnowShovelParticle.hpp"
#include "game/core/EntityType.hpp"

#include <math.h>

#include "game/core/Mth.hpp"

SnowShovelParticle::SnowShovelParticle(Level* level, double x, double y, double z,
                                       double xa, double ya, double za)
    : Particle(level, x, y, z, xa, ya, za)
{
    init(level, x, y, z, xa, ya, za, 1.0f);
}

void SnowShovelParticle::render(BufferBuilder* buffer,
                                const boost::shared_ptr<Entity>& camera,
                                float partialTicks,
                                float xa,
                                float ya,
                                float za,
                                float xa2,
                                float za2)
{
    const float progress = Mth::clamp((partialTicks + (float)age) /
                                      (float)lifetime * 32.0f,
                                      0.0f,
                                      1.0f);
    Particle::size = size * progress;
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void SnowShovelParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    int particleLifetime = lifetime;
    if (particleLifetime <= age++)
    {
        remove();
        particleLifetime = lifetime;
    }

    setMiscTex(7 - (age << 3) / particleLifetime);
    yd -= 0.03;
    move(xd, yd, zd);
    double xdNew = xd * 0.99;
    yd *= 0.99;
    double zdNew = zd * 0.99;
    xd = xdNew;
    zd = zdNew;
    if (onGround)
    {
        xd = xdNew * 0.7;
        zd = zdNew * 0.7;
    }
}

void SnowShovelParticle::init(Level* level, double x, double y, double z,
                              double xa, double ya, double za, float scale)
{
    xd = xd * 0.1 + xa;
    yd = yd * 0.1 + ya;
    zd = zd * 0.1 + za;

    const float color = 1.0f - random.nextFloat() * 0.3f;
    rCol = color;
    gCol = color;
    bCol = color;
    const float particleSize = Particle::size * 0.75f * scale;
    Particle::size = particleSize;
    size = particleSize;
    lifetime = (int)((float)(int)floor(8.0 / (random.nextDouble() * 0.8 + 0.2)) * scale);
}

uint64_t SnowShovelParticle::GetType()
{
    return ParticleKind::SNOW_SHOVEL;
}

Particle* SnowShovelParticle::Provider::createParticle(ePARTICLE_TYPE type,
                                                        Level* level,
                                                        double x,
                                                        double y,
                                                        double z,
                                                        double xa,
                                                        double ya,
                                                        double za,
                                                        arrayWithLength<int> params)
{
    return new SnowShovelParticle(level, x, y, z, xa, ya, za);
}
