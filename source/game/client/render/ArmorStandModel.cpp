#include "game/client/render/ArmorStandModel.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/ArmorStand.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/entity/mob/Entity.hpp"

ArmorStandModel::ArmorStandModel(float scale, const ModelDefinition& definition)
    : ArmorStandArmorModel(scale, 64, 64, definition)
{
    delete mHead;
    mHead = new ModelPartSkin(this, 0, 0, NULL);
    mHead->addBox(-1.0f, -7.0f, -1.0f, 2, 7, 2, scale);
    mHead->setPos(0.0f, 0.0f, 0.0f);

    delete mBody;
    mBody = new ModelPartSkin(this, 0, 26, NULL);
    mBody->addBox(-6.0f, 0.0f, -1.5f, 12, 3, 3, scale);
    mBody->setPos(0.0f, 0.0f, 0.0f);

    delete mRightArm;
    mRightArm = new ModelPartSkin(this, 24, 0, NULL);
    mRightArm->addBox(-2.0f, -2.0f, -1.0f, 2, 12, 2, scale);
    mRightArm->setPos(-5.0f, 2.0f, 0.0f);

    delete mLeftArm;
    mLeftArm = new ModelPartSkin(this, 32, 16, NULL);
    mLeftArm->mMirror = true;
    mLeftArm->addBox(0.0f, -2.0f, -1.0f, 2, 12, 2, scale);
    mLeftArm->setPos(5.0f, 2.0f, 0.0f);

    delete mRightLeg;
    mRightLeg = new ModelPartSkin(this, 8, 0, NULL);
    mRightLeg->addBox(-1.0f, 0.0f, -1.0f, 2, 11, 2, scale);
    mRightLeg->setPos(-1.9f, 12.0f, 0.0f);

    delete mLeftLeg;
    mLeftLeg = new ModelPartSkin(this, 40, 16, NULL);
    mLeftLeg->mMirror = true;
    mLeftLeg->addBox(-1.0f, 0.0f, -1.0f, 2, 11, 2, scale);
    mLeftLeg->setPos(1.9f, 12.0f, 0.0f);

    mRightArm->addChild(mRightItem);
    mLeftArm->addChild(mLeftItem);

    mArmorStandBody = new ModelPartSkin(this, 16, 0, NULL);
    mArmorStandBody->addBox(-3.0f, 3.0f, -1.0f, 2, 7, 2, scale);
    mArmorStandBody->setPos(0.0f, 0.0f, 0.0f);
    mArmorStandBody->mShow = true;

    mArmorStandRightArm = new ModelPartSkin(this, 48, 16, NULL);
    mArmorStandRightArm->addBox(1.0f, 3.0f, -1.0f, 2, 7, 2, scale);
    mArmorStandRightArm->setPos(0.0f, 0.0f, 0.0f);

    mArmorStandLeftArm = new ModelPartSkin(this, 0, 48, NULL);
    mArmorStandLeftArm->addBox(-4.0f, 10.0f, -1.0f, 8, 2, 2, scale);
    mArmorStandLeftArm->setPos(0.0f, 0.0f, 0.0f);

    mArmorStandBase = new ModelPartSkin(this, 0, 32, NULL);
    mArmorStandBase->addBox(-6.0f, 11.0f, -6.0f, 12, 1, 12, scale);
    mArmorStandBase->setPos(0.0f, 12.0f, 0.0f);
    mHat->mShow = false;
    compileSkinnedDrawlist();
}

ArmorStandModel::~ArmorStandModel()
{
}

void ArmorStandModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                                float headPitch, float scale, boost::shared_ptr<Entity> entity,
                                _SkinAdjustments skinAdjustments)
{
    ArmorStandArmorModel::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale,
                                    entity, skinAdjustments);

    bool isArmorStand = entity.get()->GetType() == EntityType::ARMOR_STAND;
    if (isArmorStand)
    {
        boost::shared_ptr<ArmorStand> armorStand = boost::dynamic_pointer_cast<ArmorStand>(entity);

        mLeftArm->mShow = armorStand.get()->isShowArms();
        mRightArm->mShow = armorStand.get()->isShowArms();
        mArmorStandBase->mShow = !armorStand.get()->isNoBasePlate();

        const float degreesToRadians = 0.017453292f;
        mArmorStandBody->mXRot = (float)armorStand.get()->getBodyPose()->x * degreesToRadians;
        mArmorStandBody->mYRot = (float)armorStand.get()->getBodyPose()->y * degreesToRadians;
        mArmorStandBody->mZRot = (float)armorStand.get()->getBodyPose()->z * degreesToRadians;

        mArmorStandRightArm->mXRot = (float)armorStand.get()->getBodyPose()->x * degreesToRadians;
        mArmorStandRightArm->mYRot = (float)armorStand.get()->getBodyPose()->y * degreesToRadians;
        mArmorStandRightArm->mZRot = (float)armorStand.get()->getBodyPose()->z * degreesToRadians;

        mArmorStandLeftArm->mXRot = (float)armorStand.get()->getBodyPose()->x * degreesToRadians;
        mArmorStandLeftArm->mYRot = (float)armorStand.get()->getBodyPose()->y * degreesToRadians;
        mArmorStandLeftArm->mZRot = (float)armorStand.get()->getBodyPose()->z * degreesToRadians;

        mArmorStandBase->mXRot = 0.0f;
        mArmorStandBase->mYRot = -(degreesToRadians * entity.get()->mYRot);
        mArmorStandBase->mZRot = 0.0f;
    }
}

void ArmorStandModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                             float ageInTicks, float netHeadYaw, float headPitch, float scale,
                             bool useDisplayList)
{
    setupBoneMatrices(entity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale,
                      useDisplayList);
    GlStateManager::pushMatrix();

    if (!mYoung)
    {
        if (entity.get()->isSneaking())
        {
            GlStateManager::translatef(0.0f, 0.2f, 0.0f);
        }

        mArmorStandBody->render(scale, useDisplayList, false);
        mArmorStandRightArm->render(scale, useDisplayList, false);
        mArmorStandLeftArm->render(scale, useDisplayList, false);
        mArmorStandBase->render(scale, useDisplayList, false);
    }
    else
    {
        GlStateManager::scalef(0.5f, 0.5f, 0.5f);
        GlStateManager::translatef(0.0f, 24.0f * scale, 0.0f);
        mArmorStandBody->render(scale, useDisplayList, false);
        mArmorStandRightArm->render(scale, useDisplayList, false);
        mArmorStandLeftArm->render(scale, useDisplayList, false);
        mArmorStandBase->render(scale, useDisplayList, false);
    }

    GlStateManager::popMatrix();
    callSkinnedDrawlist();
}

void ArmorStandModel::translateToHandItem(float scale, const HumanoidArm* arm)
{
    ModelPart* part = getArm(arm);
    bool show = part->mShow;
    part->mShow = true;
    HumanoidModel::translateToHandItem(scale, arm);
    part->mShow = show;
}
