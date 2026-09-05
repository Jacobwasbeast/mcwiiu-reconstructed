#include "game/client/render/TotemParticle.hpp"

TotemParticle::TotemParticle(Level* level, double x, double y, double z,
                             double xa, double ya, double za)
    : SimpleAnimatedParticle(level, x, y, z, 176, 8, -0.05f)
{
    size *= 0.75f;
    xd = xa;
    yd = ya;
    zd = za;
    lifetime = random.nextInt(12) + 60;

    if (random.nextInt(4) == 0)
    {
        Particle::setColor(random.nextFloat() * 0.2f + 0.6f,
                           random.nextFloat() * 0.3f + 0.6f,
                           random.nextFloat() * 0.2f);
        setBaseAirFriction(0.3f);
    }
    else
    {
        Particle::setColor(random.nextFloat() * 0.2f + 0.1f,
                           random.nextFloat() * 0.3f + 0.4f,
                           random.nextFloat() * 0.2f);
        setBaseAirFriction(0.3f);
    }
}

TotemParticle::~TotemParticle()
{
}

long long TotemParticle::GetType()
{
    return 0x0008022b00000000ULL;
}

Particle* TotemParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                  double x, double y, double z,
                                                  double xa, double ya, double za,
                                                  arrayWithLength<int> params)
{
    return new TotemParticle(level, x, y, z, xa, ya, za);
}
