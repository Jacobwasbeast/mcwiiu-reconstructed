#include <cmath>
#include <sstream>
#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/Textures.hpp"
#include "game/client/render/Font.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemColors.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

static ResourceLocation gItemRendererTexture;
static ResourceLocation gItemRendererTexture2;

ItemRenderer::ItemRenderer(EntityRenderDispatcher* dispatcher, Textures* textures, ItemColors* itemColors)
    : EntityRenderer(dispatcher), mItemColors(itemColors), mRandom(new Random()), mItemFrameEnabled(false), mTextures(textures), mIsGui3d(true), mDepth(0.0F)
{
    mShadowRadius = 0.15F;
    mShadowStrength = 0.75F;
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
    bindTexture(entity);
    mRandom->setSeed(187LL);
    not_null_ptr<ItemInstance> item = itemEntity->getItem();
    if (item->isEmpty())
        return;

    GlStateManager::pushMatrix();
    int count = 1;
    if (item->getCount() > 1)
        count = 2;
    if (item->getCount() > 5)
        count = 3;
    if (item->getCount() > 20)
        count = 4;
    if ((long long)item->getCount() > 0x28ffffffffLL)
        count = 5;

    GlStateManager::translatef((float)x, (float)y, (float)z);
    GlStateManager::enableRescaleNormal();

    Item* rawItem = item->getItem();
    Block* block = Block::byItem(rawItem);
    bool renderAsBlock = (rawItem == Items::skull) || (rawItem == Items::banner) || (block == Blocks::conduit);

    if (renderAsBlock)
    {
        GlStateManager::scalef(0.25F, 0.25F, 0.25F);
        if (mItemFrameEnabled)
        {
            GlStateManager::scalef(1.25F, 1.25F, 1.25F);
            GlStateManager::translatef(0.0F, 0.05F, 0.0F);
            GlStateManager::rotatef(-90.0F, 0.0F, 1.0F, 0.0F);
        }
        Minecraft* minecraft = Minecraft::GetInstance();
        BlockRenderDispatcher* dispatcher = minecraft->getBlockRenderer();
        const BlockState* state = block != NULL ? block->defaultBlockState() : NULL;
        for (int layer = 0; layer < count; ++layer)
        {
            GlStateManager::pushMatrix();
            if (layer > 0)
            {
                GlStateManager::translatef(
                    (mRandom->nextFloat() * 2.0F - 1.0F) * 0.8F,
                    (mRandom->nextFloat() * 2.0F - 1.0F) * 0.8F,
                    (mRandom->nextFloat() * 2.0F - 1.0F) * 0.8F);
            }
            if (state != NULL)
                dispatcher->renderSingleBlock(state, 1.0F, 1.0F, true);
            GlStateManager::popMatrix();
        }
        GlStateManager::disableRescaleNormal();
        GlStateManager::popMatrix();
        if (mItemFrameEnabled)
            GlStateManager::enableLighting();
        return;
    }

    GlStateManager::scalef(0.5F, 0.5F, 0.5F);
    TextureAtlasSprite* sprite = rawItem != NULL ? rawItem->getIcon(item) : NULL;
    if (sprite != NULL)
    {
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
    }
    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();
}

const ResourceLocation* ItemRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<ItemEntity> itemEntity = boost::static_pointer_cast<ItemEntity>(entity);
    not_null_ptr<ItemInstance> item = itemEntity->getItem();
    Item* rawItem = item->getItem();
    int iconType = rawItem != NULL ? rawItem->getIconType() : 0;
    return getTextureLocation(iconType);
}

const ResourceLocation* ItemRenderer::getTextureLocation(int iconType)
{
    if (iconType != 0)
        return &gItemRendererTexture2;
    return &gItemRendererTexture;
}

void ItemRenderer::renderItemBillboard(boost::shared_ptr<ItemEntity>& entity, TextureAtlasSprite* sprite, int count, float red, float green, float blue, float alpha)
{
    if (sprite == NULL)
    {
        not_null_ptr<ItemInstance> item = entity->getItem();
        Item* rawItem = item->getItem();
        int iconType = rawItem != NULL ? rawItem->getIconType() : 0;
        sprite = mTextures->getMissingIcon(iconType);
    }
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
                (mRandom->nextFloat() * 2.0F - 1.0F) * 0.3F,
                (mRandom->nextFloat() * 2.0F - 1.0F) * 0.3F,
                (mRandom->nextFloat() * 2.0F - 1.0F) * 0.3F);
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

void ItemRenderer::renderStatic(const not_null_ptr<ItemInstance>& item, boost::shared_ptr<LivingEntity> holder, ItemTransforms::TransformType transformType, float alpha, bool leftHanded, TextureAtlasSprite* sprite)
{
    if (item->isEmpty())
        return;

    Item* rawItem = item->getItem();
    PIXBeginNamedEvent(0.0F, "renderStatic %i %i", rawItem != NULL ? Item::getId(rawItem) : 0, (int)transformType);

    GlStateManager::pushMatrix();
    if (mIsGui3d)
    {
        GlStateManager::color4f(1.0F, 1.0F, 1.0F, alpha);
        GlStateManager::enableAlphaTest();
        GlStateManager::alphaFunc(4, 0.01F);
    }
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::enableRescaleNormal();

    GlStateManager::pushMatrix();
    ItemTransforms* transforms = ItemTransforms::getTransforms(item.get(), holder.get());
    ItemTransform* transform = transforms->getTransform(transformType);
    ItemTransforms::apply(transform, leftHanded);

    GlStateManager::pushMatrix();
    GlStateManager::translatef(-0.5F, -0.5F, -0.5F);

    Block* block = rawItem != NULL ? Block::byItem(rawItem) : NULL;
    if (block != NULL)
    {
        const BlockState* state = block->defaultBlockState();
        if (state != NULL && block->getRenderShape(state) == BLOCK_RENDER_MODEL)
        {
            BlockRenderDispatcher* dispatcher = Minecraft::GetInstance()->getBlockRenderer();
            if (dispatcher != NULL && dispatcher->canRenderGeneric(block))
            {
                GlStateManager::translatef(0.5F, 0.5F, 0.5F);
                dispatcher->renderSingleBlock(state, 1.0F, 1.0F, true);
                GlStateManager::popMatrix();
                GlStateManager::popMatrix();
                GlStateManager::disableRescaleNormal();
                GlStateManager::disableBlend();
                if (mIsGui3d)
                    GlStateManager::disableAlphaTest();
                GlStateManager::popMatrix();
                PIXEndNamedEvent();
                return;
            }
        }
    }

    if (sprite == NULL && rawItem != NULL)
        sprite = rawItem->getIcon(item);
    if (sprite != NULL)
    {
        if (mIsGui3d)
        {
            int color = mItemColors->getColor(item, 0);
            GlStateManager::color4f((float)((color >> 16) & 255) / 255.0F,
                                    (float)((color >> 8) & 255) / 255.0F,
                                    (float)(color & 255) / 255.0F, alpha);
        }
        blit(0.0F, 0.0F, sprite, 1.0F, 1.0F);
        if (item->hasFoil())
            renderFoilLayer(Tesselator::getInstance());
    }

    GlStateManager::popMatrix();
    GlStateManager::popMatrix();
    GlStateManager::disableRescaleNormal();
    GlStateManager::disableBlend();
    if (mIsGui3d)
        GlStateManager::disableAlphaTest();
    GlStateManager::popMatrix();
    PIXEndNamedEvent();
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

    int alpha = (int)std::ceil((double)scale * 255.0);

    if (item->isDamaged())
    {
        float damage = (float)item->getDamageValue();
        float maxDamage = (float)item->getMaxDamage();
        float remaining = (maxDamage - damage) / maxDamage;
        if (remaining < 0.0F)
            remaining = 0.0F;

        GlStateManager::disableLighting();
        GlStateManager::disableDepthTest();
        GlStateManager::disableTexture();
        Tesselator* tesselator = Tesselator::getInstance();

        int width = Math::round(13.0F - (damage * 13.0F) / maxDamage);
        int barColor = Mth::hsvToRgb(remaining / 3.0F, 1.0F, 1.0F);

        fillRect(tesselator, x + 2, y + 13, 13, 2, 0, alpha);
        fillRect(tesselator, x + 2, y + 13, width, 1, barColor, alpha);

        GlStateManager::enableTexture();
        GlStateManager::enableLighting();
        GlStateManager::enableDepthTest();
        GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    }
    else if (item->hasPotionStrengthBar())
    {
        static const int strengthWidths[4] = { 3, 6, 9, 11 };

        GlStateManager::disableLighting();
        GlStateManager::disableDepthTest();
        GlStateManager::disableTexture();
        Tesselator* tesselator = Tesselator::getInstance();

        int strength = item->GetPotionStrength();
        int width = strengthWidths[strength];

        fillRect(tesselator, x + 3, y + 13, 11, 2, 0, alpha);
        fillRect(tesselator, x + 3, y + 13, width, 2, 0x00e1eb, alpha);
        fillRect(tesselator, x + 5, y + 13, 1, 2, 0, alpha);
        fillRect(tesselator, x + 8, y + 13, 1, 2, 0, alpha);
        fillRect(tesselator, x + 11, y + 13, 1, 2, 0, alpha);

        GlStateManager::enableTexture();
        GlStateManager::enableLighting();
        GlStateManager::enableDepthTest();
        GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    }

    if (item->getCount() >= 2 || !text.empty())
    {
        std::wstring displayText = text;
        if (displayText.empty())
        {
            std::wstringstream stream;
            if (item->getCount() < 65)
                stream << item->getCount();
            else
                stream << 64 << L"+";
            displayText = stream.str();
        }

        int textColor = color == -1 ? 0xFFFFFF : color;

        GlStateManager::enableAlphaTest();
        GlStateManager::disableLighting();
        GlStateManager::disableDepthTest();

        if (font != NULL)
        {
            int textWidth = font->width(displayText);
            font->drawShadow(displayText, x + 17 - textWidth, y + 9, textColor | (alpha << 24));
        }

        GlStateManager::enableLighting();
        GlStateManager::enableDepthTest();
        GlStateManager::disableAlphaTest();
    }

    GlStateManager::disableBlend();
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
