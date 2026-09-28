#include "game/client/render/HugeExplosionParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation EXPLOSION_LOCATION((_TEXTURE_NAME)52);

HugeExplosionParticle::HugeExplosionParticle(Textures* textures, Level* level, double x, double y, double z,
                                             double xa, double ya, double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
    , textureManager(textures)
    , size(1.0f - (float)xa * 0.5f)
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
        textureManager->bindTexture(&EXPLOSION_LOCATION);

        const float u0 = (float)(frame % 4) * 0.25f;
        const float u1 = u0 + 0.24975f;
        const float v0 = (float)(frame / 4) * 0.25f;
        const float v1 = v0 + 0.24975f;
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
            const float y0 = py - ya * scale;
            const float y1 = py + ya * scale;
            const float z0 = pz - za * scale;
            const float z1 = pz + za * scale;
            const float xOffset = xa2 * scale;
            const float zOffset = za2 * scale;
            buffer->vertexUV(x0 - xOffset, y0, z0 - zOffset, u1, v1);
            buffer->vertexUV(x0 + xOffset, y1, z0 + zOffset, u1, v0);
            buffer->vertexUV(x1 + xOffset, y1, z1 + zOffset, u0, v0);
            buffer->vertexUV(x1 - xOffset, y0, z1 - zOffset, u0, v1);

            Tesselator::getInstance()->end();
            GlStateManager::enableLighting();
        }
    }
}

int HugeExplosionParticle::getLightColor(float partialTicks)
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

uint64_t HugeExplosionParticle::GetType()
{
    return ParticleKind::HUGE_EXPLOSION;
}

Particle* HugeExplosionParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y,
                                                           double z, double xa, double ya, double za,
                                                           arrayWithLength<int> params)
{
    return new HugeExplosionParticle(Minecraft::GetInstance()->mTextures, level, x, y, z, xa, ya, za);
}
