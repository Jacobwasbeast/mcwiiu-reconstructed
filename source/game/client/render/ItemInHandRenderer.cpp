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
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

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
    if (item->isEmpty())
    {
        return;
    }
    Block* block = Block::byItem(item->getItem());
    GlStateManager::pushMatrix();
    if (Block::getId(block) == 218)
    {
        if (mMinecraft->mPlayer->GetThirdPersonView(false) == 0)
        {
            GlStateManager::rotatef(-90.0f, 0.0f, 1.0f, 0.0f);
        }
    }
    bool wasGui3d = mItemRenderer->mIsGui3d;
    mItemRenderer->mIsGui3d = gui3d;
    mItemRenderer->renderStatic(item, entity, transform, 1.0f, leftHand, 0);
    mItemRenderer->mIsGui3d = wasGui3d;
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
    if (!player) return;
    GlStateManager::rotatef(partialTick, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(partialTick, 0.0f, 1.0f, 0.0f);
}

float ItemInHandRenderer::calculateMapTilt(float xRot)
{
    float tilt = Mth::clamp(1.1f - xRot / 45.0f, 0.0f, 1.0f);
    return -std::cos(tilt * 3.1415927f) * 0.5f + 0.5f;
}

void ItemInHandRenderer::renderMapHand(const HumanoidArm* arm)
{
    renderPlayerArm(0.0f, 0.0f, arm);
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
