#include <cmath>
#include <sstream>
#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/Textures.hpp"
#include "game/client/render/Font.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemColors.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation gItemRendererTexture;

ItemRenderer::ItemRenderer(EntityRenderDispatcher* dispatcher, Textures* textures, ItemColors* itemColors)
    : EntityRenderer(dispatcher), mItemColors(itemColors), mRandom(new Random()), mItemFrameEnabled(false), mTextures(textures), mIsGui3d(true), mDepth(0.0F)
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

void ItemRenderer::renderFoilLayer(Tesselator*)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    GlStateManager::depthFunc(2);
    GlStateManager::disableLighting();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(2, 1);
    if (minecraft != NULL && minecraft->getTextureManager() != NULL)
        minecraft->getTextureManager()->bindTexture(&gItemRendererTexture);
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    GlStateManager::matrixMode(2);
    GlStateManager::pushMatrix();
    GlStateManager::scalef(0.125F, 0.125F, 0.125F);
    GlStateManager::translatef(
        (float)(std::fmod((double)Minecraft::currentTimeMillis(), 3000.0) / 8.0), 0.0F, 0.0F);
    GlStateManager::rotatef(-50.0F, 0.0F, 0.0F, 1.0F);
    GlStateManager::popMatrix();
    GlStateManager::pushMatrix();
    GlStateManager::scalef(0.125F, 0.125F, 0.125F);
    GlStateManager::translatef(
        (float)-(std::fmod((double)Minecraft::currentTimeMillis(), 4873.0) / 8.0), 0.0F, 0.0F);
    GlStateManager::rotatef(10.0F, 0.0F, 0.0F, 1.0F);
    GlStateManager::popMatrix();
    GlStateManager::matrixMode(0);
    GlStateManager::disableBlend();
    GlStateManager::enableLighting();
    GlStateManager::depthFunc(3);
}

void ItemRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float, float)
{
    boost::shared_ptr<ItemEntity> itemEntity = boost::static_pointer_cast<ItemEntity>(entity);
    not_null_ptr<ItemInstance> item = itemEntity->getItem();
    if (item->isEmpty() || item->getItem() == NULL)
        return;
    mRandom->setSeed(187L);
    int count = item->getCount() > 40 ? 5 : item->getCount() > 20 ? 4 : item->getCount() > 5 ? 3 : item->getCount() > 1 ? 2 : 1;
    TextureAtlasSprite* sprite = item->getItem()->getIcon(item);
    if (sprite == NULL)
        return;
    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y, (float)z);
    GlStateManager::enableRescaleNormal();
    GlStateManager::scalef(0.5F, 0.5F, 0.5F);
    bindTexture(entity);
    if (mItemFrameEnabled)
    {
        GlStateManager::scalef(1.25F, 1.25F, 1.25F);
        GlStateManager::translatef(0.0F, -0.05F, 0.0F);
        GlStateManager::disableLighting();
    }
    int color = mItemColors->getColor(item, 0);
    GlStateManager::color4f((float)((color >> 16) & 255) / 255.0F,
                            (float)((color >> 8) & 255) / 255.0F,
                            (float)(color & 255) / 255.0F, 1.0F);
    renderItemBillboard(itemEntity, sprite, count, 1.0F, 1.0F, 1.0F, 1.0F);
    if (item->hasFoil())
        renderFoilLayer(Tesselator::getInstance());
    if (mItemFrameEnabled)
        GlStateManager::enableLighting();
    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();
}

const ResourceLocation* ItemRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &gItemRendererTexture;
}

const ResourceLocation* ItemRenderer::getTextureLocation(int)
{
    return &gItemRendererTexture;
}

void ItemRenderer::renderItemBillboard(boost::shared_ptr<ItemEntity>& entity, TextureAtlasSprite* sprite, int count, float red, float green, float blue, float alpha)
{
    if (sprite == NULL)
        return;

    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    double u0 = sprite->getU0(false);
    double u1 = sprite->getU1(false);
    double v0 = sprite->getV0(false);
    double v1 = sprite->getV1(false);
    GlStateManager::color4f(red, green, blue, alpha);

    int layers = count > 5 ? 5 : count;
    for (int layer = 0; layer < layers; ++layer)
    {
        GlStateManager::pushMatrix();
        if (layer > 0)
        {
            GlStateManager::translatef(
                (mRandom->nextFloat() * 2.0F - 1.0F) * 0.15F,
                (mRandom->nextFloat() * 2.0F - 1.0F) * 0.15F,
                (mRandom->nextFloat() * 2.0F - 1.0F) * 0.15F);
        }
        GlStateManager::rotatef(180.0F - entity->mYRot, 0.0F, 1.0F, 0.0F);
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

void ItemRenderer::setupGuiItem(float x, float y, float width, float height, bool lighting)
{
    GlStateManager::translatef(x, y, mDepth + 100.0F);
    GlStateManager::scalef(width, height, 1.0F);
    GlStateManager::translatef(8.0F, 8.0F, 0.0F);
    GlStateManager::scalef(1.0F, -1.0F, 1.0F);
    GlStateManager::scalef(16.0F, 16.0F, 16.0F);
    if (lighting)
        GlStateManager::enableLighting();
    else
        GlStateManager::disableLighting();
}

bool ItemRenderer::isGui3d(not_null_ptr<ItemInstance>)
{
    return true;
}

void ItemRenderer::renderStatic(const not_null_ptr<ItemInstance>& item, boost::shared_ptr<LivingEntity>, ItemTransforms::TransformType, float, bool, TextureAtlasSprite* sprite)
{
    if (sprite == NULL && item->getItem() != NULL)
        sprite = item->getItem()->getIcon(item);
    if (sprite != NULL)
        blit(0.0F, 0.0F, sprite, 1.0F, 1.0F);
}

void ItemRenderer::renderGuiItem(not_null_ptr<ItemInstance> item, float x, float y, float scale, float rotation, float alpha, bool flat, TextureAtlasSprite* sprite)
{
    setupGuiItem(x, y, scale, scale, !flat);
    GlStateManager::rotatef(rotation, 0.0F, 0.0F, 1.0F);
    renderStatic(item, boost::shared_ptr<LivingEntity>(), ItemTransforms::GUI, 0.0F, false, sprite);
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

void ItemRenderer::renderAndDecorateItem(not_null_ptr<ItemInstance> item,
                                         float x,
                                         float y,
                                         float scale,
                                         float alpha,
                                         float partialTick,
                                         bool flat,
                                         bool decorations,
                                         bool overlay,
                                         TextureAtlasSprite* sprite)
{
    renderGuiItem(item, x, y, scale, partialTick, alpha, flat, sprite);
    if (decorations)
        renderGuiItemDecorations(NULL, item, (int)x, (int)y, scale);
    if (overlay)
        renderGuiItemDecorations(NULL, item, (int)x, (int)y, std::wstring(), -1, scale);
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
    if (item->isEmpty())
        return;

    if (item->getCount() > 1 && font != NULL)
    {
        std::wstringstream stream;
        stream << item->getCount();
        std::wstring count = stream.str();
        font->drawShadow(count,
                         x + (int)(19.0F * scale) - font->width(count),
                         y + (int)(9.0F * scale), 0xFFFFFF);
    }

    if (!text.empty() && font != NULL)
        font->drawShadow(text, x, y, color);

    if (item->isDamaged())
    {
        int maxDamage = item->getMaxDamage();
        int remaining = maxDamage - item->getDamageValue();
        fillRect(Tesselator::getInstance(), x + 2, y + 13,
                 (int)(13.0F * scale), 2, 0, 255);
        fillRect(Tesselator::getInstance(), x + 2, y + 13,
                 (int)(13.0F * scale) * remaining / maxDamage,
                 1, 0x00FF00, 255);
    }
}

void ItemRenderer::renderGuiItemDecorations(Font* font, not_null_ptr<ItemInstance> item, int x, int y, float scale)
{
    renderGuiItemDecorations(font, item, x, y, std::wstring(L"bucket"), -1, scale);
}

void ItemRenderer::blit(float x, float y, TextureAtlasSprite* sprite, float width, float height)
{
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    Minecraft* minecraft = Minecraft::GetInstance();
    float sx = (float)minecraft->mWidth / (float)minecraft->mFullWidth;
    float sy = (float)minecraft->mHeight / (float)minecraft->mFullHeight;
    float x0 = std::ceil(x * sx);
    float x1 = std::floor((x + width) * sx);
    float y0 = std::ceil(y * sy);
    float y1 = std::floor((y + height) * sy);

    builder->begin();
    builder->vertexUV(x0 / sx, y1 / sy, mDepth,
                      (float)sprite->getU0(false), (float)sprite->getV1(false));
    builder->vertexUV(x1 / sx, y1 / sy, mDepth,
                      (float)sprite->getU1(false), (float)sprite->getV1(false));
    builder->vertexUV(x1 / sx, y0 / sy, mDepth,
                      (float)sprite->getU1(false), (float)sprite->getV0(false));
    builder->vertexUV(x0 / sx, y0 / sy, mDepth,
                      (float)sprite->getU0(false), (float)sprite->getV0(false));
    tesselator->end();
}

void ItemRenderer::SetItemFrame(bool itemFrame)
{
    mItemFrameEnabled = itemFrame;
}
