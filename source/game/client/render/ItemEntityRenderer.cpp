#include "game/client/render/ItemEntityRenderer.hpp"

#include <cmath>

#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemColors.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"

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
                                float yaw, float partialTick)
{
    boost::shared_ptr<ItemEntity> itemEntity = boost::static_pointer_cast<ItemEntity>(entity);
    not_null_ptr<ItemInstance> item = itemEntity->getItem();
    Item* itemType = item->getItem();
    if (itemType == NULL)
        return;

    mRandom.setSeed(187L);
    int count = 1;
    if (item->getCount() > 1)
        count = 2;
    if (item->getCount() > 5)
        count = 3;
    if (item->getCount() > 20)
        count = 4;
    if (item->getCount() > 40)
        count = 5;

    TextureAtlasSprite* sprite = itemType->getIcon(item);
    if (sprite == NULL)
        return;

    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y, (float)z);
    GlStateManager::enableRescaleNormal();
    GlStateManager::scalef(0.5F, 0.5F, 0.5F);
    bindTexture(entity);
    if (mItemFrame)
    {
        GlStateManager::scalef(1.25F, 1.25F, 1.25F);
        GlStateManager::translatef(0.0F, -0.05F, 0.0F);
        GlStateManager::disableLighting();
    }

    Block* block = Block::byItem(itemType);
    const BlockState* blockState = block == NULL ? NULL : block->defaultBlockState();
    bool renderBlock = blockState != NULL && blockState->getRenderShape() != BLOCK_RENDER_INVISIBLE;
    bool render3d = itemType->getIconType() != 0;

    if (renderBlock && mIsGui3d)
    {
        float scale = 1.0F;
        if (blockState->getRenderShape() == BLOCK_RENDER_ENTITY)
            scale = 0.5F;
        GlStateManager::scalef(scale, scale, scale);
        for (int layer = 0; layer < count; ++layer)
        {
            GlStateManager::pushMatrix();
            if (layer > 0)
                GlStateManager::translatef((mRandom.nextFloat() * 2.0F - 1.0F) * 0.2F,
                                           (mRandom.nextFloat() * 2.0F - 1.0F) * 0.2F,
                                           (mRandom.nextFloat() * 2.0F - 1.0F) * 0.2F);
            Minecraft::GetInstance()->getBlockRenderer()->renderSingleBlock(blockState, 1.0F, 1.0F, true);
            GlStateManager::popMatrix();
        }
    }
    else
    {
        if (render3d)
        {
            int color = Minecraft::GetInstance()->getItemColors()->getColor(item, 0);
            GlStateManager::color4f(((color >> 16) & 255) / 255.0F,
                                    ((color >> 8) & 255) / 255.0F,
                                    (color & 255) / 255.0F, 1.0F);
        }
        renderItemBillboard(itemEntity, sprite, count, 1.0F, 1.0F, 1.0F, 1.0F);
    }

    if (item->hasFoil() && mItemRenderer != NULL)
        mItemRenderer->renderFoilLayer(Tesselator::getInstance());
    if (mItemFrame)
        GlStateManager::enableLighting();
    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();
    (void)yaw;
    (void)partialTick;
}

const ResourceLocation* ItemEntityRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    static ResourceLocation location;
    (void)entity;
    return &location;
}

void ItemEntityRenderer::SetItemFrame(bool itemFrame)
{
    mItemFrame = itemFrame;
}
