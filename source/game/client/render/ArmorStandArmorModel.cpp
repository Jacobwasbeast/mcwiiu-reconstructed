#include "game/client/render/ArmorStandArmorModel.hpp"
#include "game/core/Mth.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/ArmorStand.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Vec3.hpp"

ArmorStandArmorModel::ArmorStandArmorModel(float scale, int texWidth, int texHeight, const ModelDefinition& definition)
    : HumanoidModel(definition, scale, 0.0f, texWidth, texHeight, true, true, true)
{
}

ArmorStandArmorModel::~ArmorStandArmorModel()
{
}

void ArmorStandArmorModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    if (eTYPE_DERIVED_FROM(EntityType::ARMOR_STAND, entity.get()->GetType()))
    {
        boost::shared_ptr<ArmorStand> armorStand = boost::dynamic_pointer_cast<ArmorStand>(entity);

        mHead->mXRot = (float)armorStand.get()->getHeadPose()->x * Mth::DEG_TO_RAD;
        mHead->mYRot = (float)armorStand.get()->getHeadPose()->y * Mth::DEG_TO_RAD;
        mHead->mZRot = (float)armorStand.get()->getHeadPose()->z * Mth::DEG_TO_RAD;
        mHead->setPos(0.0f, 1.0f, 0.0f);

        mBody->mXRot = (float)armorStand.get()->getBodyPose()->x * Mth::DEG_TO_RAD;
        mBody->mYRot = (float)armorStand.get()->getBodyPose()->y * Mth::DEG_TO_RAD;
        mBody->mZRot = (float)armorStand.get()->getBodyPose()->z * Mth::DEG_TO_RAD;

        mLeftArm->mXRot = (float)armorStand.get()->getLeftArmPose()->x * Mth::DEG_TO_RAD;
        mLeftArm->mYRot = (float)armorStand.get()->getLeftArmPose()->y * Mth::DEG_TO_RAD;
        mLeftArm->mZRot = (float)armorStand.get()->getLeftArmPose()->z * Mth::DEG_TO_RAD;

        mRightArm->mXRot = (float)armorStand.get()->getRightArmPose()->x * Mth::DEG_TO_RAD;
        mRightArm->mYRot = (float)armorStand.get()->getRightArmPose()->y * Mth::DEG_TO_RAD;
        mRightArm->mZRot = (float)armorStand.get()->getRightArmPose()->z * Mth::DEG_TO_RAD;

        mLeftLeg->mXRot = (float)armorStand.get()->getLeftLegPose()->x * Mth::DEG_TO_RAD;
        mLeftLeg->mYRot = (float)armorStand.get()->getLeftLegPose()->y * Mth::DEG_TO_RAD;
        mLeftLeg->mZRot = (float)armorStand.get()->getLeftLegPose()->z * Mth::DEG_TO_RAD;
        mLeftLeg->setPos(1.9f, 12.0f, 0.0f);

        mRightLeg->mXRot = (float)armorStand.get()->getRightLegPose()->x * Mth::DEG_TO_RAD;
        mRightLeg->mYRot = (float)armorStand.get()->getRightLegPose()->y * Mth::DEG_TO_RAD;
        mRightLeg->mZRot = (float)armorStand.get()->getRightLegPose()->z * Mth::DEG_TO_RAD;
        mRightLeg->setPos(-1.9f, 12.0f, 0.0f);

        mRightItem->mXRot = (float)armorStand.get()->getRightItemPose()->x * Mth::DEG_TO_RAD;
        mRightItem->mYRot = (float)armorStand.get()->getRightItemPose()->y * Mth::DEG_TO_RAD;
        mRightItem->mZRot = (float)armorStand.get()->getRightItemPose()->z * Mth::DEG_TO_RAD;

        if (armorStand.get()->getRightItemPose()->y == 180.0)
        {
            mRightItem->mX = -2.0f;
        }
        else
        {
            mRightItem->mX = 0.0f;
        }

        Model::copyModelPart(mHead, mHat);
    }
}
