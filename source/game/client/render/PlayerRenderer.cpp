#include "game/client/render/PlayerRenderer.hpp"

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
#include "game/world/generation/Player.hpp"

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
    addLayer(new CustomHeadLayer(getModel()->mHead));
    addLayer(new EntityRidingShoulderLayer(mDispatcher));
    addLayer(new SpinAttackLayer(this));
    mElytraLayer = new ElytraLayer(this);
    addLayer(mElytraLayer);
}

unsigned int PlayerRenderer::getNametagColour(int colour)
{
    static const unsigned int colours[8] = { 0xff000000, 0xff33cc33, 0xffcc3333, 0xff3333cc,
                                             0xffcc33cc, 0xffcc6633, 0xffcccc33, 0xff33dccc };
    return colour < 8 ? colours[colour] : 0xff000000;
}

PlayerModel* PlayerRenderer::getModel()
{
    return static_cast<PlayerModel*>(LivingEntityRenderer::getModel());
}

ElytraModel* PlayerRenderer::getElytraModel()
{
    return mElytraLayer->getElytraModel();
}

void PlayerRenderer::setModelProperties(boost::shared_ptr<Player> player, float partialTicks)
{
    PlayerModel* model = getModel();
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
        model->mHat->mShow = player->isModelPartShown(PlayerModelPart::HAT) != 0;
        if (mIs64x64)
        {
            model->mJacket->mShow = player->isModelPartShown(PlayerModelPart::JACKET) != 0;
            model->mLeftPants->mShow = player->isModelPartShown(PlayerModelPart::LEFT_PANTS_LEG) != 0;
            model->mRightPants->mShow = player->isModelPartShown(PlayerModelPart::RIGHT_PANTS_LEG) != 0;
            model->mLeftSleeve->mShow = player->isModelPartShown(PlayerModelPart::LEFT_SLEEVE) != 0;
            model->mRightSleeve->mShow = player->isModelPartShown(PlayerModelPart::RIGHT_SLEEVE) != 0;
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
                if (useAnimation == 3)
                {
                    mainHandPose = HumanoidModel::eArmPose_Block;
                }
                else if (useAnimation == 4)
                {
                    mainHandPose = HumanoidModel::eArmPose_BowAndArrow;
                }
                else if (useAnimation == 5)
                {
                    mainHandPose = HumanoidModel::eArmPose_ThrowSpear;
                    model->mSpearThrowProgress = Mth::clamp(
                        (float)(mainHandItem->getUseDuration() - player->getUseItemRemainingTicks()) / 10.0f,
                        0.0f, 1.0f);
                }
                else if (useAnimation == 1 || useAnimation == 2)
                {
                    float remainingTicks =
                        (float)player->getUseItemRemainingTicks() - partialTicks + 1.0f;
                    model->mEating = true;
                    model->mUseItemRemainingTicks = remainingTicks;
                    model->mUseItemProgress =
                        1.0f - remainingTicks / (float)mainHandItem->getUseDuration();
                }
            }
        }
        if (!offhandItem->isEmpty())
        {
            offhandPose = HumanoidModel::eArmPose_Item;
            if (player->getUseItemRemainingTicks() > 0)
            {
                if (offhandItem->getUseAnimation() == 3)
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
    if (parts == NULL)
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
    std::vector<ModelPart*>::iterator part;
    for (part = parts->begin(); part != parts->end(); ++part)
    {
        updatePartVisibility(*part, player, true);
    }
}

void PlayerRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    if (player->isLocalPlayer() && player->IsSpectatingOtherPlayer())
    {
        return;
    }
    if (player->hasInvisiblePrivilege())
    {
        return;
    }

    setModelProperties(player, partialTick);

    if (player->isSpectator())
    {
        if (player->SpectateAsInvisible() || player->IsSpectatingOtherPlayer())
        {
            hideAdditionalModelParts(player);
        }
        else
        {
            showAdditionalModelParts(player);
        }
    }
    else
    {
        showAdditionalModelParts(player);
    }
    GlStateManager::setProfile();

    LivingEntityRenderer::render(entity, x, y, z, yaw, partialTick);

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
    PlayerModel* model = getModel();
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
    PlayerModel* model = getModel();
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
        LivingEntityRenderer::setupPosition(entity, x + player->mField0x6d0, y + player->mField0x6d4,
                                            z + player->mField0x6d8);
        return;
    }
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

void PlayerRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float rotationYaw,
                                    float partialTicks)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    if (player->isAlive() && player->isSleeping())
    {
        GlStateManager::rotatef(player->getSleepRotation(), 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(getFlipDegrees(entity), 0.0f, 0.0f, 1.0f);
        return;
    }
    LivingEntityRenderer::setupRotations(entity, ageInTicks, rotationYaw, partialTicks);
}

void PlayerRenderer::renderShadow(boost::shared_ptr<Entity>& entity, double x, double y, double z, float alpha,
                                  float partialTicks)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    if (player->isSpectator() && player->SpectateAsInvisible())
    {
        return;
    }
    if (!gApp.GetGameHostOption(static_cast<eGameHostOption>(0x12)) || !player->hasInvisiblePrivilege())
    {
        EntityRenderer::renderShadow(entity, x, y, z, alpha, partialTicks);
    }
}

bool PlayerRenderer::bindTexture(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    return EntityRenderer::bindTexture(player->mSkinLocation, player->getTexture());
}

void PlayerRenderer::renderLayers(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                  float ageInTicks, float netHeadYaw, float headPitch, float scale, float partialTick,
                                  bool overlay)
{
    if (mDispatcher->mRenderLeash)
    {
        mElytraLayer->getElytraModel()->mOverrideAnim = true;
    }
    LivingEntityRenderer::renderLayers(entity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch,
                                        scale, partialTick, overlay);
    if (mDispatcher->mRenderLeash)
    {
        mElytraLayer->getElytraModel()->mOverrideAnim = false;
    }
}
