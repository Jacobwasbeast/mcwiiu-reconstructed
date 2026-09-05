#include "game/client/render/ArrowRenderer.hpp"

#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

ArrowRenderer::ArrowRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher)
{
}

ArrowRenderer::~ArrowRenderer()
{
}

void ArrowRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                           float yaw, float partialTick)
{
    unsigned long long type = entity->GetType();
    float xRotO = entity->mXRotO;
    float xRot = entity->mXRot;
    float yRotO = entity->mYRotO;
    float yRot = entity->mYRot;
    float shake = 0.0F;

    bindTexture(entity);
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    GlStateManager::pushMatrix();

    float yRotDelta = yRotO + (yRot - yRotO) * partialTick;
    if (yRotDelta <= -180.0F)
        yRotDelta += 360.0F;
    else if (yRotDelta > 180.0F)
        yRotDelta -= 360.0F;

    float xRotDelta = xRotO + (xRot - xRotO) * partialTick;
    if (xRotDelta <= -180.0F)
        xRotDelta += 360.0F;
    else if (xRotDelta > 180.0F)
        xRotDelta -= 360.0F;

    GlStateManager::translatef((float)x, (float)y, (float)z);
    GlStateManager::rotatef(yRotDelta - 90.0F, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(xRotDelta, 0.0F, 0.0F, 1.0F);

    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    GlStateManager::enableRescaleNormal();

    if (shake > 0.0F)
    {
        float shakeRotation = Mth::sin(shake * 3.0F) * shake;
        GlStateManager::rotatef(-shakeRotation, 0.0F, 0.0F, 1.0F);
    }

    GlStateManager::rotatef(45.0F, 1.0F, 0.0F, 0.0F);
    GlStateManager::scalef(0.05625F, 0.05625F, 0.05625F);
    GlStateManager::translatef(-4.0F, 0.0F, 0.0F);

    if (mSolidRender)
    {
        GlStateManager::enableColorMaterial();
        GlStateManager::setupSolidRenderingTextureCombine((int)getTeamColor(entity));
    }

    builder->begin();
    builder->normal(1.0F, 0.0F, 0.0F);
    builder->vertexUV(-7.0F, -2.0F, -2.0F, 0.15625F, 0.0F);
    builder->vertexUV(-7.0F, 2.0F, -2.0F, 0.15625F, 0.15625F);
    builder->vertexUV(-7.0F, 2.0F, 2.0F, 0.0F, 0.15625F);
    builder->vertexUV(-7.0F, -2.0F, 2.0F, 0.0F, 0.15625F);
    builder->end();

    builder->begin();
    builder->normal(-1.0F, 0.0F, 0.0F);
    builder->vertexUV(-7.0F, -2.0F, 2.0F, 0.15625F, 0.0F);
    builder->vertexUV(-7.0F, 2.0F, 2.0F, 0.15625F, 0.15625F);
    builder->vertexUV(-7.0F, 2.0F, -2.0F, 0.0F, 0.15625F);
    builder->vertexUV(-7.0F, -2.0F, -2.0F, 0.0F, 0.15625F);
    builder->end();

    for (int side = 0; side < 4; ++side)
    {
        GlStateManager::rotatef(90.0F, 1.0F, 0.0F, 0.0F);
        builder->begin();
        builder->normal(0.0F, 0.0F, 1.0F);
        builder->vertexUV(-8.0F, -2.0F, 0.0F, 0.0F, 0.0F);
        builder->vertexUV(8.0F, -2.0F, 0.0F, 0.5F, 0.0F);
        builder->vertexUV(8.0F, 2.0F, 0.0F, 0.5F, 0.15625F);
        builder->vertexUV(-8.0F, 2.0F, 0.0F, 0.0F, 0.15625F);
        builder->end();
    }

    GlStateManager::disableRescaleNormal();
    GlStateManager::enableLighting();
    GlStateManager::popMatrix();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* ArrowRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    static ResourceLocation location;
    return &location;
}
