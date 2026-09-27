#include "game/client/render/FireballRenderer.hpp"

#include <cmath>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Options.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FireBlock.hpp"

static ResourceLocation FIREBALL_LOCATION;

FireballRenderer::FireballRenderer(EntityRenderDispatcher* dispatcher,
                                   float shadowRadius)
    : EntityRenderer(dispatcher) {
    mShadowRadius = shadowRadius;
}

FireballRenderer::~FireballRenderer() {
}

void FireballRenderer::render(boost::shared_ptr<Entity>& entity, double x,
                              double y, double z, float yaw,
                              float partialTick) {
    TextureAtlasSprite* sprite = Items::fireCharge->getIcon(0);

    GlStateManager::pushMatrix();
    mDispatcher->mTextures->bindTexture(mDispatcher->mTextures->getTextureLocation(1));
    GlStateManager::translatef((float)x, (float)y, (float)z);
    GlStateManager::enableRescaleNormal();
    GlStateManager::scalef(mShadowRadius, mShadowRadius, mShadowRadius);

    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    GlStateManager::rotatef(180.0F - mDispatcher->mCameraYRot, 0.0F, 1.0F, 0.0F);
    if (mDispatcher->mOptions->mThirdPersonView == 2)
        GlStateManager::rotatef(mDispatcher->mCameraXRot, 1.0F, 0.0F, 0.0F);
    else
        GlStateManager::rotatef(-mDispatcher->mCameraXRot, 1.0F, 0.0F, 0.0F);
    if (mSolidRender)
        GlStateManager::enableColorMaterial();

    const float u0 = (float)sprite->getU0(false);
    const float u1 = (float)sprite->getU1(false);
    const float v0 = (float)sprite->getV0(false);
    const float v1 = (float)sprite->getV1(false);
    builder->begin();
    builder->normal(0.0F, 1.0F, 0.0F);
    builder->vertexUV(-0.5F, -0.25F, 0.0F, u0, v1);
    builder->vertexUV(0.5F, -0.25F, 0.0F, u1, v1);
    builder->vertexUV(0.5F, 0.75F, 0.0F, u1, v0);
    builder->vertexUV(-0.5F, 0.75F, 0.0F, u0, v0);
    builder->end();
    if (mSolidRender)
        GlStateManager::disableColorMaterial();
    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

void FireballRenderer::renderFlame(boost::shared_ptr<Entity>& entity, double x,
                                   double y, double z, float partialTick) {
    GlStateManager::disableLighting();
    FireBlock* fire = static_cast<FireBlock*>(Blocks::fire);
    TextureAtlasSprite* sprite = fire->getTextureLayer(0);
    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y, (float)z);
    const float scale = entity->mBbWidth * 1.4F;
    GlStateManager::scalef(scale, scale, scale);
    mDispatcher->mTextures->bindTexture(mDispatcher->mTextures->getTextureLocation(0));
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    const float height = (float)(entity->mY - entity->mBoundingBox->mMinY);
    GlStateManager::rotatef(180.0F - mDispatcher->mCameraYRot, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(-entity->mXRot, 1.0F, 0.0F, 0.0F);
    GlStateManager::translatef(0.0F, 0.0F, 0.1F);
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    const float u0 = (float)sprite->getU0(false);
    const float u1 = (float)sprite->getU1(false);
    const float v0 = (float)sprite->getV0(false);
    const float v1 = (float)sprite->getV1(false);
    builder->begin();
    builder->normal(0.0F, 1.0F, 0.0F);
    builder->vertexUV(-0.5F, -height, 0.0F, u0, v1);
    builder->vertexUV(0.5F, -height, 0.0F, u1, v1);
    builder->vertexUV(0.5F, 1.4F - height, 0.0F, u1, v0);
    builder->vertexUV(-0.5F, 1.4F - height, 0.0F, u0, v0);
    builder->end();
    GlStateManager::popMatrix();
    GlStateManager::enableLighting();
}

const ResourceLocation* FireballRenderer::getTextureLocation(
    boost::shared_ptr<Entity>& entity) {
    return &FIREBALL_LOCATION;
}
