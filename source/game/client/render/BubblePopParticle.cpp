#include "game/client/render/BubblePopParticle.hpp"

#include "game/client/render/TextureUVCoordinateSet.hpp"

BubblePopParticle::BubblePopParticle(Level* level,
                                     double x,
                                     double y,
                                     double z,
                                     double xa,
                                     double ya,
                                     double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    xd = xa;
    yd = ya;
    zd = za;
    rCol = 1.0f;
    gCol = 1.0f;
    bCol = 1.0f;
    setMiscTex(256);
    gravity = 0.008f;
    lifetime = 4;
}

BubblePopParticle::~BubblePopParticle()
{
}

void BubblePopParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    y -= gravity;
    setPos(x, y, z);

    if (age++ >= lifetime)
    {
        remove();
        return;
    }

    int frame = age * 5 / lifetime;
    if (frame < 5)
    {
        int frameX = frame & 0x0f;
        int frameY = frame >> 4;
        uvs = TextureUVCoordinateSet(frameX * 0.0625f + 0.375f,
                                     frameY * 0.0625f - 0.625f,
                                     (frameX + 2) * 0.0625f + 0.375f,
                                     (frameY + 2) * 0.0625f - 0.625f,
                                     1,
                                     1,
                                     1.0f);
    }
}

long long BubblePopParticle::GetType()
{
    return PARTICLE_TYPE_BUBBLE_POP;
}

Particle* BubblePopParticle::Provider::createParticle(ePARTICLE_TYPE type,
                                                       Level* level,
                                                       double x,
                                                       double y,
                                                       double z,
                                                       double xa,
                                                       double ya,
                                                       double za,
                                                       arrayWithLength<int> params)
{
    return new BubblePopParticle(level, x, y, z, xa, ya, za);
}
