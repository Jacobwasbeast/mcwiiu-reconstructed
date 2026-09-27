#include "game/client/render/ExperienceOrbRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation sExperienceOrbTexture((_TEXTURE_NAME)53);

ExperienceOrbRenderer::ExperienceOrbRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher)
{
    mShadowRadius = 0.15F;
    mShadowStrength = 0.75F;
}

ExperienceOrbRenderer::~ExperienceOrbRenderer()
{
}

void ExperienceOrbRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                    float yaw, float partialTick)
{
    if (mSolidRender)
        return;

    boost::shared_ptr<ExperienceOrb> orb = boost::static_pointer_cast<ExperienceOrb>(entity);
    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y, (float)z);

    int icon = (int)orb->getIcon();
    bindTexture(entity);
    Lighting::turnOn();

    float u0 = (float)(icon % 4) * 0.0625F;
    float u1 = u0 + 0.0625F;
    float v0 = (float)(icon / 4) * 0.0625F;
    float v1 = v0 + 0.0625F;

    int brightness = orb->getLightColor();
    GlStateManager::multiTexCoord2f((float)(brightness & 0xFFFF) / 65536.0F,
                                    (float)((unsigned int)brightness >> 16) / 65536.0F);
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);

    float angle = ((float)entity->mTickCount + partialTick) * 0.1F;
    int red = (int)((Mth::sin(angle) + 1.0F) * 0.5F * 255.0F);
    int blue = (int)((Mth::sin(angle + 4.1887903F) + 1.0F) * 0.1F * 255.0F);
    float scale = 0.3F;
    GlStateManager::translatef(0.5F, 0.1F, 0.5F);
    GlStateManager::rotatef(180.0F - mDispatcher->mCameraYRot, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(-mDispatcher->mCameraXRot, 1.0F, 0.0F, 0.0F);
    GlStateManager::scalef(scale, scale, scale);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->begin();
    builder->color(red << 16 | 0xFF00 | blue, 128);
    builder->normal(0.0F, 1.0F, 0.0F);
    builder->vertexUV(-0.5F, -0.25F, 0.0F, u0, v1);
    builder->vertexUV(0.5F, -0.25F, 0.0F, u1, v1);
    builder->vertexUV(0.5F, 0.75F, 0.0F, u1, v0);
    builder->vertexUV(-0.5F, 0.75F, 0.0F, u0, v0);
    builder->end();

    GlStateManager::disableBlend();
    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* ExperienceOrbRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &sExperienceOrbTexture;
}
