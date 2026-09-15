#include "game/client/render/FishModel.hpp"

#include "game/client/render/FishRenderer.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Fish.hpp"
#include "game/entity/mob/Entity.hpp"

FishModel::FishModel(const ModelDefinition& modelDefinition, FishRenderer::FishType fishType, int variant)
    : Model(),
      mPart0(NULL), mPart1(NULL), mPart2(NULL), mPart3(NULL), mPart4(NULL), mPart5(NULL),
      mPart6(NULL), mPart7(NULL), mPart8(NULL), mPart9(NULL), mPart10(NULL), mPart11(NULL),
      mPart12(NULL), mPart13(NULL), mPart14(NULL), mPart15(NULL), mPart16(NULL), mPart17(NULL),
      mPart18(NULL), mPart19(NULL), mPart20(NULL), mFishType(fishType), mVariant(variant),
      mAnimationScale(0.0f), mAnimationOffset(0.0f)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mPart0 = new ModelPartSkin(this, L"root", modelDefinition, NULL, 0.0f, NULL);
        mPart1 = new ModelPartSkin(this, L"body", modelDefinition, mPart0, 0.0f, NULL);
        mPart2 = new ModelPartSkin(this, L"body_1", modelDefinition, mPart0, 0.0f, NULL);
        mPart3 = new ModelPartSkin(this, L"body_2", modelDefinition, mPart0, 0.0f, NULL);
        mPart4 = new ModelPartSkin(this, L"body_3", modelDefinition, mPart0, 0.0f, NULL);
        mPart5 = new ModelPartSkin(this, L"body_4", modelDefinition, mPart0, 0.0f, NULL);
        mPart6 = new ModelPartSkin(this, L"body_5", modelDefinition, mPart0, 0.0f, NULL);
        mPart7 = new ModelPartSkin(this, L"body_6", modelDefinition, mPart0, 0.0f, NULL);
        mPart8 = new ModelPartSkin(this, L"body_7", modelDefinition, mPart0, 0.0f, NULL);
        mPart9 = new ModelPartSkin(this, L"body_8", modelDefinition, mPart0, 0.0f, NULL);
        mPart10 = new ModelPartSkin(this, L"body_9", modelDefinition, mPart0, 0.0f, NULL);
        mPart11 = new ModelPartSkin(this, L"body_10", modelDefinition, mPart0, 0.0f, NULL);
        mPart12 = new ModelPartSkin(this, L"body_11", modelDefinition, mPart0, 0.0f, NULL);
        mPart13 = new ModelPartSkin(this, L"body_12", modelDefinition, mPart0, 0.0f, NULL);
        mPart14 = new ModelPartSkin(this, L"body_13", modelDefinition, mPart0, 0.0f, NULL);
        mPart15 = new ModelPartSkin(this, L"body_14", modelDefinition, mPart0, 0.0f, NULL);
        mPart16 = new ModelPartSkin(this, L"body_15", modelDefinition, mPart0, 0.0f, NULL);
        mPart17 = new ModelPartSkin(this, L"body_16", modelDefinition, mPart0, 0.0f, NULL);
        mPart18 = new ModelPartSkin(this, L"body_17", modelDefinition, mPart0, 0.0f, NULL);
        mPart19 = new ModelPartSkin(this, L"body_18", modelDefinition, mPart0, 0.0f, NULL);
        mPart20 = new ModelPartSkin(this, L"body_19", modelDefinition, mPart0, 0.0f, NULL);
        compileSkinnedDrawlist();
        return;
    }

    mTexWidth = 32;
    mTexHeight = 32;

    if (fishType == FishRenderer::FISH_TYPE_COD)
    {
        mPart0 = new ModelPartSkin(this, 0, 0, NULL);
        mPart0->addBox(-1.5f, -2.0f, -3.5f, 3, 4, 7);
        mPart0->setPos(0.0f, 0.0f, -1.0f);
        mPart1 = new ModelPartSkin(this, 0, 0, NULL);
        mPart1->addBox(-1.5f, -1.5f, -1.0f, 3, 3, 1);
        mPart1->setPos(0.0f, 0.0f, 3.0f);
        mPart2 = new ModelPartSkin(this, 24, 4, NULL);
        mPart2->addBox(-1.0f, 0.0f, -1.0f, 2, 1, 2);
        mPart2->setPos(0.0f, 0.0f, -2.0f);
        mPart3 = new ModelPartSkin(this, 24, 1, NULL);
        mPart3->addBox(-1.0f, 0.0f, -1.0f, 2, 1, 2);
        mPart3->setPos(0.0f, 0.0f, 0.0f);
        mPart4 = new ModelPartSkin(this, 20, 1, NULL);
        mPart4->addBox(-0.0f, -2.0f, -3.0f, 0, 4, 6);
        mPart4->setPos(0.0f, 0.0f, 0.0f);
        mPart0->addChild(mPart1);
        mPart0->addChild(mPart2);
        mPart0->addChild(mPart3);
        mPart0->addChild(mPart4);
    }
    else if (fishType == FishRenderer::FISH_TYPE_PUFFERFISH)
    {
        mPart0 = new ModelPartSkin(this, 0, 0, NULL);
        mPart0->addBox(-4.0f, -4.0f, -4.0f, 8, 8, 8);
        mPart1 = new ModelPartSkin(this, 24, 3, NULL);
        mPart1->addBox(-1.0f, -1.0f, -1.0f, 2, 1, 2);
        mPart2 = new ModelPartSkin(this, 24, 0, NULL);
        mPart2->addBox(-1.0f, -1.0f, -1.0f, 2, 1, 2);
        mPart0->addChild(mPart1);
        mPart0->addChild(mPart2);
    }
    else if (fishType == FishRenderer::FISH_TYPE_SALMON)
    {
        mPart0 = new ModelPartSkin(this, 0, 0, NULL);
        mPart0->addBox(-1.5f, -2.5f, -4.0f, 3, 5, 8);
        mPart1 = new ModelPartSkin(this, 0, 13, NULL);
        mPart1->addBox(-1.5f, -2.5f, -4.0f, 3, 5, 8);
        mPart1->setPos(0.0f, 0.0f, -1.0f);
        mPart2 = new ModelPartSkin(this, 4, 2, NULL);
        mPart2->addBox(0.0f, -1.0f, -1.0f, 2, 2, 2);
        mPart3 = new ModelPartSkin(this, 2, 3, NULL);
        mPart3->addBox(0.0f, -1.0f, -1.0f, 2, 2, 3);
        mPart0->addChild(mPart1);
        mPart0->addChild(mPart2);
        mPart0->addChild(mPart3);
    }
    else
    {
        mPart0 = new ModelPartSkin(this, 0, 0, NULL);
        mPart0->addBox(-2.0f, -3.0f, -3.0f, 4, 6, 6);
        mPart1 = new ModelPartSkin(this, 24, 0, NULL);
        mPart1->addBox(-1.0f, -3.0f, -2.0f, 2, 3, 4);
        mPart2 = new ModelPartSkin(this, 2, 12, NULL);
        mPart2->addBox(-1.0f, -1.0f, -1.0f, 2, 2, 0);
        mPart3 = new ModelPartSkin(this, 2, 16, NULL);
        mPart3->addBox(-1.0f, -1.0f, -1.0f, 2, 2, 0);
        mPart0->addChild(mPart1);
        mPart0->addChild(mPart2);
        mPart0->addChild(mPart3);
    }
    compileSkinnedDrawlist();
}

FishModel::~FishModel()
{
    return;
}

void FishModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                       float ageInTicks, float netHeadYaw, float headPitch, float scale,
                       bool useDisplayList)
{
    boost::shared_ptr<Fish> fish = boost::static_pointer_cast<Fish>(entity);
    const float fishLimbSwing = Mth::Lerp(fish->mSwimRotationOld, fish->mSwimRotation, 0.05f);
    setupAnim(fishLimbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity,
              _SkinAdjustments());
    if (!fish->mSwimFlag)
    {
        mPart0->render(scale, useDisplayList, false);
        callSkinnedDrawlist();
    }
    else
    {
        GlStateManager::pushMatrix();
        GlStateManager::scalef(fish->mSwimSpeed, fish->mSwimSpeed, fish->mSwimSpeed);
        GlStateManager::translatef(0.0f, static_cast<float>(fish->mSwimTicks) * scale, 0.0f);
        mPart0->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();
        callSkinnedDrawlist();
    }
}

void FishModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                          float headPitch, float scale, boost::shared_ptr<Entity> entity,
                          _SkinAdjustments skinAdjustments)
{
    const float animation = Mth::cos(limbSwing * 0.01745329238474369f * mAnimationScale);
    if (mFishType == FishRenderer::FISH_TYPE_COD)
    {
        const float value = animation * -25.75f;
        mPart4->mYRot = animation * 4.0f;
        mPart0->mYRot = animation + animation;
        mPart1->mYRot = value;
    }
    else if (mFishType == FishRenderer::FISH_TYPE_PUFFERFISH)
    {
        mPart1->mYRot = animation * -5.75f;
    }
    else if (mFishType == FishRenderer::FISH_TYPE_SALMON)
    {
        const float value = animation * -40.0f;
        mPart0->mYRot = animation * 4.0f;
        mPart3->mYRot = animation * -21.75f;
        mPart1->mYRot = value;
    }
    else
    {
        const float value = animation * -25.75f;
        mPart0->mYRot = animation + animation;
        mPart1->mYRot = value;
    }
}

void FishModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                                float limbSwingAmount, float partialTicks)
{
    Model::prepareMobModel(entity, limbSwing, limbSwingAmount, partialTicks);
    if (mFishType == FishRenderer::FISH_TYPE_PUFFERFISH)
    {
        mAnimationScale = 30.0f;
        mAnimationOffset = 30.0f;
    }
    else
    {
        mAnimationScale = 45.0f;
        mAnimationOffset = 45.0f;
    }
    if (mPart0 != NULL)
    {
        mPart0->mZRot = Mth::cos(partialTicks * 0.25f) * 90.0f * 0.01745329238474369f;
    }
}

std::wstring FishModel::getName()
{
    if (mFishType == FishRenderer::FISH_TYPE_COD) return L"cod";
    if (mFishType == FishRenderer::FISH_TYPE_PUFFERFISH)
    {
        if (mVariant == 0) return L"pufferfish_small";
        if (mVariant == 1) return L"pufferfish_mid";
        if (mVariant == 2) return L"pufferfish_large";
        return L"pufferfish";
    }
    if (mFishType == FishRenderer::FISH_TYPE_SALMON) return L"salmon";
    if (mVariant == 0) return L"tropicalfish_a";
    if (mVariant == 1) return L"tropicalfish_b";
    return L"cod";
}
