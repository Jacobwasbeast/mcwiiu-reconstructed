#include "game/client/render/HugeExplosionParticle.hpp"

#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation PARTICLES_LOCATION((_TEXTURE_NAME)3);

HugeExplosionParticle::HugeExplosionParticle(Textures* textures, Level* level, double x, double y, double z,
                                             double xa, double ya, double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
    , textureManager(textures)
    , size(1.75f - (float)za * 0.5f)
{
    lifetime = random.nextInt(4) + 6;

    const unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Particle_HugeExplosion);
    const float red = (float)((colour >> 16) & 255) / 255.0f;
    const float green = (float)((colour >> 8) & 255) / 255.0f;
    const float blue = (float)(colour & 255) / 255.0f;
    const float intensity = random.nextFloat() * 0.6f + 0.4f;
    rCol = red * intensity;
    gCol = green * intensity;
    bCol = blue * intensity;
}

HugeExplosionParticle::~HugeExplosionParticle()
{
}

void HugeExplosionParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera,
                                   float partialTicks, float xa, float ya, float za, float xa2, float za2)
{
    const int frame = (int)(((float)age + partialTicks) * 15.0f / (float)lifetime);
    if (frame < 16)
    {
        textureManager->bindTexture(&PARTICLES_LOCATION);

        const float u1 = (float)(frame & 3) * 0.25f + 0.24975f;
        const float v1 = (float)(frame / 4) * 0.25f + 0.24975f;
        const float u0 = u1 + 0.008f;
        const float v0 = v1 + 0.008f;
        const float scale = size + size;
        const float px = (float)((x - xo) * partialTicks + xo - Particle::xOff);
        const float py = (float)((y - yo) * partialTicks + yo - Particle::yOff);
        const float pz = (float)((z - zo) * partialTicks + zo - Particle::zOff);

        if (px * px + py * py + pz * pz >= 9.0f)
        {
            GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
            GlStateManager::disableLighting();
            Lighting::turnOff();
            buffer->begin();
            buffer->color(rCol, gCol, bCol, 1.0f);
            buffer->normal(0.0f, 1.0f, 0.0f);
            buffer->tex2(240);

            const float x0 = px - xa * scale;
            const float x1 = px + xa * scale;
            const float z0 = pz - za * scale;
            const float z1 = pz + za * scale;
            const float y0 = py - ya * scale;
            const float y1 = py + ya * scale;
            buffer->vertexUV(x0, y0, z0, u0, v0);
            buffer->vertexUV(x1, y1, z0, u0, v1);
            buffer->vertexUV(x1, y1, z1, u1, v1);
            buffer->vertexUV(x0, y0, z1, u1, v0);

            Tesselator::getInstance()->end();
            GlStateManager::enableLighting();
        }
    }
}

uint32_t HugeExplosionParticle::getLightColor(float partialTicks)
{
    return 0xf0f0;
}

void HugeExplosionParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    ++age;
    if (age == lifetime)
    {
        remove();
    }
}

int HugeExplosionParticle::getParticleTexture()
{
    return 3;
}

long long HugeExplosionParticle::GetType()
{
    return 0x0000000000080216LL;
}

Particle* HugeExplosionParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y,
                                                           double z, double xa, double ya, double za,
                                                           arrayWithLength<int> params)
{
    return new HugeExplosionParticle(Minecraft::GetInstance()->mTextures, level, x, y, z, xa, ya, za);
}
