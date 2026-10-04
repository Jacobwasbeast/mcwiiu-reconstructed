#include "game/client/render/EnchantmentTableParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/BufferBuilder.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/Math.hpp"

EnchantmentTableParticle::EnchantmentTableParticle(Level* level, double x, double y, double z,
                                                   double xa, double ya, double za)
    : Particle(level, x, y, z, xa, ya, za)
    , mOriginX(x)
    , mOriginY(y)
    , mOriginZ(z)
{
    xd = xa;
    yd = ya;
    zd = za;
    xo = x + xa;
    yo = y + ya;
    zo = z + za;
    this->x = x + xa;
    this->y = y + ya;
    this->z = z + za;
    unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(
        eMinecraftColour_Particle_EnchantmentTable);
    float colourScale = random.nextFloat() * 0.6f + 0.4f;
    rCol = (float)((colour >> 16) & 255) / 255.0f * colourScale;
    gCol = (float)((colour >> 8) & 255) / 255.0f * colourScale;
    bCol = (float)(colour & 255) / 255.0f * colourScale;
    size = random.nextFloat() * 0.5f + 0.2f;
    mOriginalSize = size;
    lifetime = (int)(Math::random() * 10.0) + 30;
    setMiscTex((int)(Math::random() * 26.0 + 225.0));
}

int EnchantmentTableParticle::getLightColor(float partialTicks)
{
    int light = Particle::getLightColor(partialTicks);
    float progress = (float)age / (float)lifetime;
    float glow = progress * progress;
    glow *= glow;
    int block = ((light >> 16) & 255) + (int)(glow * 15.0f * 16.0f);
    if (block > 240)
        block = 240;
    return (light & 255) | (block << 16);
}

float EnchantmentTableParticle::getBrightness(float partialTicks)
{
    float progress = (float)age / (float)lifetime;
    float glow = progress * progress;
    glow *= glow;
    return Particle::getBrightness(partialTicks) * (1.0f - glow) + glow;
}

void EnchantmentTableParticle::move(double xa, double ya, double za)
{
    AABB* box = getBoundingBox();
    setBoundingBox(box->move(xa, ya, za));
    setLocationFromBoundingbox();
}

void EnchantmentTableParticle::tick()
{
    int currentAge = age;
    float progress = (float)currentAge / (float)lifetime;
    float remaining = 1.0f - progress;
    float drop = 1.0f - remaining;
    drop *= drop;
    drop *= drop;
    xo = x;
    yo = y;
    zo = z;
    x = xd * remaining + mOriginX;
    y = yd * remaining + mOriginY - drop * 1.2f;
    z = zd * remaining + mOriginZ;
    ++age;
    if (lifetime <= currentAge)
        remove();
}

uint64_t EnchantmentTableParticle::GetType()
{
    return ParticleKind::ENCHANTMENT_TABLE;
}

Particle* EnchantmentTableParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                              double x, double y, double z,
                                                              double xa, double ya, double za,
                                                              arrayWithLength<int> params)
{
    return new EnchantmentTableParticle(level, x, y, z, xa, ya, za);
}
