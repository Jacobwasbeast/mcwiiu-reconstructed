#include "game/client/render/WakeParticle.hpp"

#include "game/util/Random.hpp"

WakeParticle::WakeParticle(Level* level, double x, double y, double z,
                           double xa, double ya, double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    xd *= 0.3;
    yd = random.nextFloat() * 0.2 + 0.1;
    zd *= 0.3;
    rCol = 1.0f;
    gCol = 1.0f;
    bCol = 1.0f;
    setMiscTex(19);
    setSize(0.01f, 0.01f);
    lifetime = (int)(8.0 / (random.nextDouble() * 0.8 + 0.2));
    xd = xa;
    yd = ya;
    zd = za;
}

WakeParticle::~WakeParticle()
{
}

void WakeParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    y -= gravity;
    move(xd, yd, zd);
    xd *= 0.98;
    yd *= 0.98;
    zd *= 0.98;
    setLocationFromBoundingbox();
    setMiscTex(((60 - age) & ~3) + 19);
    if (age++ >= lifetime)
    {
        remove();
    }
}

uint64_t WakeParticle::GetType()
{
    return 0x0000000000080213ULL;
}

Particle* WakeParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                  double x, double y, double z,
                                                  double xa, double ya, double za,
                                                  arrayWithLength<int> params)
{
    return new WakeParticle(level, x, y, z, xa, ya, za);
}
