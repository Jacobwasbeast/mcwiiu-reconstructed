#include "game/client/render/CritParticle.hpp"

#include "game/core/Mth.hpp"

CritParticle::CritParticle(Level* level, double x, double y, double z,
                           double xa, double ya, double za)
    : Particle(level, x, y, z, xa, ya, za)
{
    _init(level, x, y, z, xa, ya, za, 1.0f);
}

CritParticle::~CritParticle()
{
}

void CritParticle::render(BufferBuilder* buffer,
                          const boost::shared_ptr<Entity>& camera,
                          float partialTicks,
                          float xa,
                          float ya,
                          float za,
                          float xa2,
                          float za2)
{
    float fade = Mth::clamp(((float)age + partialTicks) / (float)lifetime * 32.0f,
                            0.0f, 1.0f);
    alpha = sizeMultiplier * fade;
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void CritParticle::tick()
{
    int currentAge = age++;
    xo = x;
    yo = y;
    zo = z;
    if (currentAge >= lifetime)
        remove();

    move(xd, yd, zd);

    xd *= 0.7;
    zd *= 0.7;
    alpha *= 0.96f;
    gCol *= 0.9f;
    yd = yd * 0.7 - 0.1;

    if (hasPhysics)
    {
        xd *= 0.7;
        zd *= 0.7;
    }
}

void CritParticle::_init(Level* level, double x, double y, double z,
                         double xa, double ya, double za, float scale)
{
    xd = xd * 0.1 + xa * 0.4;
    yd = yd * 0.1 + ya * 0.4;
    zd = zd * 0.1 + za * 0.4;

    float color = (float)(random.nextDouble() * 0.3 + 0.6);
    rCol = color;
    gCol = color;
    bCol = color;

    size = size * 0.75f * scale;
    sizeMultiplier = size;
    lifetime = (int)(6.0 / (random.nextDouble() * 0.8 + 0.6)) * (int)scale;
    hasPhysics = true;
    setMiscTex(65);
}

uint64_t CritParticle::GetType()
{
    return 0x0000000000080214ULL;
}

Particle* CritParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                 double x, double y, double z,
                                                 double xa, double ya, double za,
                                                 arrayWithLength<int> params)
{
    CritParticle* particle = new CritParticle(level, x, y, z, xa, ya, za);
    particle->setMiscTex(65);
    return particle;
}

Particle* CritParticle::MagicProvider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                      double x, double y, double z,
                                                      double xa, double ya, double za,
                                                      arrayWithLength<int> params)
{
    CritParticle* particle = new CritParticle(level, x, y, z, xa, ya, za);
    particle->setColor((float)(particle->getRedCol() * 0.3),
                       (float)(particle->getGreenCol() * 0.8),
                       (float)particle->getBlueCol());
    particle->setMiscTex(66);
    return particle;
}

Particle* CritParticle::DamageIndicatorProvider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                                double x, double y, double z,
                                                                double xa, double ya, double za,
                                                                arrayWithLength<int> params)
{
    CritParticle* particle = new CritParticle(level, x, y, z, xa, ya, za);
    particle->setLifetime(20);
    particle->setMiscTex(67);
    return particle;
}
