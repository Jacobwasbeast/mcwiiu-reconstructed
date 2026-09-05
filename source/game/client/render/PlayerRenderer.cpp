#include "game/client/render/PlayerRenderer.hpp"

#include <cmath>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/ElytraModel.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/PlayerModel.hpp"
#include "game/client/render/PlayerModelPart.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/ElytraLayer.hpp"
#include "game/world/generation/EntityRidingShoulderLayer.hpp"
#include "game/world/generation/HumanoidArmorLayer.hpp"
#include "game/world/generation/ItemInHandLayer.hpp"
#include "game/world/generation/SpinAttackLayer.hpp"
#include "game/world/generation/Player.hpp"

PlayerRenderer::PlayerRenderer(EntityRenderDispatcher* dispatcher, bool slimArms, bool is64x64)
    : LivingEntityRenderer(dispatcher, new PlayerModel(1.0f, slimArms, is64x64), 0.5f),
      mUnknown(0), mSlimArms(slimArms), mIs64x64(is64x64), mElytraLayer(NULL)
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
    addLayer(new CustomHeadLayer(static_cast<PlayerModel*>(mModel)->mHead));
    addLayer(new EntityRidingShoulderLayer(mDispatcher));
    addLayer(new SpinAttackLayer(this));
    mElytraLayer = new ElytraLayer(this);
    addLayer(mElytraLayer);
}

unsigned int PlayerRenderer::getNametagColour(int colour)
{
    static const unsigned int colours[8] = { 0xff000000, 0xff33cc33, 0xffcc3333, 0xff3333cc,
                                              0xff000000, 0xff000000, 0xff000000, 0xff000000 };
    return colour < 8 ? colours[colour] : 0xff000000;
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
    PlayerModel* model = static_cast<PlayerModel*>(mModel);
    model->mHead->mShow = player->isModelPartShown(PlayerModelPart::HAT);
    if (mIs64x64)
    {
        model->mLeftSleeve->mShow = player->isModelPartShown(PlayerModelPart::LEFT_SLEEVE);
        model->mRightSleeve->mShow = player->isModelPartShown(PlayerModelPart::RIGHT_SLEEVE);
        model->mLeftPants->mShow = player->isModelPartShown(PlayerModelPart::LEFT_PANTS);
        model->mRightPants->mShow = player->isModelPartShown(PlayerModelPart::RIGHT_PANTS);
        model->mJacket->mShow = player->isModelPartShown(PlayerModelPart::JACKET);
    }
    model->mSlimArms = mSlimArms;
    if (player->SpectateAsFloatingHead())
    {
        model->mBody->mShow = false;
        model->mRightArm->mShow = false;
        model->mLeftArm->mShow = false;
        model->mRightLeg->mShow = false;
        model->mLeftLeg->mShow = false;
    }
    else
    {
        model->mBody->mShow = true;
        model->mRightArm->mShow = true;
        model->mLeftArm->mShow = true;
        model->mRightLeg->mShow = true;
        model->mLeftLeg->mShow = true;
    }
    if (player->SpectateAsInvisible() || player->isInvisible())
    {
        model->setAllVisible(false);
    }
}

void PlayerRenderer::hideAdditionalModelParts(boost::shared_ptr<Player> player)
{
    const std::vector<ModelPart*>& parts = player->GetAdditionalModelParts();
    std::vector<ModelPart*>::const_iterator it;
    for (it = parts.begin(); it != parts.end(); ++it)
    {
        (*it)->mShow = false;
    }
}

void PlayerRenderer::updatePartVisibility(ModelPart* part, boost::shared_ptr<Player> player, bool visible)
{
    if (player->SpectateAsFloatingHead() && part->mArmourFlags != 1 && part->mArmourFlags != 7)
    {
        part->mShow = false;
        return;
    }
    part->mShow = visible;
    if (!visible)
    {
        return;
    }
    if ((part->mArmourFlags & 1) != 0 && !player->getItemSlot(EquipmentSlot::HEAD)->isEmpty()) part->mShow = false;
    if ((part->mArmourFlags & 2) != 0 && !player->getItemSlot(EquipmentSlot::CHEST)->isEmpty()) part->mShow = false;
    if ((part->mArmourFlags & 4) != 0 && !player->getItemSlot(EquipmentSlot::LEGS)->isEmpty()) part->mShow = false;
    if ((part->mArmourFlags & 8) != 0 && !player->getItemSlot(EquipmentSlot::FEET)->isEmpty()) part->mShow = false;
}

void PlayerRenderer::showAdditionalModelParts(boost::shared_ptr<Player> player)
{
    const std::vector<ModelPart*>& parts = player->GetAdditionalModelParts();
    std::vector<ModelPart*>::const_iterator it;
    for (it = parts.begin(); it != parts.end(); ++it)
    {
        updatePartVisibility(*it, player, true);
    }
}

void PlayerRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    if (player->isInvisible() && !player->hasInvisiblePrivilege()) return;
    setModelProperties(player, partialTick);
    if (player->SpectateAsInvisible()) hideAdditionalModelParts(player);
    else showAdditionalModelParts(player);
    LivingEntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* PlayerRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    return player->getTextureLocation();
}

void PlayerRenderer::translateWeaponItem()
{
    GlStateManager::translatef(0.1875f, 0.0f, 0.0f);
}

void PlayerRenderer::renderNameTags(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                    const std::wstring& name, double distance, int colour)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    EntityRenderer::renderNameTags(entity, x, y, z, name, distance, getNametagColour(colour));
}

void PlayerRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    GlStateManager::scalef(0.9375f, 0.9375f, 0.9375f);
}

void PlayerRenderer::renderRightHand(boost::shared_ptr<Player> player)
{
    setModelProperties(player, 0.0f);
    hideAdditionalModelParts(player);
    static_cast<PlayerModel*>(mModel)->mRightArm->mShow = true;
    static_cast<PlayerModel*>(mModel)->mRightSleeve->mShow = true;
    static_cast<PlayerModel*>(mModel)->mRightArm->render(0.0625f, false, false);
    showAdditionalModelParts(player);
}

void PlayerRenderer::renderLeftHand(boost::shared_ptr<Player> player)
{
    setModelProperties(player, 0.0f);
    hideAdditionalModelParts(player);
    static_cast<PlayerModel*>(mModel)->mLeftArm->mShow = true;
    static_cast<PlayerModel*>(mModel)->mLeftSleeve->mShow = true;
    static_cast<PlayerModel*>(mModel)->mLeftArm->render(0.0625f, false, false);
    showAdditionalModelParts(player);
}

void PlayerRenderer::setupPosition(boost::shared_ptr<LivingEntity>& entity, double x, double y, double z)
{
    LivingEntityRenderer::setupPosition(entity, x, y, z);
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    if (player->getSkinAdjustments().mHeadY != 0.0f)
        GlStateManager::translatef(0.0f, player->getSkinAdjustments().mHeadY, 0.0f);
}

void PlayerRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float rotationYaw,
                                    float partialTicks)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    LivingEntityRenderer::setupRotations(entity, ageInTicks, rotationYaw, partialTicks);
    if (player->isSleeping())
    {
        GlStateManager::rotatef(player->getSleepRotation(), 0.0f, 1.0f, 0.0f);
    }
}

void PlayerRenderer::renderShadow(boost::shared_ptr<Entity>& entity, double x, double y, double z, float alpha,
                                  float partialTicks)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    if (!player->SpectateAsInvisible()) EntityRenderer::renderShadow(entity, x, y, z, alpha, partialTicks);
}

bool PlayerRenderer::bindTexture(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    return EntityRenderer::bindTexture(entity);
}

void PlayerRenderer::renderLayers(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                  float ageInTicks, float netHeadYaw, float headPitch, float scale, float partialTick,
                                  bool overlay)
{
    PlayerModel* model = static_cast<PlayerModel*>(mModel);
    if (model->mEar != NULL) model->mEar->mShow = true;
    LivingEntityRenderer::renderLayers(entity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch,
                                        scale, partialTick, overlay);
    if (model->mEar != NULL) model->mEar->mShow = false;
}
