#include "game/client/render/ConduitParticle.hpp"

ConduitParticle::ConduitParticle(Level* level,
                                 double x,
                                 double y,
                                 double z,
                                 double xa,
                                 double ya,
                                 double za,
                                 arrayWithLength<int> params)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0),
      m_startX(x),
      m_startY(y),
      m_startZ(z)
{
    xd = xa;
    yd = ya;
    zd = za;
    setPos(x + xa, y + ya, z + za);

    float colour = random.nextFloat(0.4f, 1.0f);
    float greenBlue = random.nextFloat(0.2f, 0.7f);
    rCol = colour * 0.9f;
    gCol = colour * 0.9f;
    bCol = colour;
    alpha = greenBlue;
    size = greenBlue;
    lifetime = (int)(random.nextFloat(0.5f, 2.5f) * 20.0f);

    int texture = random.nextInt(4);
    int variant = random.nextInt(0, 3);
    uvs = TextureUVCoordinateSet::fromOldSystem(texture * 3 + 0xd0 + variant);
}

ConduitParticle::~ConduitParticle()
{
}

void ConduitParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    double progress = (double)age / (double)lifetime;
    double inverseProgress = 1.0 - progress;
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
