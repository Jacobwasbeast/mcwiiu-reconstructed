#include "game/client/render/TurtleModel.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Turtle.hpp"

TurtleModel::TurtleModel(float scale, const ModelDefinition& modelDefinition)
    : QuadrupedModel(12, scale, modelDefinition)
{
    mTexWidth = 128;
    mTexHeight = 64;

    if (!modelDefinition.DoesOverrideModel())
    {
        mHead = new ModelPartSkin(this, 2, 0, NULL);
        mHead->addBox(-3.0f, -1.0f, -3.0f, 6, 5, 6, scale);
        mHead->setPos(0.0f, 19.0f, -10.0f);

        mEggBelly = new ModelPartSkin(this, 69, 33, NULL);
        mEggBelly->addBox(-9.0f, -3.0f, -12.0f, 9, 18, 1, scale);
        mEggBelly->setPos(0.0f, 13.0f, 19.0f);

        mBody = new ModelPartSkin(this);
        mBody->texOffs(6, 37);
        mBody->addBox(-9.5f, -12.0f, -10.0f, 19, 20, 6, scale);
        mBody->texOffs(30, 1);
        mBody->addBox(-5.5f, -12.0f, -13.0f, 11, 18, 3, scale);
        mBody->setPos(0.0f, 11.0f, -10.0f);

        mLeg0 = new ModelPartSkin(this, 0, 23, NULL);
        mLeg0->addBox(-3.5f, 0.0f, 0.0f, 4, 1, 10, scale);
        mLeg0->setPos(-3.5f, 22.0f, -5.5f);

        mLeg1 = new ModelPartSkin(this, 0, 12, NULL);
        mLeg1->addBox(-3.5f, 0.0f, 0.0f, 4, 1, 10, scale);
        mLeg1->setPos(3.5f, 22.0f, -5.5f);

        mLeg2 = new ModelPartSkin(this, 26, 30, NULL);
        mLeg2->addBox(-3.5f, 0.0f, 0.0f, 13, 1, 5, scale);
        mLeg2->setPos(-3.5f, 22.0f, -5.0f);

        mLeg3 = new ModelPartSkin(this, 26, 24, NULL);
        mLeg3->addBox(-3.5f, 0.0f, 0.0f, 13, 1, 5, scale);
        mLeg3->setPos(-3.5f, 22.0f, -5.0f);
    }
    else
    {
        mEggBelly = new ModelPartSkin(this, L"eggBelly", modelDefinition, NULL, 0.0f, NULL);
    }

    compileSkinnedDrawlist();
}

TurtleModel::~TurtleModel()
{
}

void TurtleModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                         float ageInTicks, float netHeadYaw, float headPitch, float scale,
                         bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale,
              entity, _SkinAdjustments());

    boost::shared_ptr<Turtle> turtle = boost::static_pointer_cast<Turtle>(entity);
    mEggBelly->mShow = !mYoung && turtle->hasEgg();

    GlStateManager::pushMatrix();
    if (!mYoung)
    {
        if (turtle->hasEgg())
        {
            GlStateManager::translatef(0.0f, -0.08f, 0.0f);
        }

        mHead->render(scale, useDisplayList, false);
        mBody->render(scale, useDisplayList, false);
        GlStateManager::pushMatrix();
        mLeg0->render(scale, useDisplayList, false);
        mLeg1->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();
        mLeg2->render(scale, useDisplayList, false);
        mLeg3->render(scale, useDisplayList, false);
        mEggBelly->render(scale, useDisplayList, false);
    }
    else
    {
        GlStateManager::scalef(0.1666666716337204f, 0.1666666716337204f, 0.1666666716337204f);
        GlStateManager::translatef(0.0f, 120.0f * scale, 0.0f);
        mHead->render(scale, useDisplayList, false);
        mBody->render(scale, useDisplayList, false);
        mLeg0->render(scale, useDisplayList, false);
        mLeg1->render(scale, useDisplayList, false);
        mLeg2->render(scale, useDisplayList, false);
        mLeg3->render(scale, useDisplayList, false);
        mEggBelly->render(scale, useDisplayList, false);
    }
    GlStateManager::popMatrix();
    callSkinnedDrawlist();
}

void TurtleModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks,
                            float netHeadYaw, float headPitch, float scale,
                            boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    QuadrupedModel::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch,
                              scale, entity, skinAdjustments);

    boost::shared_ptr<Turtle> turtle = boost::static_pointer_cast<Turtle>(entity);
    float swing = limbSwing * 0.6661999821662903f;
    float halfPi = 1.5707963705062866f;
    float lower = -1.5707963705062866f;
    float upper = 1.5707963705062866f;

    mLeg0->mXRot = Mth::clamp(Mth::cos(swing) * 0.5f * limbSwingAmount, lower, upper);
    mLeg1->mXRot = Mth::clamp(Mth::cos(swing + 3.1415927410125732f) * 0.5f * limbSwingAmount,
                              lower, upper);
    mLeg2->mXRot = 0.0f;
    mLeg3->mXRot = 0.0f;
    mLeg0->mYRot = 0.0f;
    mLeg1->mYRot = 0.0f;
    mLeg2->mYRot = 0.0f;
    mLeg3->mYRot = 0.0f;

    if (turtle->isLayingEgg())
    {
        mLeg0->mYRot = Mth::cos(ageInTicks) * halfPi;
        mLeg1->mYRot = Mth::cos(ageInTicks + 1.5707963705062866f) * halfPi;
        mLeg2->mYRot = Mth::cos(ageInTicks + 3.1415927410125732f) * halfPi;
        mLeg3->mYRot = Mth::cos(ageInTicks) * halfPi;
    }
    else
    {
        float paddle = limbSwing * 0.3997200131416321f;
        mLeg0->mYRot = Mth::clamp(Mth::cos(paddle + 3.1415927410125732f) * 0.5f * limbSwingAmount,
                                  lower, upper);
        mLeg1->mYRot = Mth::clamp(Mth::cos(paddle) * 0.5f * limbSwingAmount, lower, upper);
        mLeg2->mYRot = Mth::clamp(Mth::cos(paddle + 3.1415927410125732f) * 0.5f * limbSwingAmount,
                                  lower, upper);
        mLeg3->mYRot = Mth::clamp(Mth::cos(paddle) * 0.5f * limbSwingAmount, lower, upper);
    }

    mEggBelly->mXRot = 90.0f * 0.01745329238474369f;
}

std::wstring TurtleModel::getName()
{
    return L"turtle";
}
