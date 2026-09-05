#include "game/client/render/ItemInHandRenderer.hpp"

#include <cmath>

#include "game/client/render/Tesselator.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

ItemInHandRenderer::ItemInHandRenderer(Minecraft* minecraft)
    : mMinecraft(minecraft), mMainHandItem(), mOffHandItem(), mMainHandHeight(0.0f), mOffHandHeight(0.0f),
      mMainHandHeightOld(0.0f), mOffHandHeightOld(0.0f), mEquipProgress(0.0f), mEntityRenderDispatcher(0), mListId(-1)
{
}

unsigned long long ItemInHandRenderer::renderItem3D(Tesselator* tesselator, float x0, float y0, float x1, float y1, int width, int height, float alpha, eItemRenderMode mode)
{
    GlStateManager::matrixMode(5889);
    GlStateManager::loadIdentity();
    GlStateManager::translatef(x0, y0, 0.0f);
    if (mode == ITEM_RENDER_MODE_3D)
    {
        GlStateManager::callList(mListId);
    }
    GlStateManager::matrixMode(5888);
    return 0;
}

void ItemInHandRenderer::Reset()
{
    mMainHandItem.reset();
    mOffHandItem.reset();
    mMainHandHeight = 1.0f;
    mOffHandHeight = 1.0f;
    mMainHandHeightOld = 1.0f;
    mOffHandHeightOld = 1.0f;
    mEquipProgress = 0.0f;
    mListId = -1;
}

void ItemInHandRenderer::renderItem(boost::shared_ptr<LivingEntity> entity, not_null_ptr<ItemInstance> item, ItemTransforms::TransformType transform, bool leftHand, int light, bool foil)
{
    if (!item || item->isEmpty()) return;
    GlStateManager::pushMatrix();
    if (leftHand) GlStateManager::rotatef(180.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(180.0f, 0.0f, 0.0f, 1.0f);
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
    if (!player) return;
    GlStateManager::multiTexCoord2f(0.0f, 0.0f);
}

void ItemInHandRenderer::setPlayerBob(boost::shared_ptr<LocalPlayer> player, float partialTick)
{
    if (!player) return;
    GlStateManager::rotatef(partialTick, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(partialTick, 0.0f, 1.0f, 0.0f);
}

float ItemInHandRenderer::calculateMapTilt(float value)
{
    return Mth::clamp(value, -1.0f, 1.0f);
}

void ItemInHandRenderer::renderMapHand(const HumanoidArm* arm) { renderPlayerArm(0.0f, 0.0f, arm); }
void ItemInHandRenderer::renderMapHands() { renderMapHand(HumanoidArm::LEFT); renderMapHand(HumanoidArm::RIGHT); }
void ItemInHandRenderer::renderPlayerArm(float equip, float swing, const HumanoidArm*) { GlStateManager::translatef(equip, swing, 0.0f); }
void ItemInHandRenderer::renderMap(not_null_ptr<ItemInstance> item, int hand) { renderItem(boost::shared_ptr<LivingEntity>(), item, ItemTransforms::FIRST_PERSON_RIGHT_HAND, hand != 0, 0, false); }
void ItemInHandRenderer::renderOneHandedMap(float equip, const HumanoidArm* arm, float swing,
                                            const not_null_ptr<ItemInstance>& item)
{
    renderPlayerArm(equip, swing, arm);
    renderMap(item, arm == HumanoidArm::LEFT ? 0 : 1);
}
void ItemInHandRenderer::renderTwoHandedMap(float equip, float swing, float tilt) { GlStateManager::rotatef(tilt, 0.0f, 0.0f, 1.0f); renderMapHands(); }
void ItemInHandRenderer::applyItemAttackTransform(float value) { applyItemArmAttackTransform(HumanoidArm::RIGHT, value); }
void ItemInHandRenderer::applyEatTransform(float value, const HumanoidArm* arm, not_null_ptr<ItemInstance>) { applyItemArmTransform(arm, value); }
void ItemInHandRenderer::applyItemArmAttackTransform(const HumanoidArm* arm, float value)
{
    GlStateManager::rotatef(Mth::sin(value * value * 3.1415927f) * 45.0f,
                            1.0f, 0.0f, 0.0f);
    applyItemArmTransform(arm, value);
}
void ItemInHandRenderer::applyItemArmTransform(const HumanoidArm* arm, float value) { GlStateManager::translatef(arm == HumanoidArm::LEFT ? -0.56f : 0.56f, value * -0.4f, -0.72f); }
void ItemInHandRenderer::renderArmWithItem(boost::shared_ptr<LocalPlayer> player, float equip,
                                           InteractionHand::EInteractionHand hand, float swing,
                                           not_null_ptr<ItemInstance> item, bool leftHand)
{
    if (player && item)
    {
        renderItem(player, item,
                   leftHand ? ItemTransforms::FIRST_PERSON_LEFT_HAND : ItemTransforms::FIRST_PERSON_RIGHT_HAND,
                   leftHand, 0, false);
    }
}
void ItemInHandRenderer::render(float partialTick) { CalculateNewHandHeights(false); }
void ItemInHandRenderer::renderTex(TextureAtlasSprite* sprite) { if (!sprite) return; Tesselator::getInstance()->getBuilder()->begin(); Tesselator::getInstance()->end(); }
void ItemInHandRenderer::renderWater(float partialTick) { GlStateManager::enableBlend(); GlStateManager::disableBlend(); }
void ItemInHandRenderer::renderFire() { GlStateManager::enableBlend(); GlStateManager::disableBlend(); }
void ItemInHandRenderer::renderScreenEffect(float partialTick) { renderWater(partialTick); }
void ItemInHandRenderer::CalculateNewHandHeights(bool instant) { if (instant) { mMainHandHeight = mMainHandHeightOld; mOffHandHeight = mOffHandHeightOld; } }
void ItemInHandRenderer::tick() { mMainHandHeightOld = mMainHandHeight; mOffHandHeightOld = mOffHandHeight; }
void ItemInHandRenderer::itemUsed(InteractionHand::EInteractionHand hand) { if (hand == InteractionHand::eInteractionHand_MainHand) mMainHandHeightOld = 0.0f; else mOffHandHeightOld = 0.0f; }
