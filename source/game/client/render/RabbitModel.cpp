#include "game/client/render/RabbitModel.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Rabbit.hpp"

RabbitModel::RabbitModel(const ModelDefinition& modelDefinition)
    : Model()
{
    mJumpRotation = 0.0f;
    mOldJumpRotation = 0.0f;

    if (modelDefinition.DoesOverrideModel())
    {
        mRearFootLeft = new ModelPartSkin(this, L"rearFootLeft", modelDefinition, NULL, 0.0f, NULL);
        mRearFootRight = new ModelPartSkin(this, L"rearFootRight", modelDefinition, NULL, 0.0f, NULL);
        mHaunchLeft = new ModelPartSkin(this, L"haunchLeft", modelDefinition, NULL, 0.0f, NULL);
        mHaunchRight = new ModelPartSkin(this, L"haunchRight", modelDefinition, NULL, 0.0f, NULL);
        mBody = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);
        mFrontLegLeft = new ModelPartSkin(this, L"frontLegLeft", modelDefinition, NULL, 0.0f, NULL);
        mFrontLegRight = new ModelPartSkin(this, L"frontLegRight", modelDefinition, NULL, 0.0f, NULL);
        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, NULL);
        mEarRight = new ModelPartSkin(this, L"earRight", modelDefinition, NULL, 0.0f, NULL);
        mEarLeft = new ModelPartSkin(this, L"earLeft", modelDefinition, NULL, 0.0f, NULL);
        mTail = new ModelPartSkin(this, L"tail", modelDefinition, NULL, 0.0f, NULL);
        mNose = new ModelPartSkin(this, L"nose", modelDefinition, NULL, 0.0f, NULL);
    }
    else
    {
        setMapTex(L"head.main", 0, 0);
        setMapTex(L"head.nose", 0, 24);
        setMapTex(L"head.ear1", 0, 16);
        setMapTex(L"head.ear2", 6, 16);

        mRearFootLeft = new ModelPartSkin(this, 26, 24, NULL);
        mRearFootLeft->addBox(-1.0f, 5.5f, -3.7f, 2, 1, 7);
        mRearFootLeft->setPos(3.0f, 17.5f, 3.7f);
        mRearFootLeft->mMirror = true;
        setRotation(mRearFootLeft, 0.0f, 0.0f, 0.0f);

        mRearFootRight = new ModelPartSkin(this, 8, 24, NULL);
        mRearFootRight->addBox(-1.0f, 5.5f, -3.7f, 2, 1, 7);
        mRearFootRight->setPos(-3.0f, 17.5f, 3.7f);
        mRearFootRight->mMirror = true;
        setRotation(mRearFootRight, 0.0f, 0.0f, 0.0f);

        mHaunchLeft = new ModelPartSkin(this, 30, 15, NULL);
        mHaunchLeft->addBox(-1.0f, 0.0f, 0.0f, 2, 4, 5);
        mHaunchLeft->setPos(3.0f, 17.5f, 3.7f);
        mHaunchLeft->mMirror = true;
        setRotation(mHaunchLeft, -20.0f * 0.0174532924f, 0.0f, 0.0f);

        mHaunchRight = new ModelPartSkin(this, 16, 15, NULL);
        mHaunchRight->addBox(-1.0f, 0.0f, 0.0f, 2, 4, 5);
        mHaunchRight->setPos(-3.0f, 17.5f, 3.7f);
        mHaunchRight->mMirror = true;
        setRotation(mHaunchRight, -20.0f * 0.0174532924f, 0.0f, 0.0f);

        mBody = new ModelPartSkin(this, 0, 0, NULL);
        mBody->addBox(-3.0f, -2.0f, -10.0f, 6, 5, 10);
        mBody->setPos(0.0f, 19.0f, 8.0f);
        mBody->mMirror = true;
        setRotation(mBody, -20.0f * 0.0174532924f, 0.0f, 0.0f);

        mFrontLegLeft = new ModelPartSkin(this, 8, 15, NULL);
        mFrontLegLeft->addBox(-1.0f, 0.0f, -1.0f, 2, 7, 2);
        mFrontLegLeft->setPos(3.0f, 17.0f, -1.0f);
        mFrontLegLeft->mMirror = true;
        setRotation(mFrontLegLeft, -10.0f * 0.0174532924f, 0.0f, 0.0f);

        mFrontLegRight = new ModelPartSkin(this, 0, 15, NULL);
        mFrontLegRight->addBox(-1.0f, 0.0f, -1.0f, 2, 7, 2);
        mFrontLegRight->setPos(-3.0f, 17.0f, -1.0f);
        mFrontLegRight->mMirror = true;
        setRotation(mFrontLegRight, -10.0f * 0.0174532924f, 0.0f, 0.0f);

        mHead = new ModelPartSkin(this, 32, 0, NULL);
        mHead->addBox(-2.5f, -4.0f, -5.0f, 5, 4, 5);
        mHead->setPos(0.0f, 16.0f, -1.0f);
        mHead->mMirror = true;
        setRotation(mHead, 0.0f, 0.0f, 0.0f);

        mEarRight = new ModelPartSkin(this, 52, 0, NULL);
        mEarRight->addBox(-2.5f, -9.0f, -1.0f, 2, 5, 1);
        mEarRight->setPos(0.0f, 16.0f, -1.0f);
        mEarRight->mMirror = true;
        setRotation(mEarRight, 0.0f, -15.0f * 0.0174532924f, 0.0f);

        mEarLeft = new ModelPartSkin(this, 58, 0, NULL);
        mEarLeft->addBox(0.5f, -9.0f, -1.0f, 2, 5, 1);
        mEarLeft->setPos(0.0f, 16.0f, -1.0f);
        mEarLeft->mMirror = true;
        setRotation(mEarLeft, 0.0f, 15.0f * 0.0174532924f, 0.0f);

        mTail = new ModelPartSkin(this, 52, 6, NULL);
        mTail->addBox(-1.5f, -1.5f, 0.0f, 3, 3, 2);
        mTail->setPos(0.0f, 20.0f, 7.0f);
        mTail->mMirror = true;
        setRotation(mTail, -0.3490659f, 0.0f, 0.0f);

        mNose = new ModelPartSkin(this, 32, 9, NULL);
        mNose->addBox(-0.5f, -2.5f, -5.5f, 3, 3, 2);
        mNose->setPos(0.0f, 16.0f, -1.0f);
        mNose->mMirror = true;
        setRotation(mNose, 0.0f, 0.0f, 0.0f);
    }

    compileSkinnedDrawlist();
}

RabbitModel::~RabbitModel()
{
}

void RabbitModel::setRotation(ModelPartSkin* part, float x, float y, float z)
{
    part->mXRot = x;
    part->mYRot = y;
    part->mZRot = z;
}

void RabbitModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                         float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    if (!mYoung)
    {
        GlStateManager::pushMatrix();
        GlStateManager::scalef(0.6f, 0.6f, 0.6f);
        GlStateManager::translatef(0.0f, 16.0f * scale, 0.0f);
        mRearFootLeft->render(scale, useDisplayList, false);
        mRearFootRight->render(scale, useDisplayList, false);
        mHaunchLeft->render(scale, useDisplayList, false);
        mHaunchRight->render(scale, useDisplayList, false);
        mBody->render(scale, useDisplayList, false);
        mFrontLegLeft->render(scale, useDisplayList, false);
        mFrontLegRight->render(scale, useDisplayList, false);
        mHead->render(scale, useDisplayList, false);
        mEarRight->render(scale, useDisplayList, false);
        mEarLeft->render(scale, useDisplayList, false);
        mTail->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();
        callSkinnedDrawlist();
    }
    else
    {
        GlStateManager::pushMatrix();
        GlStateManager::scalef(0.56666666f, 0.56666666f, 0.56666666f);
        GlStateManager::translatef(0.0f, 22.0f * scale, 2.0f * scale);
        mHead->render(scale, useDisplayList, false);
        mEarRight->render(scale, useDisplayList, false);
        mEarLeft->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();

        GlStateManager::pushMatrix();
        GlStateManager::scalef(0.4f, 0.4f, 0.4f);
        GlStateManager::translatef(0.0f, 36.0f * scale, 0.0f);
        mRearFootLeft->render(scale, useDisplayList, false);
        mRearFootRight->render(scale, useDisplayList, false);
        mHaunchLeft->render(scale, useDisplayList, false);
        mHaunchRight->render(scale, useDisplayList, false);
        mBody->render(scale, useDisplayList, false);
        mFrontLegLeft->render(scale, useDisplayList, false);
        mFrontLegRight->render(scale, useDisplayList, false);
        mTail->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();
        callSkinnedDrawlist();
    }
}

void RabbitModel::render(RabbitModel* model, float scale, bool useDisplayList)
{
    model->callSkinnedDrawlist();
    model->mRearFootLeft->render(scale, useDisplayList, false);
    model->mRearFootRight->render(scale, useDisplayList, false);
    model->mHaunchLeft->render(scale, useDisplayList, false);
    model->mHaunchRight->render(scale, useDisplayList, false);
    model->mBody->render(scale, useDisplayList, false);
    model->mFrontLegLeft->render(scale, useDisplayList, false);
    model->mFrontLegRight->render(scale, useDisplayList, false);
    model->mHead->render(scale, useDisplayList, false);
    model->mEarRight->render(scale, useDisplayList, false);
    model->mEarLeft->render(scale, useDisplayList, false);
    model->mTail->render(scale, useDisplayList, false);
    model->mNose->render(scale, useDisplayList, false);
}

void RabbitModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                            float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    boost::shared_ptr<Rabbit> rabbit = boost::dynamic_pointer_cast<Rabbit>(entity);
    float jump = Mth::sin(rabbit->getJumpCompletion(ageInTicks - (float)rabbit->mTickCount) * 3.1415927f);
    mJumpRotation = jump;

    mEarRight->mXRot = headPitch * 0.0174532924f;
    mEarLeft->mXRot = headPitch * 0.0174532924f;
    mHead->mXRot = headPitch * 0.0174532924f;
    mNose->mXRot = headPitch * 0.0174532924f;

    mNose->mYRot = netHeadYaw * 0.0174532924f;
    mHead->mYRot = netHeadYaw * 0.0174532924f;
    mEarRight->mYRot = mNose->mYRot - 15.0f * 0.0174532924f;
    mEarLeft->mYRot = mNose->mYRot + 15.0f * 0.0174532924f;

    mRearFootLeft->mXRot = jump * 50.0f * 0.0174532924f;
    mRearFootRight->mXRot = jump * 50.0f * 0.0174532924f;
    mHaunchLeft->mXRot = (jump * 50.0f - 21.0f) * 0.0174532924f;
    mHaunchRight->mXRot = (jump * 50.0f - 21.0f) * 0.0174532924f;
    mJumpRotation = jump;
    mFrontLegLeft->mXRot = (jump * -40.0f - 11.0f) * 0.0174532924f;
    mFrontLegRight->mXRot = (jump * -40.0f - 11.0f) * 0.0174532924f;
}

void RabbitModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                  float partialTicks)
{
    Model::prepareMobModel(entity, limbSwing, limbSwingAmount, partialTicks);
    boost::shared_ptr<Rabbit> rabbit = boost::dynamic_pointer_cast<Rabbit>(entity);
    mJumpRotation = Mth::sin(rabbit->getJumpCompletion(partialTicks) * 3.1415927f);
}

std::wstring RabbitModel::getName()
{
    std::wstring result;
    result.assign(L"rabbit");
    return result;
}
