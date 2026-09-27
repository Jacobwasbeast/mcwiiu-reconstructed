#include "game/client/render/FootstepParticle.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>

#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/render/Textures.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"

static ResourceLocation PARTICLES_LOCATION((_TEXTURE_NAME)3);

FootstepParticle::FootstepParticle(Textures* textures, Level* level, double x, double y, double z)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
    , textures(textures)
{
    xd = 0.0;
    yd = 0.0;
    zd = 0.0;
    lifetime = 200;
}

FootstepParticle::~FootstepParticle()
{
    return;
}

void FootstepParticle::render(BufferBuilder* buffer,
                              const boost::shared_ptr<Entity>& camera,
                              float partialTicks,
                              float xa,
                              float ya,
                              float za,
                              float xa2,
                              float za2)
{
    float progress = ((float)age + partialTicks) / (float)lifetime;
    progress *= progress;
    float alpha = std::min(1.75f, 2.0f - progress * 2.0f) * 0.2f;

    GlStateManager::disableLighting();

    float px = (float)(x - Particle::xOff);
    float py = (float)(y - Particle::yOff);
    float pz = (float)(z - Particle::zOff);
    float brightness = ((LevelSource*)level)->getBrightness(BlockPos(x, y, z));

    textures->bindTexture(&PARTICLES_LOCATION);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);

    buffer->begin();
    buffer->color(brightness, brightness, brightness, alpha);

    const float scale = 0.125f;
    const float u0 = 0.5f;
    const float u1 = 1.0f;
    buffer->vertexUV(px - scale, py, pz + scale, u0, u1);
    buffer->vertexUV(px + scale, py, pz + scale, u1, u1);
    buffer->vertexUV(px + scale, py, pz - scale, u1, u0);
    buffer->vertexUV(px - scale, py, pz - scale, u0, u0);

    Tesselator::getInstance()->end();
    GlStateManager::disableBlend();
    GlStateManager::enableLighting();
}

void FootstepParticle::tick()
{
    ++age;
    if (age == lifetime)
    {
        remove();
    }
}

int FootstepParticle::getParticleTexture()
{
    return 3;
}

uint64_t FootstepParticle::GetType()
{
    return ParticleKind::FOOTSTEP;
}

Particle* FootstepParticle::Provider::createParticle(ePARTICLE_TYPE type,
                                                      Level* level,
                                                      double x,
                                                      double y,
                                                      double z,
                                                      double xa,
                                                      double ya,
                                                      double za,
                                                      arrayWithLength<int> params)
{
    return new FootstepParticle(Minecraft::GetInstance()->mTextures, level, x, y, z);
}
