#include "game/client/render/HeartParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Mth.hpp"

HeartParticle::HeartParticle(Level* level, double x, double y, double z,
                             double xa, double ya, double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    init(level, x, y, z, xa, ya, za, 2.0f);
}

HeartParticle::~HeartParticle()
{
    return;
}

void HeartParticle::render(BufferBuilder* buffer,
                           const boost::shared_ptr<Entity>& camera,
                           float partialTicks,
                           float xa,
                           float ya,
                           float za,
                           float xa2,
                           float za2)
{
    float progress = Mth::clamp(((float)age + partialTicks) / (float)lifetime * 32.0f,
                                0.0f, 1.0f);
    size = scale * progress;
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void HeartParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime)
    {
        remove();
    }

    move(xd, yd, zd);

    if (x == xo)
    {
        xd *= 1.1;
        zd *= 1.1;
    }

    xd *= 0.8600000143051147;
    yd *= 0.8600000143051147;
    zd *= 0.8600000143051147;

    if (onGround)
    {
        xd *= 0.699999988079071;
        zd *= 0.699999988079071;
    }
}

void HeartParticle::init(Level* level, double x, double y, double z,
                         double xa, double ya, double za, float scale)
{
    xd *= 0.01;
    zd *= 0.01;
    yd = yd * 0.01 + 0.1;

    lifetime = 16;
    size = this->scale = size * 0.75f * scale;
}

uint64_t HeartParticle::GetType()
{
    return ParticleKind::HEART;
}

Particle* HeartParticle::Provider::createParticle(ePARTICLE_TYPE type,
                                                  Level* level,
                                                  double x,
                                                  double y,
                                                  double z,
                                                  double xa,
                                                  double ya,
                                                  double za,
                                                  arrayWithLength<int> params)
{
    return new HeartParticle(level, x, y, z, xa, ya, za);
}

Particle* HeartParticle::AngryVillagerProvider::createParticle(ePARTICLE_TYPE type,
                                                                Level* level,
                                                                double x,
                                                                double y,
                                                                double z,
                                                                double xa,
                                                                double ya,
                                                                double za,
                                                                arrayWithLength<int> params)
{
    HeartParticle* particle = new HeartParticle(level, x, y + 0.5, z, 0.0, 0.0, 0.0);
    particle->setMiscTex(81);
    particle->setColor(1.0f, 1.0f, 1.0f);
    return particle;
}
