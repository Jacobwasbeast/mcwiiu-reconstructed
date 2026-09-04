#include "game/client/render/ChickenModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"

ChickenModel::ChickenModel(const ModelDefinition& modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, NULL);
        mBody = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);
        mLeg0 = new ModelPartSkin(this, L"leg0", modelDefinition, NULL, 0.0f, NULL);
        mLeg1 = new ModelPartSkin(this, L"leg1", modelDefinition, NULL, 0.0f, NULL);
        mWing0 = new ModelPartSkin(this, L"wing0", modelDefinition, NULL, 0.0f, NULL);
        mWing1 = new ModelPartSkin(this, L"wing1", modelDefinition, NULL, 0.0f, NULL);
        mBeak = new ModelPartSkin(this, L"beak", modelDefinition, NULL, 0.0f, NULL);
        mComb = new ModelPartSkin(this, L"comb", modelDefinition, NULL, 0.0f, NULL);

        compileSkinnedDrawlist();
    }
    else
    {
        mHead = new ModelPartSkin(this, 0, 0, NULL);
        mHead->addBox(-2.0f, -6.0f, -2.0f, 4, 6, 3, 0.0f);
        mHead->setPos(0.0f, 15.0f, -4.0f);

        mBeak = new ModelPartSkin(this, 14, 0, NULL);
        mBeak->addBox(-2.0f, -4.0f, -4.0f, 4, 2, 2, 0.0f);
        mBeak->setPos(0.0f, 15.0f, -4.0f);

        mComb = new ModelPartSkin(this, 14, 4, NULL);
        mComb->addBox(-1.0f, -2.0f, -3.0f, 2, 2, 2, 0.0f);
        mComb->setPos(0.0f, 15.0f, -4.0f);

        mBody = new ModelPartSkin(this, 0, 9, NULL);
        mBody->addBox(-3.0f, -4.0f, -3.0f, 6, 8, 6, 0.0f);
        mBody->setPos(0.0f, 16.0f, 0.0f);

        mLeg0 = new ModelPartSkin(this, 26, 0, NULL);
        mLeg0->addBox(-1.0f, 0.0f, -3.0f, 3, 5, 3);
        mLeg0->setPos(-2.0f, 19.0f, 1.0f);

        mLeg1 = new ModelPartSkin(this, 26, 0, NULL);
        mLeg1->addBox(-1.0f, 0.0f, -3.0f, 3, 5, 3);
        mLeg1->setPos(1.0f, 19.0f, 1.0f);

        mWing0 = new ModelPartSkin(this, 24, 13, NULL);
        mWing0->addBox(0.0f, 0.0f, -3.0f, 1, 4, 6);
        mWing0->setPos(-4.0f, 13.0f, 0.0f);

        mWing1 = new ModelPartSkin(this, 24, 13, NULL);
        mWing1->addBox(0.0f, 0.0f, -3.0f, 1, 4, 6);
        mWing1->setPos(4.0f, 13.0f, 0.0f);

        compileSkinnedDrawlist();
    }
}

ChickenModel::~ChickenModel()
{
}

void ChickenModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    if (!mYoung)
    {
        mHead->render(scale, useDisplayList, false);
        mBeak->render(scale, useDisplayList, false);
        mComb->render(scale, useDisplayList, false);
        mBody->render(scale, useDisplayList, false);
        mLeg0->render(scale, useDisplayList, false);
        mLeg1->render(scale, useDisplayList, false);
        mWing0->render(scale, useDisplayList, false);
        mWing1->render(scale, useDisplayList, false);
    }
    else
    {
        GlStateManager::pushMatrix();
        GlStateManager::translatef(0.0f, 5.0f * scale, 2.0f * scale);
        mHead->render(scale, useDisplayList, false);
        mBeak->render(scale, useDisplayList, false);
        mComb->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();

        GlStateManager::pushMatrix();
        GlStateManager::scalef(0.5f, 0.5f, 0.5f);
        GlStateManager::translatef(0.0f, 24.0f * scale, 0.0f);
        mBody->render(scale, useDisplayList, false);
        mLeg0->render(scale, useDisplayList, false);
        mLeg1->render(scale, useDisplayList, false);
        mWing0->render(scale, useDisplayList, false);
        mWing1->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();
    }

    callSkinnedDrawlist();
}

void ChickenModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    mHead->mXRot = headPitch * 0.01745329238474369f;
    mHead->mYRot = netHeadYaw * 0.01745329238474369f;
    mBeak->mXRot = mHead->mXRot;
    mBeak->mYRot = mHead->mYRot;
    mComb->mXRot = mHead->mXRot;
    mComb->mYRot = mHead->mYRot;
    mBody->mXRot = 90.0f * 0.01745329238474369f;

    float swing = limbSwing * 0.6661999821662903f;
    mLeg0->mXRot = Mth::cos(swing) * 1.399999976158142f * limbSwingAmount;
    mLeg1->mXRot = Mth::cos(swing + 3.1415927410125732f) * 1.399999976158142f * limbSwingAmount;
    mWing0->mZRot = ageInTicks;
    mWing1->mZRot = -ageInTicks;
}

std::wstring ChickenModel::getName()
{
    return L"chicken";
}
