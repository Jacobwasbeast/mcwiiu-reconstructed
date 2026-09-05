#include "game/client/render/SpitParticle.hpp"

SpitParticle::SpitParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : ExplodeParticle(level, x, y, z, xa, ya, za)
{
    gravity = 0.5f;
}

SpitParticle::~SpitParticle()
{
}

uint64_t SpitParticle::GetType()
{
    return 0x0008022c00000000ULL;
}

void SpitParticle::tick()
{
    ExplodeParticle::tick();
    y -= 0.04 * (double)gravity + 0.004;
}

Particle* SpitParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y,
                                                  double z, double xa, double ya, double za,
                                                  arrayWithLength<int> params)
{
    return new SpitParticle(level, x, y, z, xa, ya, za);
}
