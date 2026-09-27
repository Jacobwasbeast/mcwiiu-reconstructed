#include "game/client/render/EnderParticle.hpp"
#include "game/core/EntityType.hpp"
#include "game/client/render/TextureUVCoordinateSet.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/Math.hpp"
#include "game/util/AABB.hpp"

EnderParticle::EnderParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : Particle(level, x, y, z, xa, ya, za), originX(x), originY(y), originZ(z), scale(1.0f)
{
    unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Particle_Ender);
    rCol = (float)((colour >> 16) & 255) / 255.0f;
    gCol = (float)((colour >> 8) & 255) / 255.0f;
    bCol = (float)(colour & 255) / 255.0f;
    float colourScale = (float)(random.nextFloat() * 0.6f + 0.4f);
    rCol *= colourScale; gCol *= colourScale; bCol *= colourScale;
    size = scale = (float)(random.nextFloat() * 0.2f + 0.5f);
    lifetime = (int)(Math::random() * 10.0) + 40;
    hasPhysics = false;
    setMiscTex((int)(Math::random() * 8.0));
}
EnderParticle::~EnderParticle() {}
void EnderParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa, float ya, float za, float xa2, float za2)
{
    float progress = ((float)age + partialTicks) / (float)lifetime;
    float inverseProgress = 1.0f - progress;
    size = scale * -(inverseProgress * inverseProgress - 1.0f);
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}
int EnderParticle::getLightColor(float partialTicks)
{
    int light = Particle::getLightColor(partialTicks);
    float progress = (float)age / (float)lifetime;
    int block = (light >> 16) & 255;
    block += (int)(progress * progress * 15.0f * 16.0f);
    if (block > 240) block = 240;
    return (light & 255) | (block << 16);
}
float EnderParticle::getBrightness(float partialTicks)
{
    float progress = (float)age / (float)lifetime;
    float t = progress * progress;
    t *= t;
    return (float)(Particle::getBrightness(partialTicks) * (1.0f - t) + t);
}
void EnderParticle::tick()
{
    float progress = (float)age / (float)lifetime;
    xo = x; yo = y; zo = z; ++age;
    float curve = 1.0f - ((progress * progress + progress * progress) - progress);
    float vertical = 1.0f - progress;
    x = xd * curve + originX;
    y = yd * curve + originY + vertical;
    z = zd * curve + originZ;
    if (age - 1 >= lifetime) remove();
}
void EnderParticle::move(double xa, double ya, double za)
{
    AABB* box = getBoundingBox();
    setBoundingBox(box->move(xa, ya, za));
    setLocationFromBoundingbox();
}
uint64_t EnderParticle::GetType() { return ParticleKind::ENDER; }
Particle* EnderParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> params)
{
    return new EnderParticle(level, x, y, z, xa, ya, za);
}
