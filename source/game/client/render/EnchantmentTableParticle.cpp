#include "game/client/render/EnchantmentTableParticle.hpp"

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
    xo = x + xa;
    yo = y + ya;
    zo = z + za;
    this->x = x + xa;
    this->y = y + ya;
    this->z = z + za;
    unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour((eMinecraftColour)247);
    float colourScale = random.nextFloat() * 0.6f + 0.4f;
    rCol = (float)((colour >> 16) & 255) / 255.0f * colourScale;
    gCol = (float)((colour >> 8) & 255) / 255.0f * colourScale;
    bCol = (float)(colour & 255) / 255.0f * colourScale;
    size = random.nextFloat() * 0.5f + 0.2f;
    lifetime = (int)(Math::random() * 10.0) + 30;
    setMiscTex((int)(Math::random() * 26.0) + 225);
}

EnchantmentTableParticle::~EnchantmentTableParticle()
{
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
    unsigned int currentAge = (unsigned int)age;
    float progress = (float)currentAge / (float)lifetime;
    float remaining = 1.0f - progress;
    float drop = remaining * remaining;
    drop *= drop;
    xo = x;
    yo = y;
    zo = z;
    x = xd * remaining + mOriginX;
    y = yd * remaining + mOriginY - drop * 0.025f;
    z = zd * remaining + mOriginZ;
    ++age;
    if (lifetime <= (int)currentAge)
        remove();
}

uint64_t EnchantmentTableParticle::GetType()
{
    return 0x000000000008021CULL;
}

Particle* EnchantmentTableParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                              double x, double y, double z,
                                                              double xa, double ya, double za,
                                                              arrayWithLength<int> params)
{
    return new EnchantmentTableParticle(level, x, y, z, xa, ya, za);
}
