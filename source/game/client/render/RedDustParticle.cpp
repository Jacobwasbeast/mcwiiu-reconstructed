#include "game/client/render/RedDustParticle.hpp"

#include "game/core/Mth.hpp"

RedDustParticle::RedDustParticle(Level* level, double x, double y, double z,
                                 float red, float green, float blue)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    init(level, x, y, z, 1.0f, red, green, blue);
}

RedDustParticle::~RedDustParticle() = default;

void RedDustParticle::init(Level* level, double x, double y, double z,
                           float alpha, float red, float green, float blue)
{
    xd *= 0.1;
    yd *= 0.1;
    zd *= 0.1;

    if (red == 0.0f && green == 0.0f && blue == 0.0f)
        red = 1.0f;

    float scale = random.nextFloat() * 0.4f + 0.6f;
    rCol = (random.nextFloat() * 0.2f + 0.8f) * red * scale;
    gCol = (random.nextFloat() * 0.2f + 0.8f) * green * scale;
    bCol = (random.nextFloat() * 0.2f + 0.8f) * blue * scale;
    this->alpha *= 0.75f * alpha;
    size = this->alpha;
    lifetime = (int)(8.0 / (random.nextDouble() * 0.8 + 0.2) * alpha);
    setMiscTex(7 - (age << 3) / lifetime);
}

void RedDustParticle::render(BufferBuilder* buffer,
                             const boost::shared_ptr<Entity>& camera,
                             float partialTicks, float xa, float ya, float za,
                             float xa2, float za2)
{
    float progress = ((float)age + partialTicks) / (float)lifetime;
    float alphaScale = Mth::clamp(progress * 32.0f, 0.0f, 1.0f);
    alpha = size * alphaScale;
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void RedDustParticle::tick()
{
    int oldAge = age;
    int particleLifetime = lifetime;
    ++age;
    xo = x;
    yo = y;
    zo = z;

    if (oldAge >= particleLifetime)
        remove();

    setMiscTex(7 - (age << 3) / particleLifetime);
    move(xd, yd, zd);

    double xMotion = xd;
    double zMotion = zd;
    if (x == xo)
    {
        xMotion *= 1.1;
        zMotion *= 1.1;
    }

    xMotion *= 0.96;
    zMotion *= 0.96;
    yd *= 0.96;

    if (onGround)
    {
        xMotion *= 0.7;
        zMotion *= 0.7;
    }

    xd = xMotion;
    zd = zMotion;
}

uint64_t RedDustParticle::GetType()
{
    return 0x000000000008020BULL;
}

Particle* RedDustParticle::Provider::createParticle(ePARTICLE_TYPE type,
                                                     Level* level,
                                                     double x, double y,
                                                     double z, double xa,
                                                     double ya, double za,
                                                     arrayWithLength<int> params)
{
    return new RedDustParticle(level, x, y, z,
                               (float)xa, (float)ya, (float)za);
}
