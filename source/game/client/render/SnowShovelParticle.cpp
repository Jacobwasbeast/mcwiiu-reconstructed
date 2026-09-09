#include "game/client/render/SnowShovelParticle.hpp"

#include <math.h>

#include "game/core/Mth.hpp"

SnowShovelParticle::SnowShovelParticle(Level* level, double x, double y, double z,
                                       double xa, double ya, double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    init(level, x, y, z, xa, ya, za, 1.0f);
}

SnowShovelParticle::~SnowShovelParticle()
{
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
    const float progress = Mth::clamp((float)((double)age + partialTicks) /
                                      (float)lifetime * 32.0f,
                                      0.0f,
                                      1.0f);
    alpha = size * progress;
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void SnowShovelParticle::tick()
{
    const int oldAge = age;
    const int currentLifetime = lifetime;
    xo = x;
    yo = y;
    zo = z;
    age = oldAge + 1;
    if (currentLifetime <= oldAge)
    {
        remove();
    }

    setMiscTex(7 - (age << 3) / currentLifetime);
    yd -= 0.03;
    move(xd, yd, zd);
    xd *= 0.99;
    yd *= 0.99;
    zd *= 0.99;
    if (onGround)
    {
        xd *= 0.7;
        zd *= 0.7;
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
    alpha = alpha * 0.75f * scale;
    size = alpha;
    lifetime = (int)((double)(int)floor(8.0 / (random.nextDouble() * 0.8 + 0.2)) * scale);
}

uint64_t SnowShovelParticle::GetType()
{
    return 0x000000000008020DULL;
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
