#include "game/client/render/ItemFrameRenderer.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/BlockRenderer.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/MapRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/MapItem.hpp"
#include "game/item/MapItemSavedData.hpp"
#include "game/world/block/Block.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

class ItemFrame : public Entity
{
public:
    not_null_ptr<ItemInstance> getItem();
    int getRotation();
};

static float itemFrameDisplayRotation(int rotation)
{
    return static_cast<float>(rotation - (rotation & ~3)) * 45.0f;
}

ItemFrameRenderer::ItemFrameRenderer(EntityRenderDispatcher* dispatcher, ItemRenderer* itemRenderer)
    : EntityRenderer(dispatcher),
      mFrameIcon(NULL),
      mMinecraft(Minecraft::GetInstance()),
      mItemRenderer(itemRenderer),
      mBlockFrameDisplayList(-1),
      mFrameDisplayList(-1)
{
}

ItemFrameRenderer::~ItemFrameRenderer()
{
}

void ItemFrameRenderer::registerTerrainTextures(IconRegister* iconRegister)
{
    mFrameIcon = iconRegister->registerIcon(L"itemframe_back");
}

void ItemFrameRenderer::drawBlockFrame(const boost::shared_ptr<ItemFrame>& itemFrame)
{
    mMinecraft->mTextures->bindTexture(NULL);
    GlStateManager::pushMatrix();
    if (mBlockFrameDisplayList == -1)
    {
        mBlockFrameDisplayList = MemoryTracker::genLists(1);
        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();
        builder->begin();
        GlStateManager::newList(mBlockFrameDisplayList, 4864);
        const BlockState* state = Block::byId(0)->defaultBlockState();
        BlockRenderer* renderer = mMinecraft->getBlockRenderer()->getBlockRenderer();
        renderer->setFixedShape(0.0625f, 0.0625f, 0.9375f, 0.9375f, 0.9375f, 0.9375f);
        renderer->setFixedTexture(mFrameIcon);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->clearFixedTexture();
        renderer->clearFixedShape();
        renderer->setFixedShape(0.0f, 0.0f, 0.9375f, 1.0f, 1.0f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->setFixedShape(0.0f, 0.8125f, 0.9375f, 1.0f, 1.0f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->setFixedShape(0.0f, 0.1875f, 0.9375f, 1.0f, 0.8125f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->setFixedShape(0.0f, 0.0f, 0.9375f, 1.0f, 0.1875f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->clearFixedShape();
        renderer->clearFixedTexture();
        builder->end();
        GlStateManager::endList(0);
    }
    GlStateManager::callList(mBlockFrameDisplayList);
    GlStateManager::popMatrix();
}

void ItemFrameRenderer::drawFrame(const boost::shared_ptr<ItemFrame>& itemFrame)
{
    GlStateManager::pushMatrix();
    mMinecraft->mTextures->bindTexture(NULL);
    if (mFrameDisplayList == -1)
    {
        mFrameDisplayList = MemoryTracker::genLists(1);
        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();
        builder->begin();
        GlStateManager::newList(mFrameDisplayList, 4864);
        const BlockState* state = Block::byId(0)->defaultBlockState();
        BlockRenderer* renderer = mMinecraft->getBlockRenderer()->getBlockRenderer();
        renderer->setFixedShape(0.1875f, 0.1875f, 0.96875f, 0.8125f, 0.8125f, 1.0f);
        renderer->setFixedTexture(mFrameIcon);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->clearFixedTexture();
        renderer->clearFixedShape();
        renderer->setFixedShape(0.125f, 0.125f, 0.96875f, 0.875f, 0.1875f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->setFixedShape(0.125f, 0.8125f, 0.96875f, 0.875f, 0.875f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->setFixedShape(0.125f, 0.1875f, 0.96875f, 0.1875f, 0.8125f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->setFixedShape(0.8125f, 0.1875f, 0.96875f, 0.875f, 0.8125f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->clearFixedShape();
        renderer->clearFixedTexture();
        builder->end();
        GlStateManager::endList(0);
    }
    GlStateManager::callList(mFrameDisplayList);
    GlStateManager::popMatrix();
}

void ItemFrameRenderer::drawItem(const boost::shared_ptr<ItemFrame>& itemFrame)
{
    not_null_ptr<ItemInstance> item = itemFrame->getItem();
    if (item->isEmpty())
        return;
    GlStateManager::pushMatrix();
    GlStateManager::disableLighting();
    if (item->getItem() == Items::map)
    {
        int rotation = itemFrame->getRotation();
        GlStateManager::rotatef(itemFrameDisplayRotation(rotation), 0.0f, 0.0f, 1.0f);
        mMinecraft->mTextures->bindTexture(NULL);
        GlStateManager::rotatef(180.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::scalef(-0.0078125f, -0.0078125f, -0.0078125f);
        GlStateManager::translatef(-64.0f, -64.0f, 0.0f);
        boost::shared_ptr<MapItemSavedData> data = static_cast<MapItem*>(item->getItem())->getSavedData(item, itemFrame->mLevel);
        if (data)
            mMinecraft->mGameRenderer->getMapRenderer()->render(data, boost::shared_ptr<Player>());
    }
    else
    {
        GlStateManager::rotatef(itemFrameDisplayRotation(itemFrame->getRotation()), 0.0f, 0.0f, 1.0f);
        GlStateManager::scalef(0.5f, 0.5f, 0.5f);
        mItemRenderer->renderStatic(item, boost::shared_ptr<LivingEntity>(), static_cast<ItemTransforms::TransformType>(8), 0.0f, false, NULL);
    }
    GlStateManager::enableLighting();
    GlStateManager::popMatrix();
}

void ItemFrameRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick)
{
    PIXBeginNamedEvent(0.0f, "ItemFrameRenderer");
    boost::shared_ptr<ItemFrame> itemFrame = boost::static_pointer_cast<ItemFrame>(entity);
    GlStateManager::pushMatrix();
    GlStateManager::translated(x, y, z);
    GlStateManager::rotatef(180.0f - itemFrame->mYRot, 0.0f, 1.0f, 0.0f);
    GlStateManager::pushMatrix();
    GlStateManager::translatef(-0.5f, -0.5f, -0.5f);
    not_null_ptr<ItemInstance> item = itemFrame->getItem();
    if (item->getItem() == Items::map)
        drawBlockFrame(itemFrame);
    else
        drawFrame(itemFrame);
    GlStateManager::popMatrix();
    GlStateManager::translatef(0.0f, 0.0f, -0.4375f);
    drawItem(itemFrame);
    GlStateManager::popMatrix();
    PIXEndNamedEvent();
}

void ItemFrameRenderer::renderName(boost::shared_ptr<Entity>& entity, double x, double y, double z)
{
    boost::shared_ptr<ItemFrame> itemFrame = boost::static_pointer_cast<ItemFrame>(entity);
    if (!Minecraft::renderNames())
        return;
    not_null_ptr<ItemInstance> item = itemFrame->getItem();
    if (item->isEmpty() || item->hasCustomHoverName())
        return;
    if (itemFrame->distanceToSqr(entity) < 64.0)
        renderNameTag(entity, item->getHoverName(), x, y, z, 64, 0xff000000);
}
