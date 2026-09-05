#include "game/client/render/ParrotModel.hpp"

#include <cmath>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Parrot.hpp"
#include "game/entity/mob/Entity.hpp"

ParrotModel::ParrotModel(const ModelDefinition& modelDefinition)
    : Model()
{
    if (modelDefinition.DoesOverrideModel())
    {
        mBody = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);
        mTail = new ModelPartSkin(this, L"tail", modelDefinition, NULL, 0.0f, NULL);
        mWing0 = new ModelPartSkin(this, L"wing0", modelDefinition, NULL, 0.0f, NULL);
        mWing1 = new ModelPartSkin(this, L"wing1", modelDefinition, NULL, 0.0f, NULL);
        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, NULL);
        mHead2 = new ModelPartSkin(this, L"head2", modelDefinition, mHead, 0.0f, NULL);
        mBeak1 = new ModelPartSkin(this, L"beak1", modelDefinition, mHead, 0.0f, NULL);
        mBeak2 = new ModelPartSkin(this, L"beak2", modelDefinition, mHead, 0.0f, NULL);
        mFeather = new ModelPartSkin(this, L"feather", modelDefinition, mHead, 0.0f, NULL);
        mLeg0 = new ModelPartSkin(this, L"leg0", modelDefinition, NULL, 0.0f, NULL);
        mLeg1 = new ModelPartSkin(this, L"leg1", modelDefinition, NULL, 0.0f, NULL);
    }
    else
    {
        mTexWidth = 32;
        mTexHeight = 32;

        mBody = new ModelPartSkin(this, 2, 8, NULL);
        mBody->addBox(-1.5f, 0.0f, -1.5f, 3, 6, 3);
        mBody->setPos(0.0f, 16.5f, -3.0f);
        mTail = new ModelPartSkin(this, 22, 1, NULL);
        mTail->addBox(-1.5f, -1.0f, -1.0f, 3, 4, 1);
        mTail->setPos(0.0f, 21.07f, -0.5f);
        mWing0 = new ModelPartSkin(this, 19, 8, NULL);
        mWing0->addBox(-0.5f, 0.0f, -1.5f, 1, 5, 3);
        mWing0->setPos(1.5f, 16.94f, -2.76f);
        mWing1 = new ModelPartSkin(this, 19, 8, NULL);
        mWing1->addBox(-0.5f, 0.0f, -1.5f, 1, 5, 3);
        mWing1->setPos(-1.5f, 16.94f, -2.76f);
        mHead = new ModelPartSkin(this, 2, 2, NULL);
        mHead->addBox(-1.0f, -1.5f, -1.0f, 2, 3, 2);
        mHead->setPos(0.0f, 15.69f, -2.76f);
        mHead2 = new ModelPartSkin(this, 10, 0, NULL);
        mHead2->addBox(-1.0f, -0.5f, -2.0f, 2, 1, 4);
        mHead2->setPos(0.0f, -2.0f, -1.0f);
        mBeak1 = new ModelPartSkin(this, 11, 7, NULL);
        mBeak1->addBox(-0.5f, -1.0f, -0.5f, 1, 2, 1);
        mBeak1->setPos(0.0f, -0.5f, -1.5f);
        mBeak2 = new ModelPartSkin(this, 16, 7, NULL);
        mBeak2->addBox(-0.5f, 0.0f, -0.5f, 1, 2, 1);
        mBeak2->setPos(0.0f, -2.45f, -2.15f);
        mFeather = new ModelPartSkin(this, 2, 18, NULL);
        mFeather->addBox(0.0f, -4.0f, -2.0f, 0, 5, 4);
        mFeather->setPos(0.0f, -2.15f, 22.0f);
        mLeg0 = new ModelPartSkin(this, 14, 18, NULL);
        mLeg0->addBox(-0.5f, 0.0f, -1.0f, 1, 2, 1);
        mLeg0->setPos(1.0f, -2.15f, -1.05f);
        mLeg1 = new ModelPartSkin(this, 14, 18, NULL);
        mLeg1->addBox(-0.5f, 0.0f, -1.0f, 1, 2, 1);
        mLeg1->setPos(-1.0f, -2.15f, -1.05f);
    }
    compileSkinnedDrawlist();
}

ParrotModel::~ParrotModel()
{
}

void ParrotModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                         float ageInTicks, float netHeadYaw, float headPitch, float scale,
                         bool useDisplayList)
{
    mBody->render(scale, useDisplayList, false);
    mWing0->render(scale, useDisplayList, false);
    mWing1->render(scale, useDisplayList, false);
    mTail->render(scale, useDisplayList, false);
    mHead->render(scale, useDisplayList, false);
    mLeg0->render(scale, useDisplayList, false);
    mLeg1->render(scale, useDisplayList, false);
    callSkinnedDrawlist();
}

void ParrotModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks,
                            float netHeadYaw, float headPitch, float scale,
                            boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    const float radians = 0.01745329238474369f;
    mHead->mXRot = headPitch * radians;
    mHead->mYRot = netHeadYaw * radians;
    mHead->mZRot = mHeadZ;
    mHead2->mXRot = mHead2X;
    mBody->mXRot = mBodyX;
    mWing0->mXRot = mWing0X;
    mWing1->mXRot = mWing1X;

    if (mAnimationState == 0 || mAnimationState == 1)
    {
        mLeg0->mXRot = Mth::cos(limbSwing * 0.6662f) * 1.4f * limbSwingAmount;
        mLeg1->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 1.4f * limbSwingAmount;
        mWing0->mYRot = mWing0Y;
        mWing1->mYRot = mWing1Y;
        mTail->mZRot = mTailZ + ageInTicks * 0.3f;
    }
    else if (mAnimationState == 3)
    {
        boost::shared_ptr<Parrot> parrot = boost::static_pointer_cast<Parrot>(entity);
        float phase = (float)parrot->mFlapTime * radians - 176.0f;
        float sine = Mth::sin(phase);
        float cosine = Mth::cos(phase);
        mHead->mXRot = cosine;
        mHead->mYRot = cosine;
        mHead->mZRot = 0.0f;
        mBody->mXRot = 0.0f;
        mWing0->mXRot = cosine;
        mWing1->mXRot = cosine;
        mWing0->mYRot = 1.5f + cosine;
        mWing1->mYRot = -1.5f - cosine;
        mTail->mZRot = 0.0873f - ageInTicks;
        mLeg0->mZRot = -sine * 0.4f;
        mLeg1->mZRot = sine * 0.4f;
    }
    else
    {
        mTail->mZRot = mTailZ + ageInTicks * 0.3f;
        mWing0->mYRot = mWing0Y;
        mWing1->mYRot = mWing1Y;
    }
}

void ParrotModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                                  float limbSwingAmount, float partialTicks)
{
    mFeather->mZRot = mFeather->mZRot;
    mWing0->mZRot = mWing0Z;
    mWing1->mZRot = mWing1Z;
    mHead2->mZRot = mHead2Z;
    mAnimationState = 0;

    if (Minecraft::InMiniGame((EMiniGameId)15, true))
    {
        mAnimationState = 0;
    }
    else
    {
        boost::shared_ptr<Parrot> parrot = boost::static_pointer_cast<Parrot>(entity);
        if (parrot->isPartyParrot())
        {
            mAnimationState = 3;
            mWing0->mZRot = -0.01745329238474369f * 20.0f;
            mWing1->mZRot = 0.01745329238474369f * 20.0f;
        }
        else if (parrot->isFlying())
        {
            mAnimationState = 2;
            mTail->mYRot = 1.899999976f;
            mWing0->mZRot = -0.0873f;
            mWing1->mZRot = 0.0873f;
        }
        else
        {
            mAnimationState = 1;
            mLeg0->mYRot = 1.0153f;
            mLeg1->mYRot = 1.0153f;
        }
    }
}

std::wstring ParrotModel::getName()
{
    return L"parrot";
}
