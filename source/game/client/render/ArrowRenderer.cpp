#include "game/client/render/ArrowRenderer.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/projectile/Arrow.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation ARROW_LOCATION(TN_ITEM_FIREWORK);

ArrowRenderer::ArrowRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher)
{
}

void ArrowRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                           float yaw, float partialTick)
{
    float xRotO = entity.get()->mXRotO;
    float xRot = entity.get()->mXRot;
    float yRotO = entity.get()->mYRotO;
    float yRot = entity.get()->mYRot;
    float shake = 0.0F;

    if ((entity.get()->GetType() & EntityType::ARROW) == EntityType::ARROW)
    {
        boost::shared_ptr<Arrow> arrow = boost::static_pointer_cast<Arrow>(entity);
        shake = (float)arrow.get()->mShake - partialTick;
    }

    bindTexture(entity);
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    GlStateManager::pushMatrix();

    float yRotDelta = yRot - yRotO;
    if (yRotDelta > 180.0F)
        yRotDelta = (yRot - 360.0F) - yRotO;
    else if (yRotDelta < -180.0F)
        yRotDelta = (yRot + 360.0F) - yRotO;

    float xRotDelta = xRot - xRotO;
    if (xRotDelta > 180.0F)
        xRotDelta = (xRot - 360.0F) - xRotO;
    else if (xRotDelta < -180.0F)
        xRotDelta = (xRot + 360.0F) - xRotO;

    GlStateManager::translatef((float)x, (float)y, (float)z);
    GlStateManager::rotatef(yRotO + yRotDelta * partialTick - 90.0F,
                            0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(xRotO + xRotDelta * partialTick,
                            0.0F, 0.0F, 1.0F);

    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    GlStateManager::enableRescaleNormal();

    if (shake > 0.0F)
    {
        GlStateManager::rotatef(-Mth::sin(shake * 3.0F) * shake, 0.0F, 0.0F, 1.0F);
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
    builder->vertexUV(-7.0F, -2.0F, -2.0F, 0.0F, 0.15625F);
    builder->vertexUV(-7.0F, -2.0F, 2.0F, 0.15625F, 0.15625F);
    builder->vertexUV(-7.0F, 2.0F, 2.0F, 0.15625F, 0.3125F);
    builder->vertexUV(-7.0F, 2.0F, -2.0F, 0.0F, 0.3125F);
    builder->end();

    builder->begin();
    builder->normal(-1.0F, 0.0F, 0.0F);
    builder->vertexUV(-7.0F, 2.0F, -2.0F, 0.0F, 0.15625F);
    builder->vertexUV(-7.0F, 2.0F, 2.0F, 0.15625F, 0.15625F);
    builder->vertexUV(-7.0F, -2.0F, 2.0F, 0.15625F, 0.3125F);
    builder->vertexUV(-7.0F, -2.0F, -2.0F, 0.0F, 0.3125F);
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
    return &ARROW_LOCATION;
}
