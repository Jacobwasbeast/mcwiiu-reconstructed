#include "game/client/render/EndRodParticle.hpp"

EndRodParticle::EndRodParticle(Level* level, double x, double y, double z,
                               double xa, double ya, double za)
    : SimpleAnimatedParticle(level, x, y, z, 176, 8, 0.00048828125f)
{
    size *= 0.75f;
    xd = xa;
    yd = ya;
    zd = za;
    lifetime = random.nextInt(12) + 60;
    setFadeColor(0xf2dec9);
}

EndRodParticle::~EndRodParticle()
{
}

void EndRodParticle::move(double xa, double ya, double za)
{
    AABB* box = getBoundingBox();
    setBoundingBox(box->move(xa, ya, za));
    setLocationFromBoundingbox();
}

uint64_t EndRodParticle::GetType()
{
    return 0x0008022900000000ULL;
}

Particle* EndRodParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                    double x, double y, double z,
                                                    double xa, double ya, double za,
                                                    arrayWithLength<int> params)
{
    return new EndRodParticle(level, x, y, z, xa, ya, za);
}
