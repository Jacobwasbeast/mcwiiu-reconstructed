#include "game/client/render/ItemInHandRenderer.hpp"
#include <cmath>
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/Block.hpp"

ItemInHandRenderer::ItemInHandRenderer(Minecraft* minecraft)
    : mMinecraft(minecraft), mMainHandItem(), mOffHandItem(), mMainHandHeight(0.0f), mOffHandHeight(0.0f),
      mMainHandHeightOld(0.0f), mOffHandHeightOld(0.0f),
      mEntityRenderDispatcher(minecraft->getEntityRenderDispatcher()), mItemRenderer(minecraft->getItemRenderer()), mListId(-1)
{
}

unsigned long long ItemInHandRenderer::renderItem3D(Tesselator*, float x, float y, float, float, int, int, float, eItemRenderMode mode)
{
    GlStateManager::matrixMode(5889);
    GlStateManager::loadIdentity();
    GlStateManager::translatef(x, y, 0.0f);
    if (mode == ITEM_RENDER_MODE_3D) GlStateManager::callList(mListId);
    GlStateManager::matrixMode(5888);
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
    if (!item || item->isEmpty()) return;
    Block* block = Block::byItem(item->getItem());
    GlStateManager::pushMatrix();
    if (Block::getId(block) == 218) GlStateManager::rotatef(-90.0f, 0.0f, 1.0f, 0.0f);
    bool previousGui3d = mItemRenderer->mIsGui3d;
    mItemRenderer->mIsGui3d = gui3d;
    mItemRenderer->renderStatic(item, entity, transform, 1.0f, leftHand, 0);
    mItemRenderer->mIsGui3d = previousGui3d;
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
    if (player) GlStateManager::multiTexCoord2f(0.0f, 0.0f);
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

void ItemInHandRenderer::renderMapHand(const HumanoidArm* arm)
{
    renderPlayerArm(0.0f, 0.0f, arm);
}

void ItemInHandRenderer::renderMapHands()
{
    GlStateManager::disableCull();
    renderMapHand(HumanoidArm::RIGHT);
    renderMapHand(HumanoidArm::LEFT);
    GlStateManager::enableCull();
}

void ItemInHandRenderer::renderPlayerArm(float equip, float swing, const HumanoidArm* arm)
{
    GlStateManager::translatef(equip, swing, 0.0f);
    if (arm == HumanoidArm::LEFT) GlStateManager::scalef(-1.0f, 1.0f, 1.0f);
}

void ItemInHandRenderer::renderMap(not_null_ptr<ItemInstance> item, int hand)
{
    renderItem(boost::shared_ptr<LivingEntity>(), item, ItemTransforms::FIRST_PERSON_RIGHT_HAND, hand != 0, 0, false);
}

void ItemInHandRenderer::renderOneHandedMap(float equip, const HumanoidArm* arm, float swing,
                                            const not_null_ptr<ItemInstance>& item)
{
    GlStateManager::pushMatrix();
    renderPlayerArm(equip, swing, arm);
    renderMap(item, arm == HumanoidArm::LEFT ? 0 : 1);
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

void ItemInHandRenderer::applyEatTransform(float value, const HumanoidArm* arm, not_null_ptr<ItemInstance> item)
{
    float remaining = (float)item->getUseDuration() - value + 1.0f;
    float useProgress = remaining / (float)item->getUseDuration();
    float bob = Mth::cos(remaining * 0.25f * 3.1415927f);
    float y = useProgress < 0.8f ? std::fabs(bob * 0.1f) : 0.0f;
    GlStateManager::translatef(0.0f, y, 0.0f);
    float pull = 1.0f - std::pow(useProgress, 27.0f);
    float side = arm == HumanoidArm::RIGHT ? 1.0f : -1.0f;
    GlStateManager::translatef(pull * 0.6f * side, pull * -0.5f, 0.0f);
    GlStateManager::rotatef(side * pull * 90.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(pull * 10.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(side * pull * 30.0f, 0.0f, 0.0f, 1.0f);
}

void ItemInHandRenderer::applyItemArmAttackTransform(const HumanoidArm* arm, float value)
{
    float side = arm == HumanoidArm::LEFT ? -1.0f : 1.0f;
    float swing = Mth::sin(value * value * 3.1415927f);
    float recovery = Mth::sin(Mth::sqrt(value) * 3.1415927f);
    GlStateManager::rotatef(side * (swing * 20.0f + 45.0f), 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(side * recovery * 20.0f, 0.0f, 0.0f, 1.0f);
    GlStateManager::rotatef(recovery * -20.0f, 0.0f, 1.0f, 0.0f);
}

void ItemInHandRenderer::applyItemArmTransform(const HumanoidArm* arm, float value)
{
    float side = arm == HumanoidArm::LEFT ? -1.0f : 1.0f;
    GlStateManager::translatef(side * 0.56f, value * -0.4f, -0.72f);
}

void ItemInHandRenderer::renderArmWithItem(boost::shared_ptr<LocalPlayer> player, float equip,
                                           InteractionHand::EInteractionHand hand, float swing,
                                           not_null_ptr<ItemInstance> item, bool leftHand)
{
    const HumanoidArm* arm = leftHand ? HumanoidArm::LEFT : HumanoidArm::RIGHT;
    if (hand == InteractionHand::eInteractionHand_OffHand) arm = arm->getOpposite();
    GlStateManager::pushMatrix();
    if (!item || item->isEmpty())
    {
        if (player && hand == InteractionHand::eInteractionHand_MainHand) renderPlayerArm(equip, swing, arm);
    }
    else
    {
        int animation = item->getUseAnimation();
        if (animation == 1 || animation == 2) applyEatTransform(equip, arm, item);
        applyItemArmTransform(arm, equip);
        applyItemArmAttackTransform(arm, swing);
        renderItem(boost::shared_ptr<LivingEntity>(), item,
                   leftHand ? ItemTransforms::FIRST_PERSON_LEFT_HAND : ItemTransforms::FIRST_PERSON_RIGHT_HAND,
                   leftHand, 0, true);
    }
    GlStateManager::popMatrix();
}

void ItemInHandRenderer::render(float partialTick)
{
    float mainHandHeight = mMainHandHeightOld + (mMainHandHeight - mMainHandHeightOld) * partialTick;
    float offHandHeight = mOffHandHeightOld + (mOffHandHeight - mOffHandHeightOld) * partialTick;
    enableLight(mainHandHeight, offHandHeight);
    GlStateManager::enableRescaleNormal();
    renderArmWithItem(boost::shared_ptr<LocalPlayer>(), partialTick, InteractionHand::eInteractionHand_MainHand,
                      1.0f - mainHandHeight, mMainHandItem, false);
    renderArmWithItem(boost::shared_ptr<LocalPlayer>(), partialTick, InteractionHand::eInteractionHand_OffHand,
                      1.0f - offHandHeight, mOffHandItem, true);
    GlStateManager::disableRescaleNormal();
    Lighting::turnOff();
}

void ItemInHandRenderer::renderTex(TextureAtlasSprite* sprite)
{
    if (!sprite) return;
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->begin();
    builder->vertexUV(0.0f, 0.0f, 0.0f, (float)sprite->getU0(false), (float)sprite->getV0(false));
    builder->vertexUV(1.0f, 0.0f, 0.0f, (float)sprite->getU1(false), (float)sprite->getV0(false));
    builder->vertexUV(1.0f, 1.0f, 0.0f, (float)sprite->getU1(false), (float)sprite->getV1(false));
    builder->vertexUV(0.0f, 1.0f, 0.0f, (float)sprite->getU0(false), (float)sprite->getV1(false));
    Tesselator::getInstance()->end();
}

void ItemInHandRenderer::renderWater(float)
{
    GlStateManager::enableBlend();
    GlStateManager::color4f(0.8f, 0.8f, 0.8f, 0.5f);
    GlStateManager::disableBlend();
}

void ItemInHandRenderer::renderFire()
{
    GlStateManager::enableBlend();
    GlStateManager::disableBlend();
}

void ItemInHandRenderer::renderScreenEffect(float partialTick)
{
    renderWater(partialTick);
}

void ItemInHandRenderer::CalculateNewHandHeights(bool instant)
{
    if (instant)
    {
        mMainHandHeight = mMainHandHeightOld;
        mOffHandHeight = mOffHandHeightOld;
    }
    else
    {
        mMainHandHeight += (mMainHandHeightOld - mMainHandHeight) * 0.4f;
        mOffHandHeight += (mOffHandHeightOld - mOffHandHeight) * 0.4f;
    }
}

void ItemInHandRenderer::tick()
{
    mMainHandHeightOld = mMainHandHeight;
    mOffHandHeightOld = mOffHandHeight;
}

void ItemInHandRenderer::itemUsed(InteractionHand::EInteractionHand hand)
{
    if (hand == InteractionHand::eInteractionHand_MainHand) mMainHandHeightOld = 0.0f;
    else mOffHandHeightOld = 0.0f;
}
