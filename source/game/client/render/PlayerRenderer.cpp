#include "game/client/render/PlayerRenderer.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/ElytraModel.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/PlayerModel.hpp"
#include "game/client/render/PlayerModelPart.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/ElytraLayer.hpp"
#include "game/world/generation/ArrowLayer.hpp"
#include "game/world/generation/CapeLayer.hpp"
#include "game/world/generation/CustomHeadLayer.hpp"
#include "game/world/generation/Deadmau5EarsLayer.hpp"
#include "game/world/generation/EntityRidingShoulderLayer.hpp"
#include "game/world/generation/HumanoidArmorLayer.hpp"
#include "game/world/generation/ItemInHandLayer.hpp"
#include "game/world/generation/SpinAttackLayer.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/minigame/EMiniGameId.hpp"

PlayerRenderer::PlayerRenderer(EntityRenderDispatcher* dispatcher, bool slimArms, bool is64x64)
    : LivingEntityRenderer(dispatcher, new PlayerModel(0.0f, slimArms, is64x64), 0.5f),
      mSlimArms(slimArms), mIs64x64(is64x64)
{
    _init();
}

PlayerRenderer::~PlayerRenderer()
{
}

void PlayerRenderer::_init()
{
    addLayer(new HumanoidArmorLayer(this));
    addLayer(new ItemInHandLayer(this));
    addLayer(new ArrowLayer(this));
    addLayer(new Deadmau5EarsLayer(this));
    addLayer(new CapeLayer(this));
    addLayer(new CustomHeadLayer(static_cast<PlayerModel*>(getModel())->mHead));
    addLayer(new EntityRidingShoulderLayer(mDispatcher));
    addLayer(new SpinAttackLayer(this));
    mElytraLayer = new ElytraLayer(this);
    addLayer(mElytraLayer);
}

unsigned int PlayerRenderer::getNametagColour(int colour)
{
    static const unsigned int colours[8] = { 0xff000000, 0xff33cc33, 0xffcc3333, 0xff3333cc,
                                             0xffcc33cc, 0xffcc6633, 0xffcccc33, 0xff33dccc };
    return static_cast<unsigned int>(colour) < 8 ? colours[colour] : 0xff000000;
}

Model* PlayerRenderer::getModel()
{
    return LivingEntityRenderer::getModel();
}

ElytraModel* PlayerRenderer::getElytraModel()
{
    return mElytraLayer->getElytraModel();
}

void PlayerRenderer::setModelProperties(boost::shared_ptr<Player> player, float partialTicks)
{
    PlayerModel* model = static_cast<PlayerModel*>(getModel());
    model->mSkinAdjustments = player->getSkinAdjustments();
    model->mSitting = false;
    if (player->isSpectator()
        && (player->SpectateAsFloatingHead() || player->SpectateAsInvisible()
            || player->IsSpectatingOtherPlayer()))
    {
        model->setAllVisible(false);
        if (player->SpectateAsFloatingHead())
        {
            model->mHead->mShow = true;
            model->mHat->mShow = true;
        }
    }
    else
    {
        not_null_ptr<ItemInstance> mainHandItem = player->getMainHandItem();
        not_null_ptr<ItemInstance> offhandItem = player->getOffhandItem();
        model->setAllVisible(true);
        model->mHat->mShow = player->isModelPartShown(PlayerModelPart::HAT);
        if (mIs64x64)
        {
            model->mJacket->mShow = player->isModelPartShown(PlayerModelPart::JACKET);
            model->mLeftPants->mShow = player->isModelPartShown(PlayerModelPart::LEFT_PANTS_LEG);
            model->mRightPants->mShow = player->isModelPartShown(PlayerModelPart::RIGHT_PANTS_LEG);
            model->mLeftSleeve->mShow = player->isModelPartShown(PlayerModelPart::LEFT_SLEEVE);
            model->mRightSleeve->mShow = player->isModelPartShown(PlayerModelPart::RIGHT_SLEEVE);
        }
        model->mCrouching = player->isSneaking();
        model->mSwimming = player->getDamageNearbyMobs();
        model->mEating = false;
        HumanoidModel::eArmPose mainHandPose = HumanoidModel::eArmPose_Empty;
        HumanoidModel::eArmPose offhandPose = HumanoidModel::eArmPose_Empty;
        if (!mainHandItem->isEmpty())
        {
            mainHandPose = HumanoidModel::eArmPose_Item;
            if (player->getUseItemRemainingTicks() > 0)
            {
                int useAnimation = mainHandItem->getUseAnimation();
                if (useAnimation == UseAnim_block)
                {
                    mainHandPose = HumanoidModel::eArmPose_Block;
                }
                else if (useAnimation == UseAnim_bow)
                {
                    mainHandPose = HumanoidModel::eArmPose_BowAndArrow;
                }
                else if (useAnimation == UseAnim_spear)
                {
                    mainHandPose = HumanoidModel::eArmPose_ThrowSpear;
                    model->mSpearThrowProgress = Mth::clamp(
                        (float)(mainHandItem->getUseDuration() - player->getUseItemRemainingTicks()) / 10.0f,
                        0.0f, 1.0f);
                }
                else if (useAnimation == UseAnim_eat || useAnimation == UseAnim_drink)
                {
                    float remainingTicks =
                        (float)player->getUseItemRemainingTicks() - partialTicks + 1.0f;
                    float useDuration = (float)mainHandItem->getUseDuration();
                    model->mEating = true;
                    model->mUseItemRemainingTicks = remainingTicks;
                    model->mUseItemProgress = 1.0f - remainingTicks / useDuration;
                }
            }
        }
        if (!offhandItem->isEmpty())
        {
            offhandPose = HumanoidModel::eArmPose_Item;
            if (player->getUseItemRemainingTicks() > 0)
            {
                if (offhandItem->getUseAnimation() == UseAnim_block)
                {
                    offhandPose = HumanoidModel::eArmPose_Block;
                }
            }
        }
        if (player->getMainArm() == HumanoidArm::RIGHT)
        {
            model->mRightArmPose = mainHandPose;
            model->mLeftArmPose = offhandPose;
        }
        else
        {
            model->mRightArmPose = offhandPose;
            model->mLeftArmPose = mainHandPose;
        }
    }
    if (player->getItemSlot(EquipmentSlot::HEAD)->getItem() == Items::skull)
    {
        model->mHead->mShow = false;
        model->mHat->mShow = false;
    }
    if ((player->getSkinAdjustments().mAnimFlags & 8) != 0)
    {
        model->mSitting = player->isIdle();
    }
}

void PlayerRenderer::hideAdditionalModelParts(boost::shared_ptr<Player> player)
{
    std::vector<ModelPart*>* parts = player->GetAdditionalModelParts();
    if (parts == NULL || parts->empty())
    {
        return;
    }
    std::vector<ModelPart*>::iterator part;
    for (part = parts->begin(); part != parts->end(); ++part)
    {
        (*part)->mShow = false;
    }
}

void PlayerRenderer::updatePartVisibility(ModelPart* part, boost::shared_ptr<Player> player, bool visible)
{
    if (player->isSpectator() && player->SpectateAsFloatingHead()
        && part->mSkinBoxType != 1 && part->mSkinBoxType != 7)
    {
        part->mShow = false;
        return;
    }

    bool coveredHead = false;
    if (visible && part->mArmourFlags != 0)
    {
        if ((part->mArmourFlags & 1) != 0 && !player->getItemSlot(EquipmentSlot::HEAD)->isEmpty())
        {
            visible = false;
        }
        if (visible && (part->mArmourFlags & 2) != 0
            && !player->getItemSlot(EquipmentSlot::CHEST)->isEmpty())
        {
            visible = false;
        }
        if (visible && (part->mArmourFlags & 4) != 0
            && !player->getItemSlot(EquipmentSlot::LEGS)->isEmpty())
        {
            visible = false;
        }
        if (visible && (part->mArmourFlags & 8) != 0
            && !player->getItemSlot(EquipmentSlot::FEET)->isEmpty())
        {
            visible = false;
        }
    }
    else if (visible && (part->mSkinBoxType == 1 || part->mSkinBoxType == 7))
    {
        if (!player->getItemSlot(EquipmentSlot::HEAD)->isEmpty()
            && (player->getItemSlot(EquipmentSlot::HEAD)->getItem() == Items::skull
                || player->getItemSlot(EquipmentSlot::HEAD)->getItem()
                       == Item::byBlock(Blocks::carved_pumpkin)))
        {
            coveredHead = true;
        }
    }
    part->mShow = coveredHead ? false : visible;
}

void PlayerRenderer::showAdditionalModelParts(boost::shared_ptr<Player> player)
{
    std::vector<ModelPart*>* parts = player->GetAdditionalModelParts();
    if (parts == NULL)
    {
        return;
    }
    for (std::vector<ModelPart*>::iterator part = parts->begin(); part != parts->end(); ++part)
    {
        ModelPart* modelPart = *part;
        updatePartVisibility(modelPart, player, true);
    }
}

void PlayerRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    if (player->isLocalPlayer() && boost::static_pointer_cast<LocalPlayer>(player)->IsSpectatingOtherPlayer())
    {
        return;
    }
    if (player->hasInvisiblePrivilege())
    {
        return;
    }

    bool fallFlying = false;
    if (player)
    {
        fallFlying = (player->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY
            && boost::static_pointer_cast<LivingEntity>(player)->getFallFlyingTicks() > 4;
    }

    double renderY = y;
    if (player->isSneaking() && !fallFlying)
    {
        renderY -= 0.125;
    }

    setModelProperties(player, partialTick);

    if (player->isSpectator() && (player->SpectateAsInvisible() || player->IsSpectatingOtherPlayer()))
    {
        hideAdditionalModelParts(player);
    }
    else
    {
        showAdditionalModelParts(player);
    }
    GlStateManager::setProfile();

    bool suppressSpecialRender = false;
    if (player && (player->GetType() & EntityType::PLAYER) == EntityType::PLAYER)
    {
        boost::shared_ptr<Player> localPlayer = Minecraft::GetInstance()->mPlayer;
        if (localPlayer->GetThirdPersonView(false) != 0
            && (Minecraft::GetInstance()->mPlayer.get() == player.get() || localPlayer->IsSpectatingOtherPlayer()))
        {
            suppressSpecialRender = true;
        }
    }

    mSpecialRenderActive = player->isFallFlying() && Minecraft::InMiniGame(eMiniGameId_Glide, true)
        && !Minecraft::GetInstance()->getEntityRenderDispatcher()->mRenderName && !suppressSpecialRender;
    if (mSpecialRenderActive)
    {
        mSpecialRenderAlpha = Mth::clamp((float)(x * x + y * y + z * z) * 0.5f, 0.0f, 1.0f);
        GlStateManager::pushMatrix();
        GlStateManager::enableBlend();
        GlStateManager::blendFunc(4, 5);
        GlStateManager::color4f(1.0f, 1.0f, 1.0f, mSpecialRenderAlpha);
    }

    LivingEntityRenderer::render(entity, x, renderY, z, yaw, partialTick);

    if (mSpecialRenderActive)
    {
        GlStateManager::popMatrix();
    }

    GlStateManager::unsetProfile();
    hideAdditionalModelParts(player);
}

const ResourceLocation* PlayerRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    return new ResourceLocation(static_cast<_TEXTURE_NAME>(player->getTexture()));
}

void PlayerRenderer::translateWeaponItem()
{
    GlStateManager::translatef(0.0f, 0.1875f, 0.0f);
}

void PlayerRenderer::renderNameTags(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                    const std::wstring& name, double distance, int colour)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    EntityRenderer::renderNameTags(entity, x, y, z, name, distance,
                                   getNametagColour(player->mPlayerIndex));
}

void PlayerRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    GlStateManager::scalef(0.9375f, 0.9375f, 0.9375f);
}

void PlayerRenderer::renderRightHand(boost::shared_ptr<Player> player)
{
    GlStateManager::color3f(1.0f, 1.0f, 1.0f);
    PlayerModel* model = static_cast<PlayerModel*>(getModel());
    setModelProperties(player, 0.0f);
    model->mSitting = false;
    showAdditionalModelParts(player);
    model->mAttackTime = 0.0f;
    model->mRiding = false;
    model->mCrouching = false;
    model->setupAnim(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0625f, player, _SkinAdjustments());
    model->mRightArm->mXRot = 0.0f;
    model->mRightArm->render(0.0625f, true,
                             model->IsBodyPartDisabled(HumanoidModel::eAnimBit_RightArm));
    if (mIs64x64)
    {
        model->mRightSleeve->mXRot = 0.0f;
        model->mRightSleeve->render(0.0625f, true,
                                    model->IsBodyPartDisabled(HumanoidModel::eAnimBit_RightSleeve));
    }
    hideAdditionalModelParts(player);
}

void PlayerRenderer::renderLeftHand(boost::shared_ptr<Player> player)
{
    GlStateManager::color3f(1.0f, 1.0f, 1.0f);
    PlayerModel* model = static_cast<PlayerModel*>(getModel());
    setModelProperties(player, 0.0f);
    model->mSitting = false;
    showAdditionalModelParts(player);
    model->mAttackTime = 0.0f;
    model->mRiding = false;
    model->mCrouching = false;
    model->setupAnim(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0625f, player, _SkinAdjustments());
    model->mLeftArm->mXRot = 0.0f;
    model->mLeftArm->render(0.0625f, true,
                            model->IsBodyPartDisabled(HumanoidModel::eAnimBit_LeftArm));
    if (mIs64x64)
    {
        model->mLeftSleeve->mXRot = 0.0f;
        model->mLeftSleeve->render(0.0625f, true,
                                   model->IsBodyPartDisabled(HumanoidModel::eAnimBit_LeftSleeve));
    }
    hideAdditionalModelParts(player);
}

void PlayerRenderer::setupPosition(boost::shared_ptr<LivingEntity>& entity, double x, double y, double z)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    if (player->isAlive() && player->isSleeping())
    {
        LivingEntityRenderer::setupPosition(entity, x + player->mBedOffsetX, y + player->mBedOffsetY,
                                            z + player->mBedOffsetZ);
    }
    else
    {
        if (player->isPassenger())
        {
            if ((player->getSkinAdjustments().mAnimFlags & (1 << HumanoidModel::eAnimBit_Seated)) == 0)
            {
                if (player->getSkinAdjustments().mHeadY > 0.0f)
                {
                    y += (double)(player->getSkinAdjustments().mHeadY * 0.0625f);
                }
            }
            else
            {
                y += 0.5;
            }
        }
        LivingEntityRenderer::setupPosition(entity, x, y, z);
    }
}

void PlayerRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float rotationYaw,
                                    float partialTicks)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    if (!mDispatcher->mRenderLeash && player->isAlive() && player->isSleeping())
    {
        GlStateManager::rotatef(player->getSleepRotation(), 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(getFlipDegrees(entity), 0.0f, 0.0f, 1.0f);
        GlStateManager::rotatef(270.0f, 0.0f, 1.0f, 0.0f);
    }
    else if (!mDispatcher->mRenderLeash && player->isFallFlying())
    {
        LivingEntityRenderer::setupRotations(entity, ageInTicks, rotationYaw, partialTicks);

        float fallFlyingTicks = (float)player->getFallFlyingTicks() + partialTicks;
        float fallFlyingProgress = Mth::clamp((fallFlyingTicks * fallFlyingTicks) / 100.0f, 0.0f, 1.0f);
        float interpolatedXRot = player->mXRotO + (player->mXRot - player->mXRotO) * partialTicks;
        GlStateManager::rotatef(fallFlyingProgress * (-90.0f - interpolatedXRot), 1.0f, 0.0f, 0.0f);

        if (player->isLocalPlayer())
        {
            boost::shared_ptr<LocalPlayer> localPlayer = boost::static_pointer_cast<LocalPlayer>(player);
            float fallFlyYaw = localPlayer->mFallFlyYawOld
                + (localPlayer->mFallFlyYaw - localPlayer->mFallFlyYawOld) * partialTicks;
            GlStateManager::rotatef(fallFlyYaw, 0.0f, 1.0f, 0.0f);
        }
        else
        {
            Vec3* viewVector = player->getViewVector(partialTicks);
            double horizontalMotionLengthSqr = player->mXd * player->mXd + player->mZd * player->mZd;
            double horizontalViewLengthSqr = viewVector->x * viewVector->x + viewVector->z * viewVector->z;
            if (horizontalMotionLengthSqr > 0.0 && horizontalViewLengthSqr > 0.0)
            {
                double cosine = Mth::clamp((player->mXd * viewVector->x + player->mZd * viewVector->z)
                                                / (double)(float)(Mth::sqrt(horizontalMotionLengthSqr)
                                                                  * Mth::sqrt(horizontalViewLengthSqr)),
                                            -1.0, 1.0);
                double cross = player->mXd * viewVector->z - player->mZd * viewVector->x;
                double direction = (double)((cross > 0.0) - (cross < 0.0));
                GlStateManager::rotatef((float)(direction * std::acos(cosine)) * 180.0f / 3.1415927f,
                                        0.0f, 1.0f, 0.0f);
            }
        }
    }
    else if (player->getDamageNearbyMobs())
    {
        LivingEntityRenderer::setupRotations(entity, ageInTicks, rotationYaw, partialTicks);
        GlStateManager::rotatef(-90.0f - player->mXRot, 1.0f, 0.0f, 0.0f);
        GlStateManager::translatef(0.0f, -1.0f, 0.5f);
        GlStateManager::rotatef((player->mAutoSpinAttackTicks + partialTicks) * -75.0f,
                                0.0f, 1.0f, 0.0f);
    }
    else
    {
        LivingEntityRenderer::setupRotations(entity, ageInTicks, rotationYaw, partialTicks);
    }
}

void PlayerRenderer::renderShadow(boost::shared_ptr<Entity>& entity, double x, double y, double z, float alpha,
                                  float partialTicks)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    if (player->isSpectator() && player->SpectateAsInvisible())
    {
        return;
    }
    if (gApp.GetGameHostOption(eGameHostOption_HostCanBeInvisible) && player && player->hasInvisiblePrivilege())
    {
        return;
    }
    EntityRenderer::renderShadow(entity, x, y, z, alpha, partialTicks);
}

bool PlayerRenderer::bindTexture(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    return bindTexture(player->mSkinLocation, player->getTexture());
}

void PlayerRenderer::renderLayers(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                  float partialTick, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                                  bool overlay)
{
    if (mDispatcher->mRenderLeash)
    {
        mElytraLayer->getElytraModel()->mOverrideAnim = true;
    }
    LivingEntityRenderer::renderLayers(entity, limbSwing, limbSwingAmount, partialTick, ageInTicks, netHeadYaw,
                                        headPitch, scale, overlay);
    if (mDispatcher->mRenderLeash)
    {
        mElytraLayer->getElytraModel()->mOverrideAnim = false;
    }
}
