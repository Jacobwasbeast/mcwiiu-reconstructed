#include "game/client/render/ArmorStandArmorModel.hpp"
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
    if (entity.get()->GetType() == EntityType::ARMOR_STAND)
    {
        boost::shared_ptr<ArmorStand> armorStand = boost::dynamic_pointer_cast<ArmorStand>(entity);

        mHead->mXRot = (float)armorStand.get()->getHeadPose()->x * 0.017453292f;
        mHead->mYRot = (float)armorStand.get()->getHeadPose()->y * 0.017453292f;
        mHead->mZRot = (float)armorStand.get()->getHeadPose()->z * 0.017453292f;
        mHead->setPos(0.0f, 1.0f, 0.0f);

        mBody->mXRot = (float)armorStand.get()->getBodyPose()->x * 0.017453292f;
        mBody->mYRot = (float)armorStand.get()->getBodyPose()->y * 0.017453292f;
        mBody->mZRot = (float)armorStand.get()->getBodyPose()->z * 0.017453292f;

        mLeftArm->mXRot = (float)armorStand.get()->getLeftArmPose()->x * 0.017453292f;
        mLeftArm->mYRot = (float)armorStand.get()->getLeftArmPose()->y * 0.017453292f;
        mLeftArm->mZRot = (float)armorStand.get()->getLeftArmPose()->z * 0.017453292f;

        mRightArm->mXRot = (float)armorStand.get()->getRightArmPose()->x * 0.017453292f;
        mRightArm->mYRot = (float)armorStand.get()->getRightArmPose()->y * 0.017453292f;
        mRightArm->mZRot = (float)armorStand.get()->getRightArmPose()->z * 0.017453292f;

        mLeftLeg->mXRot = (float)armorStand.get()->getLeftLegPose()->x * 0.017453292f;
        mLeftLeg->mYRot = (float)armorStand.get()->getLeftLegPose()->y * 0.017453292f;
        mLeftLeg->mZRot = (float)armorStand.get()->getLeftLegPose()->z * 0.017453292f;
        mLeftLeg->setPos(1.9f, 12.0f, 0.0f);

        mRightLeg->mXRot = (float)armorStand.get()->getRightLegPose()->x * 0.017453292f;
        mRightLeg->mYRot = (float)armorStand.get()->getRightLegPose()->y * 0.017453292f;
        mRightLeg->mZRot = (float)armorStand.get()->getRightLegPose()->z * 0.017453292f;
        mRightLeg->setPos(-1.9f, 12.0f, 0.0f);

        mRightItem->mXRot = (float)armorStand.get()->getRightItemPose()->x * 0.017453292f;
        mRightItem->mYRot = (float)armorStand.get()->getRightItemPose()->y * 0.017453292f;
        mRightItem->mZRot = (float)armorStand.get()->getRightItemPose()->z * 0.017453292f;

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
