#include "game/client/render/FlameParticle.hpp"

#include "game/core/BufferBuilder.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Math.hpp"

FlameParticle::FlameParticle(Level* level, double x, double y, double z,
                             double xa, double ya, double za)
    : Particle(level, x, y, z, xa, ya, za)
    , mOriginalSize(0.0f)
{
    xd = xd * 1.035 + xa;
    yd = yd * 1.035 + ya;
    zd = zd * 1.035 + za;
    xd += (random.nextFloat() - random.nextFloat()) * 0.05f;
    yd += (random.nextFloat() - random.nextFloat()) * 0.05f;
    zd += (random.nextFloat() - random.nextFloat()) * 0.05f;
    rCol = 1.0f;
    gCol = 1.0f;
    bCol = 1.0f;
    mOriginalSize = size;
    lifetime = (int)(8.0 / (Math::random() * 0.8 + 0.2)) + 4;
    setMiscTex(0x30);
}

FlameParticle::~FlameParticle()
{
}

void FlameParticle::move(double xa, double ya, double za)
{
    AABB* box = getBoundingBox();
    setBoundingBox(box->move(xa, ya, za));
    setLocationFromBoundingbox();
}

void FlameParticle::render(BufferBuilder* buffer,
                           const boost::shared_ptr<Entity>& camera,
                           float partialTicks, float xa, float ya, float za,
                           float xa2, float za2)
{
    float progress = ((float)age + partialTicks) / (float)lifetime;
    size = mOriginalSize * (1.75f - progress * progress * 0.5f);
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

int FlameParticle::getLightColor(float partialTicks)
{
    float progress = Mth::clamp(((float)age + partialTicks) / (float)lifetime, 0.0f, 1.0f);
    int light = Particle::getLightColor(partialTicks);
    int block = (light & 255) + (int)(progress * 15.0f * 16.0f);
    if (block > 240)
        block = 240;
    return (light & 0xff0000) | block;
}

float FlameParticle::getBrightness(float partialTicks)
{
    float progress = Mth::clamp(((float)age + partialTicks) / (float)lifetime, 0.0f, 1.75f);
    return Particle::getBrightness(partialTicks) * progress + (1.75f - progress);
}

void FlameParticle::tick()
{
    int currentAge = age++;
    xo = x;
    yo = y;
    zo = z;
    if (lifetime <= currentAge)
        remove();
    move(xd, yd, zd);
    xd *= 0.96;
    yd *= 0.96;
    zd *= 0.96;
    if (!onGround)
        return;
    xd *= 0.7;
    zd *= 0.7;
}

long long FlameParticle::GetType()
{
    return 0x0008020500000000LL;
}

Particle* FlameParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                  double x, double y, double z,
                                                  double xa, double ya, double za,
                                                  arrayWithLength<int> params)
{
    return new FlameParticle(level, x, y, z, xa, ya, za);
}
