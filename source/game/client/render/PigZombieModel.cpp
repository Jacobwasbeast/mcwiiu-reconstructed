#include "game/client/render/PigZombieModel.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/block/MaterialTable.hpp"

#include "game/entity/mob/PigZombie.hpp"

PigZombieModel::PigZombieModel(const ModelDefinition& definition)
    : HumanoidModel(definition, 0.0f, 0.0f, 64, 32, false, false, true)
{
    init(definition, 0.0f, false);
}

PigZombieModel::~PigZombieModel()
{
}

void PigZombieModel::init(const ModelDefinition& definition, float scale, bool armourModel)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    if (definition.DoesOverrideModel())
    {
        mHasOverriddenParts = true;

        mHead = createModelPart(true, L"head", definition, NULL, 0.0f);
        mHat = createModelPart(true, L"hat", definition, NULL, 0.0f);
        mBody = createModelPart(true, L"body", definition, NULL, 0.0f);
        mRightArm = createModelPart(true, L"rightArm", definition, NULL, 0.0f);
        mLeftArm = createModelPart(true, L"leftArm", definition, NULL, 0.0f);
        mRightLeg = createModelPart(true, L"rightLeg", definition, NULL, 0.0f);
        mLeftLeg = createModelPart(true, L"leftLeg", definition, NULL, 0.0f);

        if (definition.hasModelPart(L"rightItem"))
        {
            mRightItem = createModelPart(true, L"rightItem", definition, mRightArm, 0.0f);
        }

        if (definition.hasModelPart(L"leftItem"))
        {
            mLeftItem = createModelPart(true, L"leftItem", definition, mLeftArm, 0.0f);
        }

        builder->setVertexAndPixelShader(eMinecraftEntityMaterial_ZombiePigman);
    }
    else
    {
        mHead = createModelPart(true, 0, 0);
        mHead->addHumanoidBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, scale);
        mHead->setPos(0.0f, 0.0f, 0.0f);

        mHat = createModelPart(true, 32, 0);
        mHat->addHumanoidBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, scale + 0.5f);
        mHat->setPos(0.0f, 0.0f, 0.0f);

        mBody = createModelPart(true, 16, 16);
        mBody->addHumanoidBox(-4.0f, 0.0f, -2.0f, 8, 12, 4, scale);
        mBody->setPos(0.0f, 0.0f, 0.0f);

        mRightArm = createModelPart(true, 40, 16);
        mRightArm->addHumanoidBox(-3.0f, -2.0f, -2.0f, 4, 12, 4, scale);
        mRightArm->setPos(-5.0f, 2.0f, 0.0f);

        mLeftArm = createModelPart(true, 40, 16);
        mLeftArm->mMirror = true;
        mLeftArm->addHumanoidBox(-1.0f, -2.0f, -2.0f, 4, 12, 4, scale);
        mLeftArm->setPos(5.0f, 2.0f, 0.0f);

        mRightLeg = createModelPart(true, 0, 16);
        mRightLeg->addHumanoidBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, scale);
        mRightLeg->setPos(-1.899999976158142f, 12.0f, 0.0f);

        mLeftLeg = createModelPart(true, 0, 16);
        mLeftLeg->mMirror = true;
        mLeftLeg->addHumanoidBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, scale);
        mLeftLeg->setPos(1.899999976158142f, 12.0f, 0.0f);
    }

    if (mRightItem == NULL)
    {
        mRightItem = createModelPart(true, 0, 0);
        mRightItem->setPos(-6.0f - mRightArm->mX, 9.0f - mRightArm->mY, 1.0f - mRightArm->mZ);
    }

    if (mLeftItem == NULL)
    {
        mLeftItem = createModelPart(true, 0, 0);
        mLeftItem->setPos(mRightItem->mX, mRightItem->mY, mRightItem->mZ);
    }

    mRightArm->addChild(mRightItem);
    mLeftArm->addChild(mLeftItem);

    compileSkinnedDrawlist();

    builder->useDefaultShaders();
}

void PigZombieModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    HumanoidModel::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, skinAdjustments);

    bool angry = false;

    if (entity->GetType() == ENTITY_TYPE_PIG_ZOMBIE)
    {
        boost::shared_ptr<PigZombie> pigZombie = boost::static_pointer_cast<PigZombie>(entity);

        if (pigZombie->isAngry())
        {
            angry = true;
        }
    }

    float attackSwing = Mth::sin(mAttackTime * 3.1415927410125732f);
    float attackEase = Mth::sin((1.0f - (1.0f - mAttackTime) * (1.0f - mAttackTime)) * 3.1415927410125732f);

    mRightArm->mZRot = 0.0f;
    mLeftArm->mZRot = 0.0f;
    mRightArm->mYRot = -(0.10000000149011612f - attackSwing * 0.6000000238418579f);
    mLeftArm->mYRot = 0.10000000149011612f - attackSwing * 0.6000000238418579f;

    float armRaise = -3.1415927410125732f / (angry ? 1.5f : 2.25f);

    if (mRightArmPose != eArmPose_ThrowSpear)
    {
        mRightArm->mXRot = armRaise;
    }

    mLeftArm->mXRot = armRaise;

    mRightArm->mXRot += attackSwing * 1.2000000476837158f - attackEase * 0.4000000059604645f;
    mLeftArm->mXRot += attackSwing * 1.2000000476837158f - attackEase * 0.4000000059604645f;

    mRightArm->mZRot += Mth::cos(ageInTicks * 0.09000000357627869f) * 0.05000000074505806f + 0.05000000074505806f;
    mLeftArm->mZRot -= Mth::cos(ageInTicks * 0.09000000357627869f) * 0.05000000074505806f + 0.05000000074505806f;
    mRightArm->mXRot += Mth::sin(ageInTicks * 0.06700000166893005f) * 0.05000000074505806f;
    mLeftArm->mXRot -= Mth::sin(ageInTicks * 0.06700000166893005f) * 0.05000000074505806f;
}

std::wstring PigZombieModel::getName()
{
    return L"pigzombie";
}
