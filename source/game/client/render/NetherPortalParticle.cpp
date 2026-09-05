#include "game/client/render/NetherPortalParticle.hpp"

#include "game/client/render/Particle.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/AABB.hpp"

NetherPortalParticle::NetherPortalParticle(Level* level,
                                           double x,
                                           double y,
                                           double z,
                                           double xa,
                                           double ya,
                                           double za)
    : Particle(level, x, y, z, xa, ya, za),
      scale(random.nextFloat() * 0.2f + 0.5f),
      originX(x),
      originY(y),
      originZ(z)
{
    alpha = scale;

    unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Particle_NetherPortal);
    float colourScale = random.nextFloat() * 0.6f + 0.4f;
    rCol = (float)((colour >> 16) & 255) / 255.0f * colourScale;
    gCol = (float)((colour >> 8) & 255) / 255.0f * colourScale;
    bCol = (float)(colour & 255) / 255.0f * colourScale;
    lifetime = random.nextInt(10) + 40;
    setMiscTex(random.nextInt(8));
}

NetherPortalParticle::~NetherPortalParticle()
{
}

void NetherPortalParticle::render(BufferBuilder* buffer,
                                  const boost::shared_ptr<Entity>& camera,
                                  float partialTicks,
                                  float xa,
                                  float ya,
                                  float za,
                                  float xa2,
                                  float za2)
{
    float progress = ((float)age + partialTicks) / (float)lifetime;
    alpha = scale * -(progress * progress - 1.0f);
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

uint32_t NetherPortalParticle::getLightColor(float partialTicks)
{
    unsigned int light = (unsigned int)Particle::getLightColor(partialTicks);
    float progress = (float)age / (float)lifetime;
    int block = (int)((light >> 16) & 255);
    block += (int)(progress * progress * progress * progress * 15.0f * 16.0f);
    if (block > 240)
        block = 240;
    return (light & 255) | ((unsigned int)block << 16);
}

double NetherPortalParticle::getBrightness(float partialTicks)
{
    float progress = (float)age / (float)lifetime;
    float value = progress * progress;
    value *= value;
    return (double)((float)(Particle::getBrightness(partialTicks) * (1.0f - value) + value));
}

void NetherPortalParticle::tick()
{
    float progress = (float)age / (float)lifetime;

    xo = x;
    yo = y;
    zo = z;
    ++age;

    float curve = 1.0f - ((progress * progress + progress * progress) - progress);
    float vertical = 1.0f - progress;
    x = xd * curve + originX;
    y = yd * curve + originY + (double)vertical;
    z = zd * curve + originZ;

    if (age >= lifetime)
        remove();
}

void NetherPortalParticle::move(double xa, double ya, double za)
{
    AABB* box = getBoundingBox();
    setBoundingBox(box->move(xa, ya, za));
    setLocationFromBoundingbox();
}

uint64_t NetherPortalParticle::GetType()
{
    return 0x0008020A00000000ULL;
}

Particle* NetherPortalParticle::Provider::createParticle(ePARTICLE_TYPE type,
                                                          Level* level,
                                                          double x,
                                                          double y,
                                                          double z,
                                                          double xa,
                                                          double ya,
                                                          double za,
                                                          arrayWithLength<int> params)
{
    return new NetherPortalParticle(level, x, y, z, xa, ya, za);
}
