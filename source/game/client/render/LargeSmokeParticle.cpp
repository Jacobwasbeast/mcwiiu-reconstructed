#include "game/client/render/LargeSmokeParticle.hpp"

LargeSmokeParticle::LargeSmokeParticle(Level* level, double x, double y, double z,
                                       double xa, double ya, double za)
    : SmokeParticle(level, x, y, z, xa, ya, za, 2.5f)
{
}

LargeSmokeParticle::~LargeSmokeParticle()
{
}

uint64_t LargeSmokeParticle::GetType()
{
    return 0x0000000000080226ULL;
}

Particle* LargeSmokeParticle::Provider::createParticle(ePARTICLE_TYPE type,
                                                        Level* level,
                                                        double x, double y, double z,
                                                        double xa, double ya, double za,
                                                        arrayWithLength<int> params)
{
    return new LargeSmokeParticle(level, x, y, z, xa, ya, za);
}
