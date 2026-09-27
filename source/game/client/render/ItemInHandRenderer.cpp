#include "game/client/render/ItemInHandRenderer.hpp"
#include "game/world/block/BlockId.hpp"
#include <cmath>
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/MapRenderer.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Direction.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/item/BowItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/MapItem.hpp"
#include "game/item/MapItemSavedData.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FireBlock.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/util/ResourceLocation.hpp"

namespace
{
int sItemRenderDisplayLists[5] = {-1, -1, -1, -1, -1};
static ResourceLocation TERRAIN_LOCATION((_TEXTURE_NAME)0);
static ResourceLocation WATER_OVERLAY_LOCATION((_TEXTURE_NAME)2);
static ResourceLocation MAP_BACKGROUND_LOCATION((_TEXTURE_NAME)13);

PlayerRenderer* getPlayerRenderer(EntityRenderDispatcher* dispatcher, boost::shared_ptr<LocalPlayer> player)
{
    boost::shared_ptr<Entity> entity = player;
    return static_cast<PlayerRenderer*>(dispatcher->getRenderer(entity));
}

void bindPlayerSkin(Textures* textures, boost::shared_ptr<LocalPlayer> player)
{
    int skinIndex = player->getTexture();
    int texture = textures->loadMemTexture(player->mSkinLocation, skinIndex);
    GlStateManager::bindTexture(texture);
    textures->clearLastBoundId();
}

}

ItemInHandRenderer::ItemInHandRenderer(Minecraft* minecraft)
    : mMinecraft(minecraft), mMainHandItem(ItemInstance::EMPTY), mOffHandItem(ItemInstance::EMPTY),
      mMainHandHeight(0.0f), mOffHandHeight(0.0f),
      mMainHandHeightOld(0.0f), mOffHandHeightOld(0.0f),
      mEntityRenderDispatcher(minecraft->getEntityRenderDispatcher()), mItemRenderer(minecraft->getItemRenderer()), mListId(-1)
{
    if (sItemRenderDisplayLists[0] == -1)
    {
        sItemRenderDisplayLists[0] = MemoryTracker::genLists(1);
        GlStateManager::newList(sItemRenderDisplayLists[0], 0);
        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
        builder->begin();
        for (int row = 0; row < 16; ++row)
        {
            float y0 = static_cast<float>(row) * 0.0625f;
            float v = static_cast<float>(15 - row) / 272.0f + 0.0018382353f;
            for (int column = 0; column < 16; ++column)
            {
                float y1 = y0 + 0.0625f;
                float x0 = static_cast<float>(column) * 0.0625f;
                float u = static_cast<float>(15 - column) * 0.00390625f + 0.001953125f;
                float x1 = x0 + 0.0625f;
                builder->normal(0.0f, 0.0f, 1.0f);
                builder->vertexUV(x0, y0, 0.0f, u, v);
                builder->vertexUV(x1, y0, 0.0f, u, v);
                builder->vertexUV(x1, y1, 0.0f, u, v);
                builder->vertexUV(x0, y1, 0.0f, u, v);
                builder->normal(0.0f, 0.0f, -1.0f);
                builder->vertexUV(x0, y1, -0.0625f, u, v);
                builder->vertexUV(x1, y1, -0.0625f, u, v);
                builder->vertexUV(x1, y0, -0.0625f, u, v);
                builder->vertexUV(x0, y0, -0.0625f, u, v);
                builder->normal(-1.0f, 0.0f, 0.0f);
                builder->vertexUV(x0, y0, -0.0625f, u, v);
                builder->vertexUV(x0, y0, 0.0f, u, v);
                builder->vertexUV(x0, y1, 0.0f, u, v);
                builder->vertexUV(x0, y1, -0.0625f, u, v);
                builder->normal(1.0f, 0.0f, 0.0f);
                builder->vertexUV(x1, y1, -0.0625f, u, v);
                builder->vertexUV(x1, y1, 0.0f, u, v);
                builder->vertexUV(x1, y0, 0.0f, u, v);
                builder->vertexUV(x1, y0, -0.0625f, u, v);
                builder->normal(0.0f, 1.0f, 0.0f);
                builder->vertexUV(x1, y0, 0.0f, u, v);
                builder->vertexUV(x0, y0, 0.0f, u, v);
                builder->vertexUV(x0, y0, -0.0625f, u, v);
                builder->vertexUV(x1, y0, -0.0625f, u, v);
                builder->normal(0.0f, -1.0f, 0.0f);
                builder->vertexUV(x1, y1, -0.0625f, u, v);
                builder->vertexUV(x0, y1, -0.0625f, u, v);
                builder->vertexUV(x0, y1, 0.0f, u, v);
                builder->vertexUV(x1, y1, 0.0f, u, v);
            }
        }
        builder->end();
        GlStateManager::endList(0);
    }

    if (sItemRenderDisplayLists[1] == -1)
    {
        int list = MemoryTracker::genLists(2);
        sItemRenderDisplayLists[1] = list;
        float pixels = 16.0f;
        float textureSize = 256.0f;
        float depth = 0.0625f;
        for (int size = 0; size < 2; ++size, ++list)
        {
            GlStateManager::newList(list, 0);
            BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
            builder->begin();
            float uvSize = pixels / textureSize;
            float uvEdge = uvSize + -6.1035156e-05f;
            builder->normal(0.0f, 0.0f, 1.0f);
            builder->vertexUV(0.0f, 0.0f, 0.0f, uvEdge, uvEdge);
            builder->vertexUV(1.0f, 0.0f, 0.0f, -6.1035156e-05f, uvEdge);
            builder->vertexUV(1.0f, 1.0f, 0.0f, -6.1035156e-05f, -6.1035156e-05f);
            builder->vertexUV(0.0f, 1.0f, 0.0f, uvEdge, -6.1035156e-05f);
            builder->normal(0.0f, 0.0f, -1.0f);
            float back = -depth;
            builder->vertexUV(0.0f, 1.0f, back, uvEdge, -6.1035156e-05f);
            builder->vertexUV(1.0f, 1.0f, back, -6.1035156e-05f, -6.1035156e-05f);
            builder->vertexUV(1.0f, 0.0f, back, -6.1035156e-05f, uvEdge);
            builder->vertexUV(0.0f, 0.0f, back, uvEdge, uvEdge);
            builder->normal(-1.0f, 0.0f, 0.0f);
            int layers = static_cast<int>(pixels);
            float halfTexel = 0.5f / textureSize;
            for (int layer = 0; layer < layers; ++layer)
            {
                float x = static_cast<float>(layer) / pixels;
                float u = -(uvSize * x - uvSize) - halfTexel;
                builder->vertexUV(x, 0.0f, back, u, uvSize);
                builder->vertexUV(x, 0.0f, 0.0f, u, uvSize);
                builder->vertexUV(x, 1.0f, 0.0f, u, 0.0f);
                builder->vertexUV(x, 1.0f, back, u, 0.0f);
            }
            builder->normal(1.0f, 0.0f, 0.0f);
            for (int layer = 0; layer < layers; ++layer)
            {
                float x = static_cast<float>(layer) / pixels;
                float x1 = x + 1.0f / pixels;
                float u = -(uvSize * x - uvSize) - halfTexel;
                builder->vertexUV(x1, 1.0f, back, u, 0.0f);
                builder->vertexUV(x1, 1.0f, 0.0f, u, 0.0f);
                builder->vertexUV(x1, 0.0f, 0.0f, u, uvSize);
                builder->vertexUV(x1, 0.0f, back, u, uvSize);
            }
            builder->normal(0.0f, 1.0f, 0.0f);
            for (int layer = 0; layer < 16; ++layer)
            {
                float y = static_cast<float>(layer) / pixels;
                float y1 = y + 1.0f / pixels;
                float v = -(uvSize * y - uvSize) - halfTexel;
                builder->vertexUV(0.0f, y1, 0.0f, uvSize, v);
                builder->vertexUV(1.0f, y1, 0.0f, 0.0f, v);
                builder->vertexUV(1.0f, y1, back, 0.0f, v);
                builder->vertexUV(0.0f, y1, back, uvSize, v);
            }
            builder->normal(0.0f, -1.0f, 0.0f);
            for (int layer = 0; layer < 16; ++layer)
            {
                float y = static_cast<float>(layer) / pixels;
                float v = -(uvSize * y - uvSize) - halfTexel;
                builder->vertexUV(1.0f, y, 0.0f, 0.0f, v);
                builder->vertexUV(0.0f, y, 0.0f, uvSize, v);
                builder->vertexUV(0.0f, y, back, uvSize, v);
                builder->vertexUV(1.0f, y, back, 0.0f, v);
            }
            builder->end();
            GlStateManager::endList(0);
            pixels = pixels + pixels;
            textureSize = textureSize + textureSize;
            depth = depth * 0.5f;
        }
    }

    if (sItemRenderDisplayLists[2] == -1)
    {
        sItemRenderDisplayLists[2] = MemoryTracker::genLists(1);
        GlStateManager::newList(sItemRenderDisplayLists[2], 0);
        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
        builder->begin();
        builder->normal(0.0f, 0.0f, -1.0f);
        builder->vertexUV(0.0f, 1.0f, -0.0625f, 0.0625f, 0.0f);
        builder->vertexUV(1.0f, 1.0f, -0.0625f, 0.0f, 0.0f);
        builder->vertexUV(1.0f, 0.0f, -0.0625f, 0.0f, 0.05882353f);
        builder->vertexUV(0.0f, 0.0f, -0.0625f, 0.0625f, 0.05882353f);
        builder->end();
        GlStateManager::endList(0);
    }

    if (sItemRenderDisplayLists[3] == -1)
    {
        sItemRenderDisplayLists[3] = MemoryTracker::genLists(1);
        GlStateManager::newList(sItemRenderDisplayLists[3], 0);
        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
        builder->begin();
        for (int row = 0; row < 16; ++row)
        {
            float y0 = static_cast<float>(row) * 0.0625f;
            float v = static_cast<float>(15 - row) / 544.0f + 0.00091911765f;
            for (int column = 0; column < 16; ++column)
            {
                float y1 = y0 + 0.0625f;
                float x0 = static_cast<float>(column) * 0.0625f;
                float u = static_cast<float>(15 - column) * 0.00390625f + 0.001953125f;
                float x1 = x0 + 0.0625f;
                builder->normal(0.0f, 0.0f, 1.0f);
                builder->vertexUV(x0, y0, 0.0f, u, v);
                builder->vertexUV(x1, y0, 0.0f, u, v);
                builder->vertexUV(x1, y1, 0.0f, u, v);
                builder->vertexUV(x0, y1, 0.0f, u, v);
                builder->normal(0.0f, 0.0f, -1.0f);
                builder->vertexUV(x0, y1, -0.0625f, u, v);
                builder->vertexUV(x1, y1, -0.0625f, u, v);
                builder->vertexUV(x1, y0, -0.0625f, u, v);
                builder->vertexUV(x0, y0, -0.0625f, u, v);
                builder->normal(-1.0f, 0.0f, 0.0f);
                builder->vertexUV(x0, y0, -0.0625f, u, v);
                builder->vertexUV(x0, y0, 0.0f, u, v);
                builder->vertexUV(x0, y1, 0.0f, u, v);
                builder->vertexUV(x0, y1, -0.0625f, u, v);
                builder->normal(1.0f, 0.0f, 0.0f);
                builder->vertexUV(x1, y1, -0.0625f, u, v);
                builder->vertexUV(x1, y1, 0.0f, u, v);
                builder->vertexUV(x1, y0, 0.0f, u, v);
                builder->vertexUV(x1, y0, -0.0625f, u, v);
                builder->normal(0.0f, 1.0f, 0.0f);
                builder->vertexUV(x1, y0, 0.0f, u, v);
                builder->vertexUV(x0, y0, 0.0f, u, v);
                builder->vertexUV(x0, y0, -0.0625f, u, v);
                builder->vertexUV(x1, y0, -0.0625f, u, v);
                builder->normal(0.0f, -1.0f, 0.0f);
                builder->vertexUV(x1, y1, -0.0625f, u, v);
                builder->vertexUV(x0, y1, -0.0625f, u, v);
                builder->vertexUV(x0, y1, 0.0f, u, v);
                builder->vertexUV(x1, y1, 0.0f, u, v);
            }
        }
        builder->end();
        GlStateManager::endList(0);
    }

    if (sItemRenderDisplayLists[4] == -1)
    {
        sItemRenderDisplayLists[4] = MemoryTracker::genLists(1);
        GlStateManager::newList(sItemRenderDisplayLists[4], 0);
        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
        builder->begin();
        builder->normal(0.0f, 0.0f, 1.0f);
        builder->vertexUV(0.0f, 0.0f, 0.0f, 0.0625f, 0.029411765f);
        builder->vertexUV(1.0f, 0.0f, 0.0f, 0.0f, 0.029411765f);
        builder->vertexUV(1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
        builder->vertexUV(0.0f, 1.0f, 0.0f, 0.0625f, 0.0f);
        builder->normal(0.0f, 0.0f, -1.0f);
        builder->vertexUV(0.0f, 1.0f, -0.0625f, 0.0625f, 0.0f);
        builder->vertexUV(1.0f, 1.0f, -0.0625f, 0.0f, 0.0f);
        builder->vertexUV(1.0f, 0.0f, -0.0625f, 0.0f, 0.029411765f);
        builder->vertexUV(0.0f, 0.0f, -0.0625f, 0.0625f, 0.029411765f);
        builder->normal(-1.0f, 0.0f, 0.0f);
        builder->vertexUV(0.0f, 0.0f, -0.0625f, 0.0625f, 0.029411765f);
        builder->vertexUV(0.0f, 0.0f, 0.0f, 0.0625f, 0.029411765f);
        builder->vertexUV(0.0f, 1.0f, 0.0f, 0.05859375f, 0.0f);
        builder->vertexUV(0.0f, 1.0f, -0.0625f, 0.05859375f, 0.0f);
        builder->normal(1.0f, 0.0f, 0.0f);
        builder->vertexUV(1.0f, 1.0f, -0.0625f, 0.0f, 0.0f);
        builder->vertexUV(1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
        builder->vertexUV(1.0f, 0.0f, 0.0f, 0.00390625f, 0.029411765f);
        builder->vertexUV(1.0f, 0.0f, -0.0625f, 0.00390625f, 0.029411765f);
        builder->normal(0.0f, 1.0f, 0.0f);
        builder->vertexUV(1.0f, 0.0f, 0.0f, 0.0f, 0.027573530f);
        builder->vertexUV(0.0f, 0.0f, 0.0f, 0.0625f, 0.029411765f);
        builder->vertexUV(0.0f, 0.0f, -0.0625f, 0.0625f, 0.029411765f);
        builder->vertexUV(1.0f, 0.0f, -0.0625f, 0.0f, 0.027573530f);
        builder->normal(0.0f, -1.0f, 0.0f);
        builder->vertexUV(1.0f, 1.0f, -0.0625f, 0.0f, 0.0f);
        builder->vertexUV(0.0f, 1.0f, -0.0625f, 0.0625f, 0.0018382353f);
        builder->vertexUV(0.0f, 1.0f, 0.0f, 0.0625f, 0.0018382353f);
        builder->vertexUV(1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
        builder->end();
        GlStateManager::endList(0);
    }
}

unsigned long long ItemInHandRenderer::renderItem3D(Tesselator*, float x, float y, float, float, int, int, float, eItemRenderMode mode)
{
    int listId = -1;
    switch (mode)
    {
    case ITEM_RENDER_MODE_NONE:
        listId = sItemRenderDisplayLists[3];
        break;
    case ITEM_RENDER_MODE_GUI:
        listId = sItemRenderDisplayLists[4];
        break;
    case ITEM_RENDER_MODE_3D:
        listId = sItemRenderDisplayLists[0];
        break;
    case ITEM_RENDER_MODE_FLAT:
        listId = sItemRenderDisplayLists[2];
        break;
    case ITEM_RENDER_MODE_EXTRUDED:
        listId = sItemRenderDisplayLists[1];
        break;
    case ITEM_RENDER_MODE_EXTRUDED_HIGH_RES:
        listId = sItemRenderDisplayLists[1] + 1;
        break;
    default:
        return 0;
    }
    GlStateManager::matrixMode(2);
    GlStateManager::loadIdentity();
    GlStateManager::translatef(x, y, 0.0f);
    GlStateManager::callList(listId);
    GlStateManager::loadIdentity();
    GlStateManager::matrixMode(0);
    return 0;
}

void ItemInHandRenderer::Reset()
{
    mMainHandItem = ItemInstance::EMPTY;
    mOffHandItem = ItemInstance::EMPTY;
    mMainHandHeight = 0.0f;
    mOffHandHeight = 0.0f;
    mMainHandHeightOld = 0.0f;
    mOffHandHeightOld = 0.0f;
    mListId = -1;
}

void ItemInHandRenderer::renderItem(boost::shared_ptr<LivingEntity> entity, not_null_ptr<ItemInstance> item,
                                    ItemTransforms::TransformType transform, bool leftHand, int, bool gui3d)
{
    if (item->isEmpty())
    {
        return;
    }
    Block* block = Block::byItem(item->getItem());
    GlStateManager::pushMatrix();
    if (Block::getId(block) == BlockId::OBSERVER)
    {
        if (mMinecraft->mPlayer->GetThirdPersonView(false) == 0)
        {
            GlStateManager::rotatef(-90.0f, 0.0f, 1.0f, 0.0f);
        }
    }
    bool wasGui3d = mItemRenderer->mRenderWithColor;
    mItemRenderer->mRenderWithColor = gui3d;
    mItemRenderer->renderStatic(item, entity, transform, 1.0f, leftHand, 0);
    mItemRenderer->mRenderWithColor = wasGui3d;
    GlStateManager::popMatrix();
}

bool ItemInHandRenderer::isTransparentBlock(Block* block)
{
    return block != 0 && block->getRenderLayer() == 1;
}

void ItemInHandRenderer::enableLight(float x, float y)
{
    GlStateManager::pushMatrix();
    GlStateManager::rotatef(x, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(y, 0.0f, 1.0f, 0.0f);
    Lighting::turnOn();
    GlStateManager::popMatrix();
}

void ItemInHandRenderer::setLightValue(boost::shared_ptr<AbstractClientPlayer> player)
{
    int light = mMinecraft->mLevel->getLightColor(
        BlockPos(player->mX, player->mY + player->getEyeHeight(), player->mZ), 0, -1);
    GlStateManager::multiTexCoord2f((float)(light & 0xFFFF), (float)(light >> 16));
}

void ItemInHandRenderer::setPlayerBob(boost::shared_ptr<LocalPlayer> player, float partialTick)
{
    float xRot = player->mXRotO + (player->mXRot - player->mXRotO) * partialTick;
    float yRot = player->mYRotO + (player->mYRot - player->mYRotO) * partialTick;
    float bobX = player->mViewXRotO + (player->mViewXRot - player->mViewXRotO) * partialTick;
    float bobY = player->mViewYRotO + (player->mViewYRot - player->mViewYRotO) * partialTick;
    GlStateManager::rotatef((xRot - bobX) * 0.1f, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef((yRot - bobY) * 0.1f, 0.0f, 1.0f, 0.0f);
}

float ItemInHandRenderer::calculateMapTilt(float xRot)
{
    float tilt = Mth::clamp(1.1f - xRot / 45.0f, 0.0f, 1.0f);
    return -std::cos(tilt * 3.1415927f) * 0.5f + 0.5f;
}

void ItemInHandRenderer::renderMapHand(const HumanoidArm* arm)
{
    boost::shared_ptr<LocalPlayer> player = mMinecraft->mPlayer;
    bindPlayerSkin(mMinecraft->mTextures, player);
    PlayerRenderer* renderer = getPlayerRenderer(mEntityRenderDispatcher, player);
    GlStateManager::pushMatrix();
    if (arm == HumanoidArm::RIGHT)
    {
        GlStateManager::rotatef(92.0f, 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(45.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(-41.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::translatef(0.20000000298023224f, -1.0f, 0.44999998807907104f);
        renderer->renderRightHand(player);
    }
    else
    {
        GlStateManager::rotatef(92.0f, 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(45.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(41.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::translatef(-0.20000000298023224f, -1.0f, 0.44999998807907104f);
        renderer->renderLeftHand(player);
    }
    GlStateManager::popMatrix();
}

void ItemInHandRenderer::renderMapHands()
{
    if (!mMinecraft->mPlayer->isInvisible())
    {
        GlStateManager::disableCull();
        GlStateManager::pushMatrix();
        GlStateManager::rotatef(90.0f, 0.0f, 1.0f, 0.0f);
        renderMapHand(HumanoidArm::RIGHT);
        renderMapHand(HumanoidArm::LEFT);
        GlStateManager::popMatrix();
        GlStateManager::enableCull();
    }
}

void ItemInHandRenderer::renderPlayerArm(float equip, float swing, const HumanoidArm* arm)
{
    float side = arm == HumanoidArm::LEFT ? -1.0f : 1.0f;
    float swingRoot = Mth::sqrt(swing);
    float swingSin = Mth::sin(swingRoot * 3.1415927f);
    float swingBob = -0.30000001192092896f * swingSin;
    float swingSin2 = Mth::sin(swingRoot * 3.1415927f + swingRoot * 3.1415927f);
    float swingDrop = 0.4000000059604645f * swingSin2;
    float swingRoll = Mth::sin(swing * 3.1415927f);
    GlStateManager::translatef(side * (swingBob + 0.64000004529953f),
                               equip * -0.6000000238418579f + swingDrop - 0.6000000238418579f,
                               -0.4000000059604645f * swingRoll - 0.7199999690055847f);
    GlStateManager::rotatef(side * 45.0f, 0.0f, 1.0f, 0.0f);
    float swingRootSin = Mth::sin(swingRoot * 3.1415927f);
    GlStateManager::rotatef(side * swingRootSin * 70.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(side * Mth::sin(swing * swing * 3.1415927f) * -20.0f, 0.0f, 0.0f, 1.0f);
    boost::shared_ptr<LocalPlayer> player = mMinecraft->mPlayer;
    bindPlayerSkin(mMinecraft->mTextures, player);
    GlStateManager::translatef(-side, 3.5999999046325684f, 3.5f);
    GlStateManager::rotatef(side * 120.0f, 0.0f, 0.0f, 1.0f);
    GlStateManager::rotatef(200.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(side * -135.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::translatef(side * 5.599999904632568f, 0.0f, 0.0f);
    PlayerRenderer* renderer = getPlayerRenderer(mEntityRenderDispatcher, player);
    GlStateManager::disableCull();
    if (arm == HumanoidArm::LEFT)
    {
        renderer->renderLeftHand(player);
    }
    else
    {
        renderer->renderRightHand(player);
    }
    GlStateManager::enableCull();
}

void ItemInHandRenderer::renderMap(not_null_ptr<ItemInstance> item, int hand)
{
    GlStateManager::rotatef(180.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(180.0f, 0.0f, 0.0f, 1.0f);
    GlStateManager::scalef(0.3799999952316284f, 0.3799999952316284f, 0.3799999952316284f);
    GlStateManager::disableLighting();
    mMinecraft->mTextures->bindTexture(&MAP_BACKGROUND_LOCATION);
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    GlStateManager::translatef(-0.5f, -0.5f, 0.0f);
    GlStateManager::scalef(0.0078125f, 0.0078125f, 0.0078125f);
    builder->begin();
    builder->vertexUV(-7.0f, 135.0f, 0.0f, 0.0f, 1.0f);
    builder->vertexUV(135.0f, 135.0f, 0.0f, 1.0f, 1.0f);
    builder->vertexUV(135.0f, -7.0f, 0.0f, 1.0f, 0.0f);
    builder->vertexUV(-7.0f, -7.0f, 0.0f, 0.0f, 0.0f);
    builder->end();
    boost::shared_ptr<MapItemSavedData> savedData =
        static_cast<MapItem*>(Items::filledMap)->getSavedData(item, mMinecraft->mLevel);
    if (savedData.get() != 0)
    {
        mMinecraft->mGameRenderer->getMapRenderer()->render(savedData, mMinecraft->mPlayer);
    }
    GlStateManager::enableLighting();
}

void ItemInHandRenderer::renderOneHandedMap(float equip, const HumanoidArm* arm, float swing,
                                            const not_null_ptr<ItemInstance>& item)
{
    float side = arm == HumanoidArm::RIGHT ? 1.0f : -1.0f;
    GlStateManager::translatef(side * 0.125f, -0.05999999865889549f, 0.0f);
    boost::shared_ptr<LocalPlayer> player = mMinecraft->mPlayer;
    if (!player->isInvisible())
    {
        GlStateManager::pushMatrix();
        GlStateManager::rotatef(side * 10.0f, 0.0f, 0.0f, 1.0f);
        renderPlayerArm(equip, swing, arm);
        GlStateManager::popMatrix();
    }
    GlStateManager::pushMatrix();
    GlStateManager::translatef(side * 0.5099999904632568f, equip * -1.2000000476837158f + -0.07999999821186066f,
                                -0.75f);
    float swingRoot = Mth::sqrt(swing);
    float swingSin = Mth::sin(swingRoot * 3.1415927f);
    float swingSin2 = Mth::sin(swingRoot * 6.2831854820251465f);
    float swingRoll = Mth::sin(swing * 3.1415927f);
    GlStateManager::translatef(side * (-0.5f * swingSin), -(0.30000001192092896f * swingSin - 0.4000000059604645f * swingSin2),
                               -0.30000001192092896f * swingRoll);
    GlStateManager::rotatef(-45.0f * swingSin, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(side * swingSin * -30.0f, 0.0f, 1.0f, 0.0f);
    renderMap(item, arm == HumanoidArm::RIGHT ? 0 : 1);
    GlStateManager::popMatrix();
}

void ItemInHandRenderer::renderTwoHandedMap(float partialTick, float equipProgress, float swingProgress)
{
    float swingRoot = Mth::sqrt(swingProgress);
    float swing = Mth::sin(swingProgress * 3.1415927f);
    float swingRootSin = Mth::sin(swingRoot * 3.1415927f);
    GlStateManager::translatef(0.0f, swing * 0.085f, swingRootSin * -0.17f);
    float mapTilt = calculateMapTilt(partialTick);
    GlStateManager::translatef(0.0f, mapTilt * -0.5f + equipProgress * -1.2f + 0.1f, -0.72f);
    GlStateManager::rotatef(mapTilt * -85.0f, 1.0f, 0.0f, 0.0f);
    renderMapHands();
    GlStateManager::rotatef(swingRootSin * 20.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::scalef(1.75f, 1.75f, 1.75f);
    renderMap(mMainHandItem, 0);
}

void ItemInHandRenderer::applyEatTransform(float partialTick, const HumanoidArm* arm, not_null_ptr<ItemInstance> item)
{
    float remainingTicks = (float)mMinecraft->mPlayer->getUseItemRemainingTicks() - partialTick + 1.0f;
    float useProgress = remainingTicks / (float)item->getUseDuration();
    float bob = 0.0f;
    if (useProgress < 0.8f)
    {
        bob = std::fabs(Mth::cos(remainingTicks * 0.25f * 3.1415927f) * 0.1f);
    }
    GlStateManager::translatef(0.0f, bob, 0.0f);
    float pull = 1.0f - std::pow(useProgress, 27.0f);
    int side = arm == HumanoidArm::RIGHT ? 1 : -1;
    GlStateManager::translatef(pull * 0.6f * (float)side, pull * -0.5f, pull * 0.0f);
    GlStateManager::rotatef((float)side * pull * 90.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(pull * 10.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef((float)side * pull * 30.0f, 0.0f, 0.0f, 1.0f);
}

void ItemInHandRenderer::applyItemArmAttackTransform(const HumanoidArm* arm, float swingProgress)
{
    int side = arm == HumanoidArm::RIGHT ? 1 : -1;
    float swing = Mth::sin(swingProgress * swingProgress * 3.1415927f);
    GlStateManager::rotatef((float)side * (45.0f + swing * -20.0f), 0.0f, 1.0f, 0.0f);
    float swingRoot = Mth::sin(Mth::sqrt(swingProgress) * 3.1415927f);
    GlStateManager::rotatef((float)side * swingRoot * -20.0f, 0.0f, 0.0f, 1.0f);
    GlStateManager::rotatef(swingRoot * -80.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef((float)side * -45.0f, 0.0f, 1.0f, 0.0f);
}

void ItemInHandRenderer::applyItemArmTransform(const HumanoidArm* arm, float equipProgress)
{
    int side = arm == HumanoidArm::RIGHT ? 1 : -1;
    GlStateManager::translatef((float)side * 0.56f, -0.52f + equipProgress * -0.6f, -0.72f);
}

void ItemInHandRenderer::renderArmWithItem(boost::shared_ptr<LocalPlayer> player, float partialTick,
                                           InteractionHand::EInteractionHand hand, float equipProgress,
                                           not_null_ptr<ItemInstance> item, float swingProgress)
{
    bool mainHand = hand == InteractionHand::eInteractionHand_MainHand;
    const HumanoidArm* arm = player->getMainArm();
    if (!mainHand)
    {
        arm = arm->getOpposite();
    }
    float side = 0.0f;
    float depth = 0.0f;
    bool flipSide = false;
    if (mainHand)
    {
        side = 0.0f;
        depth = 0.0f;
        if (player->mViewport == 2)
        {
            flipSide = true;
            depth = 0.07999999821186066f;
        }
        else if (player->mViewport != 1)
        {
            if (player->mViewport != 3 && player->mViewport != 4 && gRenderer->IsWidescreen())
            {
                side = 0.0f;
            }
            else if (arm != HumanoidArm::LEFT)
            {
                side = -0.30000001192092896f;
            }
            else
            {
                depth = 0.30000001192092896f;
            }
        }
    }
    GlStateManager::pushMatrix();
    if (item->isEmpty())
    {
        if (mainHand && !player->isInvisible())
        {
            GlStateManager::translatef(side, depth, 0.0f);
            renderPlayerArm(equipProgress, swingProgress, arm);
        }
        GlStateManager::popMatrix();
        return;
    }
    if (item->getItem() == Items::filledMap)
    {
        if (flipSide)
        {
            GlStateManager::translatef(0.0f, 0.0f, -0.30000001192092896f);
        }
        if (mainHand && mOffHandItem->isEmpty())
        {
            renderTwoHandedMap(partialTick, equipProgress, swingProgress);
            GlStateManager::popMatrix();
            return;
        }
        GlStateManager::translatef(side, depth, 0.0f);
        renderOneHandedMap(equipProgress, arm, swingProgress, item);
        GlStateManager::popMatrix();
        return;
    }
    bool rightArm = arm == HumanoidArm::RIGHT;
    if (player->isUsingItem() && player->getUsedItemHand() == hand)
    {
        int animation = item->getUseAnimation();
        if (animation == 0)
        {
            GlStateManager::translatef(side, depth, 0.0f);
            applyItemArmTransform(arm, equipProgress);
        }
        else if (animation < 3)
        {
            applyEatTransform(partialTick, arm, item);
            applyItemArmTransform(arm, equipProgress);
        }
        else if (animation == 3)
        {
            applyItemArmTransform(arm, equipProgress);
            Item* useItem = item->getItem();
            if (useItem == Items::ironSword || useItem == Items::woodenSword || useItem == Items::stoneSword
                || useItem == Items::diamondSword || useItem == Items::goldenSword)
            {
                float blockSide = rightArm ? 1.0f : -1.0f;
                GlStateManager::translatef(blockSide * 0.07999999821186066f, 0.07999999821186066f, 0.0f);
                GlStateManager::rotatef(-80.0f, 1.0f, 0.0f, 0.0f);
                GlStateManager::rotatef(blockSide * 10.0f, 0.0f, 0.0f, 1.0f);
                GlStateManager::rotatef(blockSide * -0.27856820821762085f, 0.0f, 1.0f, 0.0f);
            }
        }
        else if (animation == 4)
        {
            GlStateManager::translatef(side, depth, 0.0f);
            applyItemArmTransform(arm, equipProgress);
            float blockSide = rightArm ? 1.0f : -1.0f;
            GlStateManager::translatef(blockSide * -0.27856820821762085f, 0.18344387412071228f, -13.9350004196167f);
            GlStateManager::rotatef(35.29999923706055f, 1.0f, 0.0f, 0.0f);
            GlStateManager::rotatef(blockSide * -9.78499984741211f, 0.0f, 1.0f, 0.0f);
            GlStateManager::rotatef(blockSide * 1.2999999523162842f, 0.0f, 0.0f, 1.0f);
            float useDuration = (float)item->getUseDuration();
            float remaining = (float)player->getUseItemRemainingTicks() - partialTick + 1.0f;
            float draw = (useDuration - remaining) / (float)BowItem::GetMaxBowDuration();
            float power = (draw * draw + draw + draw) / 3.0f;
            if (power > 1.0f)
            {
                if (0.004000000189989805f < power)
                {
                    float pull = Mth::sin((remaining - 0.004000000189989805f) * 0.004000000189989805f);
                    float offset = pull * (power - 0.004000000189989805f);
                    GlStateManager::translatef(offset, offset * 0.03999999910593033f, offset);
                }
                power = 1.0f;
            }
            else if (0.004000000189989805f < power)
            {
                float pull = Mth::sin((remaining - 0.004000000189989805f) * 0.004000000189989805f);
                float offset = pull * (1.0f - 0.004000000189989805f);
                GlStateManager::translatef(offset, offset * 0.03999999910593033f, offset);
            }
            GlStateManager::translatef(power, power, power * 0.5128204822540283f + 1.0f);
            GlStateManager::scalef(1.0f, 1.0f, power * 0.20000000298023224f + 1.0f);
            GlStateManager::rotatef(blockSide * 45.0f, 0.0f, -1.0f, 0.0f);
        }
        else if (animation == 5)
        {
            applyItemArmTransform(arm, equipProgress);
            float useDuration = (float)item->getUseDuration();
            float remaining = (float)player->getUseItemRemainingTicks() - partialTick + 1.0f;
            float draw = (useDuration - remaining) / 10.0f;
            float power = Mth::clamp(draw, 0.0f, 1.0f);
            GlStateManager::translatef(0.0f, 0.8500000238418579f, 0.0f);
            GlStateManager::translatef(0.0f, 0.0f, power * 0.30000001192092896f + 0.05000000074505806f);
            GlStateManager::rotatef(-60.0f, 1.0f, 0.0f, 0.0f);
            GlStateManager::rotatef(15.0f, 0.0f, 1.0f, 0.0f);
            if (0.004000000189989805f < power)
            {
                float pull = Mth::sin((remaining - 0.004000000189989805f) * 0.004000000189989805f);
                GlStateManager::translatef(0.0f, pull * 0.46875f * (power - 0.004000000189989805f), 0.0f);
            }
        }
    }
    else
    {
        if (!player->isHandsBusy())
        {
            if (equipProgress == 0.0f)
            {
                GlStateManager::translatef(side, depth, 0.0f);
            }
            float swingRoot = Mth::sqrt(swingProgress);
            float swingSin = Mth::sin(swingRoot * 3.1415927f);
            float swingSin2 = Mth::sin(swingRoot * 6.2831854820251465f);
            float swingRoll = Mth::sin(swingProgress * 3.1415927f);
            GlStateManager::translatef((rightArm ? 1.0f : -1.0f) * (-0.4000000059604645f * swingSin),
                                       0.20000000298023224f * swingSin2, 27.0f * swingRoll);
            applyItemArmTransform(arm, equipProgress);
            applyItemArmAttackTransform(arm, swingProgress);
        }
        else
        {
            applyItemArmTransform(arm, equipProgress);
            GlStateManager::translatef(0.25f, 1.0f, -0.5f);
            GlStateManager::rotatef(20.0f, 0.0f, 1.0f, 0.0f);
            GlStateManager::rotatef(-80.0f, 1.0f, 0.0f, 0.0f);
            GlStateManager::rotatef(-135.0f, 0.0f, 0.0f, 1.0f);
            GlStateManager::scalef(2.5f, 2.5f, 2.5f);
        }
    }
    ItemTransforms::TransformType transform = rightArm ? ItemTransforms::FIRST_PERSON_RIGHT_HAND
                                                       : ItemTransforms::FIRST_PERSON_LEFT_HAND;
    renderItem(player, item, transform, !rightArm, 0, true);
    GlStateManager::popMatrix();
}

void ItemInHandRenderer::render(float partialTick)
{
    boost::shared_ptr<LocalPlayer> player = mMinecraft->mPlayer;
    float attackProgress = player->getAttackAnim(partialTick);
    float mainHandHeight =
        mMainHandHeightOld + (mMainHandHeight - mMainHandHeightOld) * partialTick;
    float offHandHeight = mOffHandHeightOld + (mOffHandHeight - mOffHandHeightOld) * partialTick;
    bool renderMainHand = true;
    bool renderOffHand = true;
    if (player->isUsingItem())
    {
        not_null_ptr<ItemInstance> useItem = player->getUseItem();
        if (!useItem->isEmpty() && useItem->getItem() == Items::bow)
        {
            InteractionHand::EInteractionHand usedHand = player->getUsedItemHand();
            renderMainHand = usedHand == InteractionHand::eInteractionHand_MainHand;
            renderOffHand = usedHand != InteractionHand::eInteractionHand_MainHand;
        }
    }
    float pitchDelta = player->mXRot - player->mXRotO;
    float pitch = pitchDelta * partialTick + player->mXRotO;
    float yawDelta = player->mYRot - player->mYRotO;
    float yaw = yawDelta * partialTick + player->mYRotO;
    enableLight(pitch, yaw);
    setLightValue(player);
    setPlayerBob(player, partialTick);
    GlStateManager::enableRescaleNormal();
    const float white = 1.0f;
    const float tintScale = 255.0f;
    if (renderMainHand)
    {
        if (!mMainHandItem->isEmpty())
        {
            int tint = mMainHandItem->getItem()->getColor(mMainHandItem, 0);
            GlStateManager::color4f((float)((tint >> 16) & 0xFF) / tintScale,
                                    (float)((tint >> 8) & 0xFF) / tintScale,
                                    (float)(tint & 0xFF) / tintScale, white);
        }
        else
        {
            GlStateManager::color4f(white, white, white, white);
        }
        float mainEquip = 1.0f - mainHandHeight;
        float mainSwing = player->mSwingingArm == InteractionHand::eInteractionHand_MainHand
                              ? attackProgress
                              : 0.0f;
        renderArmWithItem(player, partialTick, InteractionHand::eInteractionHand_MainHand, mainEquip,
                          mMainHandItem, mainSwing);
    }
    if (renderOffHand)
    {
        if (!mOffHandItem->isEmpty())
        {
            int tint = mOffHandItem->getItem()->getColor(mOffHandItem, 0);
            GlStateManager::color4f((float)((tint >> 16) & 0xFF) / tintScale,
                                    (float)((tint >> 8) & 0xFF) / tintScale,
                                    (float)(tint & 0xFF) / tintScale, white);
        }
        else
        {
            GlStateManager::color4f(white, white, white, white);
        }
        float offEquip = 1.0f - offHandHeight;
        float offSwing = player->mSwingingArm == InteractionHand::eInteractionHand_OffHand
                             ? attackProgress
                             : 0.0f;
        renderArmWithItem(player, partialTick, InteractionHand::eInteractionHand_OffHand, offEquip,
                          mOffHandItem, offSwing);
    }
    GlStateManager::disableRescaleNormal();
    Lighting::turnOff();
}

void ItemInHandRenderer::renderTex(TextureAtlasSprite* sprite)
{
    mMinecraft->mTextures->bindTexture(&TERRAIN_LOCATION);
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    GlStateManager::color4f(0.1f, 0.1f, 0.1f, 0.5f);
    GlStateManager::pushMatrix();
    float u0 = (float)sprite->getU0(false);
    float v0 = (float)sprite->getV0(false);
    float u1 = (float)sprite->getU1(false);
    float v1 = (float)sprite->getV1(false);
    builder->begin();
    builder->vertexUV(-1.0f, -1.0f, -0.5f, u0, v1);
    builder->vertexUV(1.0f, -1.0f, -0.5f, u1, v1);
    builder->vertexUV(1.0f, 1.0f, -0.5f, u1, v0);
    builder->vertexUV(-1.0f, 1.0f, -0.5f, u0, v0);
    builder->end();
    GlStateManager::popMatrix();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void ItemInHandRenderer::renderWater(float)
{
    Textures* textures = mMinecraft->mTextures;
    textures->bindTexture(&WATER_OVERLAY_LOCATION);
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    boost::shared_ptr<LocalPlayer> player = mMinecraft->mPlayer;
    float vision = (float)player->getUnderwaterVisionScale();
    GlStateManager::color4f(vision, vision, vision, 0.1f);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 1);
    GlStateManager::blendFuncSeparate(4, 5, 1, 0);
    GlStateManager::pushMatrix();
    float xRot = player->mXRot * 0.015625f;
    float yRot = player->mYRot * 0.015625f;
    float x0 = 4.0f + yRot;
    float y0 = 4.0f + xRot;
    builder->begin();
    builder->vertexUV(-1.0f, -1.0f, -0.5f, -xRot - 4.0f, y0);
    builder->vertexUV(1.0f, -1.0f, -0.5f, -xRot, y0);
    builder->vertexUV(1.0f, 1.0f, -0.5f, -xRot, x0);
    builder->vertexUV(-1.0f, 1.0f, -0.5f, -xRot - 4.0f, x0);
    tesselator->end();
    GlStateManager::popMatrix();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::disableBlend();
}

void ItemInHandRenderer::renderFire()
{
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Fire_Overlay);
    float r = (float)((colour >> 16) & 0xFF) / 255.0f;
    float g = (float)((colour >> 8) & 0xFF) / 255.0f;
    float b = (float)(colour & 0xFF) / 255.0f;
    float a = (float)(colour >> 24) / 255.0f;
    GlStateManager::color4f(r, g, b, a);
    GlStateManager::depthFunc(7);
    GlStateManager::depthMask(false);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    FireBlock* fire = static_cast<FireBlock*>(Blocks::fire);
    TextureAtlasSprite* sprite = fire->getTextureLayer(1);
    mMinecraft->mTextures->bindTexture(&TERRAIN_LOCATION);
    float u0 = (float)sprite->getU0(true);
    float v0 = (float)sprite->getV0(true);
    float u1 = (float)sprite->getU1(true);
    float v1 = (float)sprite->getV1(true);
    GlStateManager::pushMatrix();
    GlStateManager::translatef(-0.24f, 0.0f, 0.0f);
    GlStateManager::rotatef(-10.0f, 0.0f, 1.0f, 0.0f);
    builder->begin();
    builder->vertexUV(-0.5f, -0.5f, -0.5f, u1, v1);
    builder->vertexUV(0.5f, -0.5f, -0.5f, u0, v1);
    builder->vertexUV(0.5f, 0.5f, -0.5f, u0, v0);
    builder->vertexUV(-0.5f, 0.5f, -0.5f, u1, v0);
    builder->end();
    GlStateManager::popMatrix();
    GlStateManager::pushMatrix();
    GlStateManager::translatef(0.24f, 0.0f, 0.0f);
    GlStateManager::rotatef(15.0f, 0.0f, 1.0f, 0.0f);
    builder->begin();
    builder->vertexUV(-0.5f, -0.5f, -0.5f, u1, v1);
    builder->vertexUV(0.5f, -0.5f, -0.5f, u0, v1);
    builder->vertexUV(0.5f, 0.5f, -0.5f, u0, v0);
    builder->vertexUV(-0.5f, 0.5f, -0.5f, u1, v0);
    builder->end();
    GlStateManager::popMatrix();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::disableBlend();
    GlStateManager::depthMask(true);
    GlStateManager::depthFunc(3);
}

void ItemInHandRenderer::renderScreenEffect(float partialTick)
{
    GlStateManager::disableAlphaTest();
    if (mMinecraft->mPlayer->isInWall())
    {
        const BlockState* state = mMinecraft->mLevel->getBlockState(BlockPos(mMinecraft->mPlayer));
        boost::shared_ptr<LocalPlayer> player = mMinecraft->mPlayer;
        for (int sample = 0; sample < 8; ++sample)
        {
            const float offsetX = static_cast<float>(sample % 2) - 0.5f;
            const float offsetY = static_cast<float>((sample >> 1) % 2) - 0.5f;
            const float offsetZ = static_cast<float>((sample >> 2) % 2) - 0.5f;
            const BlockPos samplePos(player->mX + offsetX * player->mBbWidth * 0.8f,
                                     player->mY + offsetY * 0.1f + player->getEyeHeight(),
                                     player->mZ + offsetZ * player->mBbWidth * 0.8f);
            const BlockState* sampleState = mMinecraft->mLevel->getBlockState(samplePos);
            if (sampleState->isViewBlocking())
            {
                state = sampleState;
            }
        }
        if (state->getBlock()->defaultBlockState()->getRenderShape() != BLOCK_RENDER_INVISIBLE)
        {
            renderTex(state->getBlock()->getIconTexture(Direction::UP, state));
        }
    }
    if (!mMinecraft->mPlayer->isSpectator())
    {
        if (mMinecraft->mPlayer->isUnderLiquid(Material::WATER))
        {
            renderWater(partialTick);
        }
        if (mMinecraft->mPlayer->isOnFire())
        {
            renderFire();
        }
    }
    GlStateManager::enableAlphaTest();
}

void ItemInHandRenderer::CalculateNewHandHeights(bool compareTags)
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mPlayer;
    not_null_ptr<ItemInstance> mainHandItem = player->getMainHandItem();
    not_null_ptr<ItemInstance> offHandItem = player->getOffhandItem();
    float attackStrength = (float)player->getAttackStrengthScale(1.0f);
    float mainHandTarget = ItemInstance::matches(mMainHandItem, mainHandItem, compareTags)
                               ? attackStrength * attackStrength * attackStrength
                               : 0.0f;
    mMainHandHeight += Mth::clamp(mainHandTarget - mMainHandHeight, -0.4f, 0.4f);
    float offHandTarget = ItemInstance::matches(mOffHandItem, offHandItem, compareTags) ? 1.0f : 0.0f;
    mOffHandHeight += Mth::clamp(offHandTarget - mOffHandHeight, -0.4f, 0.4f);
}

void ItemInHandRenderer::tick()
{
    mMainHandHeightOld = mMainHandHeight;
    mOffHandHeightOld = mOffHandHeight;
    boost::shared_ptr<LocalPlayer> player = mMinecraft->mPlayer;
    not_null_ptr<ItemInstance> mainHandItem = player->getMainHandItem();
    not_null_ptr<ItemInstance> offHandItem = player->getOffhandItem();
    if (!player->isHandsBusy())
    {
        bool bothMaps = mMainHandItem->getItem() == Items::filledMap && mainHandItem->getItem() == Items::filledMap;
        int aux = mMainHandItem->getAuxValue();
        bool specialMapAux = aux == 0xFF || aux == 0xFE || aux == 0xFD;
        if (bothMaps && specialMapAux)
        {
            CalculateNewHandHeights(false);
            mMainHandItem = mainHandItem;
            mOffHandItem = offHandItem;
        }
        else
        {
            CalculateNewHandHeights(true);
        }
    }
    else
    {
        mMainHandHeight = Mth::clamp(mMainHandHeight - 0.4f, 0.0f, 1.0f);
        mOffHandHeight = Mth::clamp(mOffHandHeight - 0.4f, 0.0f, 1.0f);
    }
    if (mMainHandHeight <= 0.1f)
    {
        mMainHandItem = mainHandItem;
    }
    if (mOffHandHeight < 0.1f)
    {
        mOffHandItem = offHandItem;
    }
}

void ItemInHandRenderer::itemUsed(InteractionHand::EInteractionHand hand)
{
    if (hand == InteractionHand::eInteractionHand_MainHand)
    {
        mMainHandHeight = 0.0f;
    }
    else
    {
        mOffHandHeight = 0.0f;
    }
}
