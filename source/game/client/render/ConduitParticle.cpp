#include "game/client/render/ConduitParticle.hpp"
#include "game/world/Level.hpp"

ConduitParticle::ConduitParticle(Level* level,
                                 double x,
                                 double y,
                                 double z,
                                 double xa,
                                 double ya,
                                 double za,
                                 arrayWithLength<int> params)
    : Particle(level, x, y, z, xa, ya, za),
      m_conduitParticleValue(0.0f),
      m_startX(x),
      m_startY(y),
      m_startZ(z)
{
    xd = xa;
    yd = ya;
    zd = za;
    setPos(x + xa, y + ya, z + za);

    Random* levelRandom = level->mRandom;
    float colour = levelRandom->nextFloat(0.4f, 1.0f);
    float greenBlue = levelRandom->nextFloat(0.2f, 0.7f);
    size = greenBlue;
    m_conduitParticleValue = greenBlue;
    rCol = colour * 0.9f;
    gCol = colour * 0.9f;
    bCol = colour;
    float lifetimeScale = levelRandom->nextFloat(0.5f, 2.5f);
    float lifetimeValue = lifetimeScale * 20.0f;
    hasPhysics = false;
    lifetime = (int)lifetimeValue;

    int texture = levelRandom->nextInt(4);
    int variant = levelRandom->nextInt(0, 3);
    uvs = TextureUVCoordinateSet::fromOldSystem(texture * 3 + 0xd0 + variant);
}

void ConduitParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    double inverseProgress = 1.0 - (double)age / (double)lifetime;
    double progress = 1.0 - inverseProgress;
    x = xd * inverseProgress + m_startX;
    y = -(progress * progress * progress * 1.2 - (yd * inverseProgress + m_startY));
    z = zd * inverseProgress + m_startZ;
    setPos(x, y, z);

    if (age++ >= lifetime)
        remove();
}

uint64_t ConduitParticle::GetType()
{
    return PARTICLE_TYPE_CONDUIT;
}

Particle* ConduitParticle::Provider::createParticle(ePARTICLE_TYPE type,
                                                     Level* level,
                                                     double x,
                                                     double y,
                                                     double z,
                                                     double xa,
                                                     double ya,
                                                     double za,
                                                     arrayWithLength<int> params)
{
    return new ConduitParticle(level, x, y, z, xa, ya, za, params);
}
