#include "game/client/render/AttackSweepParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation PARTICLES_LOCATION(TN_MISC_EXPLOSION);

AttackSweepParticle::AttackSweepParticle(Textures* textures, Level* level,
                                         double x, double y, double z,
                                         double xa, double ya, double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
    , textures(textures)
{
    quadSize = (float)(-(xa * 0.5 - 1.0));
    lifetime = 4;
    rCol = random.nextFloat() * 0.6f + 0.4f;
    gCol = rCol;
    bCol = rCol;
}

void AttackSweepParticle::render(BufferBuilder* buffer,
                                 const boost::shared_ptr<Entity>& camera,
                                 float partialTicks,
                                 float xa,
                                 float ya,
                                 float za,
                                 float xa2,
                                 float za2)
{
    int frame = (int)(((float)age + partialTicks) * 3.0f / (float)lifetime);
    if (frame < 8)
    {
        textures->bindTexture(&PARTICLES_LOCATION);

        float u0 = (float)(frame % 4) * 0.25f;
        float u1 = (float)(frame / 2) * 0.5f;
        float px = (float)((x - xo) * partialTicks + xo - Particle::xOff);
        float py = (float)((y - yo) * partialTicks + yo - Particle::yOff);
        float pz = (float)((z - zo) * partialTicks + zo - Particle::zOff);
        float halfHeight = ya * quadSize * 0.5f;
        float width = xa2 * quadSize;
        float depth = za2 * quadSize;
        float left = px - xa * quadSize;
        float back = pz - za * quadSize;

        GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
        GlStateManager::disableLighting();
        Lighting::turnOff();
        buffer->begin(3);
        buffer->color(rCol, gCol, bCol);
        buffer->vertexUV(left - width, py - halfHeight, back - depth, u0 + 0.24975f, u1 + 0.4995f);
        buffer->vertexUV(left + width, py + halfHeight, back + depth, u0 + 0.24975f, u1);
        buffer->vertexUV(px + xa * quadSize + width, py + halfHeight,
                         pz + za * quadSize + depth, u0, u1);
        buffer->vertexUV(px + xa * quadSize - width, py - halfHeight,
                         pz + za * quadSize - depth, u0, u1 + 0.4995f);
        buffer->end();
        GlStateManager::enableLighting();
    }
}

int AttackSweepParticle::getLightColor(float partialTicks)
{
    return 61680;
}

void AttackSweepParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    ++age;
    if (age == lifetime)
        remove();
}

int AttackSweepParticle::getParticleTexture()
{
    return 3;
}

uint64_t AttackSweepParticle::GetType()
{
    return ParticleKind::ATTACK_SWEEP;
}

Particle* AttackSweepParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                         double x, double y, double z,
                                                         double xa, double ya, double za,
                                                         arrayWithLength<int> params)
{
    return new AttackSweepParticle(Minecraft::GetInstance()->mTextures, level,
                                   x, y, z, xa, ya, za);
}
