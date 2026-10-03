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
    if (entity->GetType() == EntityType::ARMOR_STAND)
    {
        boost::shared_ptr<ArmorStand> armorStand = boost::dynamic_pointer_cast<ArmorStand>(entity);

        mHead->mXRot = (float)armorStand->getHeadPose()->x * 0.017453292f;
        mHead->mYRot = (float)armorStand->getHeadPose()->y * 0.017453292f;
        mHead->mZRot = (float)armorStand->getHeadPose()->z * 0.017453292f;
        mHead->setPos(0.0f, 1.0f, 0.0f);

        mBody->mXRot = (float)armorStand->getBodyPose()->x * 0.017453292f;
        mBody->mYRot = (float)armorStand->getBodyPose()->y * 0.017453292f;
        mBody->mZRot = (float)armorStand->getBodyPose()->z * 0.017453292f;

        mLeftArm->mXRot = (float)armorStand->getLeftArmPose()->x * 0.017453292f;
        mLeftArm->mYRot = (float)armorStand->getLeftArmPose()->y * 0.017453292f;
        mLeftArm->mZRot = (float)armorStand->getLeftArmPose()->z * 0.017453292f;

        mRightArm->mXRot = (float)armorStand->getRightArmPose()->x * 0.017453292f;
        mRightArm->mYRot = (float)armorStand->getRightArmPose()->y * 0.017453292f;
        mRightArm->mZRot = (float)armorStand->getRightArmPose()->z * 0.017453292f;

        mLeftLeg->mXRot = (float)armorStand->getLeftLegPose()->x * 0.017453292f;
        mLeftLeg->mYRot = (float)armorStand->getLeftLegPose()->y * 0.017453292f;
        mLeftLeg->mZRot = (float)armorStand->getLeftLegPose()->z * 0.017453292f;
        mLeftLeg->setPos(1.9f, 12.0f, 0.0f);

        mRightLeg->mXRot = (float)armorStand->getRightLegPose()->x * 0.017453292f;
        mRightLeg->mYRot = (float)armorStand->getRightLegPose()->y * 0.017453292f;
        mRightLeg->mZRot = (float)armorStand->getRightLegPose()->z * 0.017453292f;
        mRightLeg->setPos(-1.9f, 12.0f, 0.0f);

        mRightItem->mXRot = (float)armorStand->getRightItemPose()->x * 0.017453292f;
        mRightItem->mYRot = (float)armorStand->getRightItemPose()->y * 0.017453292f;
        mRightItem->mZRot = (float)armorStand->getRightItemPose()->z * 0.017453292f;

        if (armorStand->getRightItemPose()->y == 180.0)
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
