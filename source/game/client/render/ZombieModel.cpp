#include "game/client/render/ZombieModel.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Zombie.hpp"

ZombieModel::ZombieModel(const ModelDefinition& definition)
    : HumanoidModel(definition, 0.0f, 0.0f, 64, 32, false, true, true)
{
}

ZombieModel::ZombieModel(float scale, bool armourModel, const ModelDefinition& definition)
    : HumanoidModel(definition, scale, 0.0f, 64, armourModel ? 32 : 64, armourModel, true, true)
{
}

ZombieModel::ZombieModel(C4JRender::eVertexType vertexType, C4JRender::ePixelShaderType pixelShaderType, const ModelDefinition& definition, float scale, float yOffset, int texWidth, int texHeight, bool armourModel, bool createParts, bool skinned)
    : HumanoidModel(vertexType, pixelShaderType, definition, scale, yOffset, texWidth, texHeight, armourModel, createParts, skinned)
{
}

ZombieModel::~ZombieModel()
{
}

void ZombieModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    HumanoidModel::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, skinAdjustments);

    bool aggressive = false;

    if ((entity->GetType() & ENTITY_TYPE_ZOMBIE) == ENTITY_TYPE_ZOMBIE)
    {
        boost::shared_ptr<Zombie> zombie = boost::dynamic_pointer_cast<Zombie>(entity);
        aggressive = zombie->isAggressive();
    }

    float attackSwing = Mth::sin(mAttackTime * 3.1415927410125732f);
    float attackSwingBack = Mth::sin((1.0f - (1.0f - mAttackTime) * (1.0f - mAttackTime)) * 3.1415927410125732f);

    mRightArm->mZRot = 0.0f;
    mLeftArm->mZRot = 0.0f;
    mRightArm->mYRot = -(0.1f - attackSwing * 0.6f);
    mLeftArm->mYRot = 0.1f - attackSwing * 0.6f;

    float armXRot = -3.1415927410125732f / (aggressive ? 1.5f : 2.25f);

    if (mRightArmPose != eArmPose_ThrowSpear)
    {
        mRightArm->mXRot = armXRot;
    }

    mLeftArm->mXRot = armXRot;
    mRightArm->mXRot += attackSwing * 1.2f - attackSwingBack * 0.4f;
    mLeftArm->mXRot += attackSwing * 1.2f - attackSwingBack * 0.4f;
    mRightArm->mZRot += Mth::cos(ageInTicks * 0.09f) * 0.05f + 0.05f;
    mLeftArm->mZRot -= Mth::cos(ageInTicks * 0.09f) * 0.05f + 0.05f;
    mRightArm->mXRot += Mth::sin(ageInTicks * 0.067f) * 0.05f;
    mLeftArm->mXRot -= Mth::sin(ageInTicks * 0.067f) * 0.05f;
}

std::wstring ZombieModel::getName()
{
    return L"zombie";
}
