#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/Random.hpp"

ItemRenderer::ItemRenderer(EntityRenderDispatcher* dispatcher, Textures* textures, ItemColors* itemColors)
    : EntityRenderer(dispatcher), mItemColors(itemColors), mRandom(new Random()), mItemFrameEnabled(false), mTextures(textures), mIsGui3d(false), mDepth(0.0F)
{
}

ItemRenderer::~ItemRenderer()
{
    delete mRandom;
}

ItemColors* ItemRenderer::getItemColors()
{
    return mItemColors;
}

void ItemRenderer::renderFoilLayer(Tesselator* tesselator)
{
    GlStateManager::depthFunc(2);
    GlStateManager::disableLighting();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(2, 1);
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    GlStateManager::matrixMode(2);
    GlStateManager::pushMatrix();
    GlStateManager::scalef(0.125F, 0.125F, 0.125F);
    GlStateManager::rotatef(-50.0F, 0.0F, 0.0F, 1.0F);
    GlStateManager::popMatrix();
    GlStateManager::pushMatrix();
    GlStateManager::scalef(0.125F, 0.125F, 0.125F);
    GlStateManager::rotatef(10.0F, 0.0F, 0.0F, 1.0F);
    GlStateManager::popMatrix();
    GlStateManager::matrixMode(0);
    GlStateManager::disableBlend();
    GlStateManager::enableLighting();
    GlStateManager::depthFunc(3);
    (void)tesselator;
}

void ItemRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick)
{
    mRandom->setSeed(187L);
    if (entity.get() != NULL)
        renderName(entity, x, y, z);
    mDepth = partialTick + yaw * 0.0F + (float)((x + y + z) * 0.0);
}

const ResourceLocation* ItemRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    if (entity.get() == NULL)
        return NULL;
    return getTextureLocation(0);
}

const ResourceLocation* ItemRenderer::getTextureLocation(int itemId)
{
    (void)itemId;
    return NULL;
}

void ItemRenderer::renderItemBillboard(boost::shared_ptr<ItemEntity>& entity, TextureAtlasSprite* sprite, int count, float x, float y, float z, float partialTick)
{
    Tesselator* tesselator = Tesselator::getInstance();
    if (sprite == NULL || tesselator == NULL)
        return;
    GlStateManager::pushMatrix();
    GlStateManager::translatef(x, y, z);
    GlStateManager::rotatef(180.0F - partialTick, 0.0F, 1.0F, 0.0F);
    for (int i = 0; i < count && i < 5; ++i)
        blit((float)i * 0.01F, (float)i * 0.01F, sprite, 1.0F, 1.0F);
    GlStateManager::popMatrix();
    (void)entity;
}

void ItemRenderer::setupGuiItem(float x, float y, float width, float height, bool enableLighting)
{
    GlStateManager::translatef(x, y, mDepth + 100.0F);
    GlStateManager::scalef(width, height, 1.0F);
    GlStateManager::translatef(8.0F, 8.0F, 0.0F);
    GlStateManager::scalef(1.0F, -1.0F, 1.0F);
    GlStateManager::scalef(16.0F, 16.0F, 16.0F);
    if (enableLighting)
        GlStateManager::enableLighting();
    else
        GlStateManager::disableLighting();
}

bool ItemRenderer::isGui3d(not_null_ptr<ItemInstance> item)
{
    (void)item;
    return true;
}

void ItemRenderer::renderStatic(const not_null_ptr<ItemInstance>& item, boost::shared_ptr<LivingEntity> entity, ItemTransforms::TransformType transformType, float partialTick, bool leftHand, TextureAtlasSprite* sprite)
{
    (void)item;
    (void)entity;
    (void)transformType;
    (void)partialTick;
    (void)leftHand;
    if (sprite != NULL)
        blit(0.0F, 0.0F, sprite, 1.0F, 1.0F);
}

void ItemRenderer::renderGuiItem(not_null_ptr<ItemInstance> item, float x, float y, float scale, float rotation, float alpha, bool flat, TextureAtlasSprite* sprite)
{
    setupGuiItem(x, y, scale, scale, !flat);
    renderStatic(item, boost::shared_ptr<LivingEntity>(), 0, rotation, flat, sprite);
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, alpha);
}

void ItemRenderer::renderGuiItem(not_null_ptr<ItemInstance> item, float x, float y, float scale, float alpha)
{
    renderGuiItem(item, x, y, scale, 0.0F, alpha, false, NULL);
}

void ItemRenderer::renderGuiItem(not_null_ptr<ItemInstance> item, int x, int y)
{
    renderGuiItem(item, (float)x, (float)y, 1.0F, 1.0F);
}

void ItemRenderer::renderAndDecorateItem(not_null_ptr<ItemInstance> item, float x, float y, float scale, float alpha, float partialTick, bool flat, bool drawDecorations, bool drawOverlay, TextureAtlasSprite* sprite)
{
    renderGuiItem(item, x, y, scale, partialTick, alpha, flat, sprite);
    if (drawDecorations)
        renderGuiItemDecorations(NULL, item, (int)x, (int)y, scale);
    if (drawOverlay)
        mDepth += 0.0F;
}

void ItemRenderer::renderAndDecorateItem(not_null_ptr<ItemInstance> item, float x, float y, float scale, float alpha, bool flat)
{
    renderAndDecorateItem(item, x, y, scale, alpha, 0.0F, flat, true, true, NULL);
}

void ItemRenderer::renderAndDecorateItem(not_null_ptr<ItemInstance> item, int x, int y)
{
    renderAndDecorateItem(item, (float)x, (float)y, 1.0F, 1.0F, false);
}

void ItemRenderer::fillRect(Tesselator* tesselator, int x, int y, int width, int height, int color, int alpha)
{
    if (tesselator == NULL)
        return;
    BufferBuilder* builder = tesselator->getBuilder();
    builder->begin();
    builder->color(color, alpha);
    builder->vertex((float)x, (float)y, mDepth);
    builder->vertex((float)x, (float)(y + height), mDepth);
    builder->vertex((float)(x + width), (float)(y + height), mDepth);
    builder->vertex((float)(x + width), (float)y, mDepth);
    tesselator->end();
}

void ItemRenderer::renderGuiItemDecorations(Font* font, not_null_ptr<ItemInstance> item, int x, int y, const std::wstring& text, int color, float scale)
{
    (void)font;
    (void)item;
    (void)text;
    (void)color;
    fillRect(Tesselator::getInstance(), x, y + (int)(13.0F * scale), 13, 2, 0, 0);
}

void ItemRenderer::renderGuiItemDecorations(Font* font, not_null_ptr<ItemInstance> item, int x, int y, float scale)
{
    renderGuiItemDecorations(font, item, x, y, std::wstring(), -1, scale);
}

void ItemRenderer::blit(float x, float y, TextureAtlasSprite* sprite, float width, float height)
{
    if (sprite == NULL)
        return;
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    builder->begin();
    builder->vertexUV(x, y + height, mDepth, (float)sprite->getU0(false), (float)sprite->getV1(false));
    builder->vertexUV(x + width, y + height, mDepth, (float)sprite->getU1(false), (float)sprite->getV1(false));
    builder->vertexUV(x + width, y, mDepth, (float)sprite->getU1(false), (float)sprite->getV0(false));
    builder->vertexUV(x, y, mDepth, (float)sprite->getU0(false), (float)sprite->getV0(false));
    tesselator->end();
}

void ItemRenderer::SetItemFrame(bool itemFrame)
{
    mItemFrameEnabled = itemFrame;
}
