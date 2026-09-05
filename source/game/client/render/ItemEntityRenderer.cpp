#include "game/client/render/ItemEntityRenderer.hpp"

#include <cmath>

#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/util/ResourceLocation.hpp"

ItemEntityRenderer::ItemEntityRenderer(EntityRenderDispatcher* dispatcher, ItemRenderer* itemRenderer)
    : EntityRenderer(dispatcher), mItemRenderer(itemRenderer), mRandom(), mItemFrame(false), mIsGui3d(true)
{
    mShadowRadius = 0.15F;
    mShadowStrength = 0.75F;
}

ItemEntityRenderer::~ItemEntityRenderer()
{
}

void ItemEntityRenderer::renderItemBillboard(boost::shared_ptr<ItemEntity> entity, TextureAtlasSprite* sprite,
                                             int count, float red, float green, float blue, float alpha)
{
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    double u0 = sprite->getU0(false);
    double u1 = sprite->getU1(false);
    double v0 = sprite->getV0(false);
    double v1 = sprite->getV1(false);
    GlStateManager::color4f(red, green, blue, alpha);

    int layers = count;
    if (layers > 5)
        layers = 5;
    for (int layer = 0; layer < layers; ++layer)
    {
        GlStateManager::pushMatrix();
        if (layer > 0)
        {
            GlStateManager::translatef((mRandom.nextFloat() * 2.0F - 1.0F) * 0.15F,
                                       (mRandom.nextFloat() * 2.0F - 1.0F) * 0.15F,
                                       (mRandom.nextFloat() * 2.0F - 1.0F) * 0.15F);
        }
        GlStateManager::rotatef(180.0F - (float)entity->mYRot, 0.0F, 1.0F, 0.0F);
        builder->begin();
        builder->normal(0.0F, 0.0F, 1.0F);
        builder->vertexUV(-0.5F, -0.25F, 0.0F, (float)u0, (float)v1);
        builder->vertexUV(0.5F, -0.25F, 0.0F, (float)u1, (float)v1);
        builder->vertexUV(0.5F, 0.75F, 0.0F, (float)u1, (float)v0);
        builder->vertexUV(-0.5F, 0.75F, 0.0F, (float)u0, (float)v0);
        tesselator->end();
        GlStateManager::popMatrix();
    }
}

void ItemEntityRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                float, float partialTick)
{
    boost::shared_ptr<ItemEntity> itemEntity = boost::static_pointer_cast<ItemEntity>(entity);
    not_null_ptr<ItemInstance> item = itemEntity->getItem();
    if (mItemRenderer == NULL)
        return;

    mRandom.setSeed(187L);
    int count = (int)getRenderAmount(item);
    float bob = std::sin(((float)itemEntity->getAge() + partialTick) / 10.0F) * 0.1F + 0.1F;

    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y + bob, (float)z);
    GlStateManager::enableRescaleNormal();
    bindTexture(entity);

    bool gui3d = mItemRenderer->isGui3d(item);
    if (gui3d)
        GlStateManager::rotatef((((float)itemEntity->getAge() + partialTick) / 20.0F + itemEntity->mYRot) * 57.295776F,
                                0.0F, 1.0F, 0.0F);
    else
        GlStateManager::rotatef(180.0F - itemEntity->mYRot, 0.0F, 1.0F, 0.0F);

    if (mItemFrame)
    {
        GlStateManager::scalef(1.25F, 1.25F, 1.25F);
        GlStateManager::translatef(0.0F, -0.05F, 0.0F);
        GlStateManager::disableLighting();
    }

    for (int layer = 0; layer < count; ++layer)
    {
        GlStateManager::pushMatrix();
        if (layer > 0)
        {
            if (gui3d)
                GlStateManager::translatef((mRandom.nextFloat() * 2.0F - 1.0F) * 0.2F,
                                           (mRandom.nextFloat() * 2.0F - 1.0F) * 0.2F,
                                           (mRandom.nextFloat() * 2.0F - 1.0F) * 0.2F);
            else
                GlStateManager::translatef(0.0F, 0.0F, 0.084375F);
        }
        mItemRenderer->renderStatic(item, boost::shared_ptr<LivingEntity>(), ItemTransforms::GROUND,
                                    partialTick, false, NULL);
        GlStateManager::popMatrix();
    }

    if (mItemFrame)
        GlStateManager::enableLighting();
    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();
}

const ResourceLocation* ItemEntityRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    static ResourceLocation location;
    return &location;
}

void ItemEntityRenderer::SetItemFrame(bool itemFrame)
{
    mItemFrame = itemFrame;
}
