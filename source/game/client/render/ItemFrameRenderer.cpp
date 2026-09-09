#include "game/client/render/ItemFrameRenderer.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/BlockRenderer.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/MapRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/MapItem.hpp"
#include "game/item/MapItemSavedData.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

static ResourceLocation MAP_BACKGROUND_LOCATION((_TEXTURE_NAME)13);

static float itemFrameDisplayRotation(int rotation)
{
    return static_cast<float>(rotation) * 45.0f;
}

ItemFrameRenderer::ItemFrameRenderer(EntityRenderDispatcher* dispatcher, ItemRenderer* itemRenderer)
    : EntityRenderer(dispatcher),
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
    mDispatcher->mTextures->bindTexture(&BLOCKS_LOCATION);
    GlStateManager::pushMatrix();
    if (mBlockFrameDisplayList == -1)
    {
        mBlockFrameDisplayList = MemoryTracker::genLists(1);
        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();
        builder->begin();
        GlStateManager::newList(mBlockFrameDisplayList, 0);
        const BlockState* state = Blocks::AIR->defaultBlockState();
        BlockRenderer* renderer = mMinecraft->getBlockRenderer()->getBlockRenderer();
        renderer->setFixedShape(0.0625f, 0.0625f, 0.9375625f, 0.9375f, 0.9375f, 1.0f);
        renderer->setFixedTexture(mFrameIcon);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->clearFixedTexture();
        renderer->clearFixedShape();
        renderer->setFixedShape(0.0f, 0.0f, 0.9375625f, 1.0f, 0.0625f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->setFixedShape(0.0f, 0.9375f, 0.9375625f, 1.0f, 1.0f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->setFixedShape(0.0f, 0.0625f, 0.9375625f, 0.0625f, 0.9375f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->setFixedShape(0.9375f, 0.0625f, 0.9375625f, 1.0f, 0.9375f, 1.0f);
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
    mDispatcher->mTextures->bindTexture(&BLOCKS_LOCATION);
    if (mFrameDisplayList == -1)
    {
        mFrameDisplayList = MemoryTracker::genLists(1);
        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();
        builder->begin();
        GlStateManager::newList(mFrameDisplayList, 0);
        const BlockState* state = Blocks::AIR->defaultBlockState();
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
        int rotation = itemFrame->getRotation() & 3;
        GlStateManager::rotatef(static_cast<float>(rotation) * 90.0f, 0.0f, 0.0f, 1.0f);
        mDispatcher->mTextures->bindTexture(&MAP_BACKGROUND_LOCATION);
        GlStateManager::rotatef(180.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::scalef(0.0078125f, 0.0078125f, 0.0078125f);
        GlStateManager::translatef(-64.0f, -64.0f, 0.0f);
        boost::shared_ptr<MapItemSavedData> data = static_cast<MapItem*>(item->getItem())->getSavedData(item, itemFrame->mLevel);
        GlStateManager::translatef(0.0f, 0.0f, -1.0f);
        if (data)
            mMinecraft->mGameRenderer->getMapRenderer()->render(data, boost::shared_ptr<Player>());
    }
    else
    {
        GlStateManager::rotatef(itemFrameDisplayRotation(itemFrame->getRotation()), 0.0f, 0.0f, 1.0f);
        GlStateManager::scalef(0.5f, 0.5f, 0.5f);
        mItemRenderer->renderStatic(item, boost::shared_ptr<LivingEntity>(), ItemTransforms::FIXED, 1.0f, false, NULL);
    }
    GlStateManager::enableLighting();
    GlStateManager::popMatrix();
}

void ItemFrameRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick)
{
    PIXBeginNamedEvent(0.0f, "ItemFrameRenderer");
    boost::shared_ptr<ItemFrame> itemFrame = boost::static_pointer_cast<ItemFrame>(entity);
    BlockPos pos = itemFrame->getPos();
    GlStateManager::pushMatrix();
    GlStateManager::translated((double)pos.x - itemFrame->mX + x + 0.5,
                               (double)pos.y - itemFrame->mY + y + 0.5,
                               (double)pos.z - itemFrame->mZ + z + 0.5);
    GlStateManager::rotatef(180.0f - itemFrame->mYRot, 0.0f, 1.0f, 0.0f);
    GlStateManager::pushMatrix();
    GlStateManager::translatef(-0.5f, -0.5f, -0.5f);
    if (mSolidRender)
    {
        GlStateManager::enableColorMaterial();
        GlStateManager::setupSolidRenderingTextureCombine((int)getTeamColor(entity));
    }
    not_null_ptr<ItemInstance> item = itemFrame->getItem();
    if (item->getItem() == Items::map)
        drawBlockFrame(itemFrame);
    else
        drawFrame(itemFrame);
    if (mSolidRender)
    {
        GlStateManager::tearDownSolidRenderingTextureCombine();
        GlStateManager::disableColorMaterial();
    }
    GlStateManager::popMatrix();
    GlStateManager::translatef(0.0f, 0.0f, -0.4375f);
    drawItem(itemFrame);
    GlStateManager::popMatrix();
    const Direction* direction = itemFrame->getDirection();
    EntityRenderer::render(entity, x + (double)direction->getStepX() * 0.3,
                           y - 0.25, z + (double)direction->getStepZ() * 0.3,
                           yaw, partialTick);
    PIXEndNamedEvent();
}

void ItemFrameRenderer::renderName(boost::shared_ptr<Entity>& entity, double x, double y, double z)
{
    boost::shared_ptr<ItemFrame> itemFrame = boost::static_pointer_cast<ItemFrame>(entity);
    if (!Minecraft::renderNames())
        return;
    not_null_ptr<ItemInstance> item = itemFrame->getItem();
    if (item->isEmpty() || !item->hasCustomHoverName() || mDispatcher->mPlayer.get() != itemFrame.get())
        return;
    double distance = itemFrame->distanceToSqr(mDispatcher->mCamera);
    float maxDistance = itemFrame->isSneaking() ? 32.0f : 64.0f;
    if (distance < (double)(maxDistance * maxDistance))
        renderNameTag(entity, item->getHoverName(), x, y, z, 64, 0xff000000);
}
