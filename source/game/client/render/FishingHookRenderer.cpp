#include "game/client/render/FishingHookRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/projectile/FishingHook.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Vec3.hpp"

FishingHookRenderer::FishingHookRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher)
{
}

FishingHookRenderer::~FishingHookRenderer()
{
}

void FishingHookRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                 float yaw, float partialTick)
{
    boost::shared_ptr<FishingHook> hook = boost::static_pointer_cast<FishingHook>(entity);
    boost::shared_ptr<Player> owner = hook->getOwner();

    if (owner)
    {
        Entity* ownerEntity = (Entity*)owner.get();

        GlStateManager::pushMatrix();
        GlStateManager::translatef((float)x, (float)y, (float)z);
        GlStateManager::enableRescaleNormal();
        GlStateManager::scalef(0.5F, 0.5F, 0.5F);
        bindTexture(entity);

        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();
        GlStateManager::rotatef(180.0F - hook->mYRot, 0.0F, 1.0F, 0.0F);
        builder->begin();
        builder->normal(0.5F, 1.75F, 0.5F);
        builder->vertexUV(-0.5F, -0.5F, 0.5F, 0.0625F, 0.1875F);
        builder->vertexUV(1.25F, -0.5F, 0.5F, 0.125F, 0.1875F);
        builder->vertexUV(1.25F, 1.25F, 0.5F, 0.125F, 0.0625F);
        builder->vertexUV(-0.5F, 1.25F, 0.5F, 0.0625F, 0.0625F);
        tesselator->end();
        GlStateManager::disableRescaleNormal();
        GlStateManager::popMatrix();

        Vec3* view = ownerEntity->getViewVector(partialTick);
        float horizontal = Mth::sqrt((float)(view->x * view->x + view->z * view->z));
        float vertical = Mth::sqrt((float)(view->x * view->x + view->y * view->y + view->z * view->z));
        double handX = ownerEntity->mXo + (ownerEntity->mX - ownerEntity->mXo) * (double)partialTick;
        double handY = ownerEntity->mYo + (ownerEntity->mY - ownerEntity->mYo) * (double)partialTick;
        double handZ = ownerEntity->mZo + (ownerEntity->mZ - ownerEntity->mZo) * (double)partialTick;
        double rotation = (double)(ownerEntity->mYRotO +
                                   (ownerEntity->mYRot - ownerEntity->mYRotO) * partialTick) * 0.01745329238474369;
        float handOffset = -0.045F;
        if (horizontal > 0.0F)
        {
            handX += (double)(Mth::cos((float)rotation) * handOffset);
            handZ += (double)(Mth::sin((float)rotation) * handOffset);
        }
        handY += (double)ownerEntity->getEyeHeight() - 0.1;

        double lineX = handX - (hook->mXo + (hook->mX - hook->mXo) * (double)partialTick);
        double lineY = handY - (hook->mYo + (hook->mY - hook->mYo) * (double)partialTick);
        double lineZ = handZ - (hook->mZo + (hook->mZ - hook->mZo) * (double)partialTick);

        GlStateManager::disableTexture();
        GlStateManager::disableLighting();
        builder->begin(5);
        builder->color(0);
        for (int segment = 0; segment <= 16; ++segment)
        {
            double fraction = (double)segment / 16.0;
            double sag = fraction * fraction + fraction;
            builder->vertex((float)(x + lineX * fraction),
                            (float)(y + lineY * fraction + sag * 0.25 - 1.0),
                            (float)(z + lineZ * fraction));
        }
        tesselator->end();
        GlStateManager::enableLighting();
        GlStateManager::enableTexture();
    }

    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* FishingHookRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    static ResourceLocation location((_TEXTURE_NAME)20);
    return &location;
}
