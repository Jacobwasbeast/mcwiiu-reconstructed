#include "game/client/render/OcelotModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Ocelot.hpp"

OcelotModel::OcelotModel()
    : mBackLegL(NULL), mBackLegR(NULL), mFrontLegL(NULL), mFrontLegR(NULL),
      mTail1(NULL), mTail2(NULL), mHead(NULL), mBody(NULL), mPose(0)
{
    ModelDefinition modelDefinition;
    LoadOcelotModelDefinition(modelDefinition);
}

OcelotModel::OcelotModel(const ModelDefinition& modelDefinition)
    : mBackLegL(NULL), mBackLegR(NULL), mFrontLegL(NULL), mFrontLegR(NULL),
      mTail1(NULL), mTail2(NULL), mHead(NULL), mBody(NULL), mPose(0)
{
    LoadOcelotModelDefinition(modelDefinition);
}

OcelotModel::~OcelotModel()
{
}

void OcelotModel::LoadOcelotModelDefinition(const ModelDefinition& modelDefinition)
{
    mPose = 1;
    Tesselator::getInstance()->getBuilder();

    if (modelDefinition.DoesOverrideModel())
    {
        mBackLegL = new ModelPartSkin(this, L"backLegL", modelDefinition, NULL, 0.0f, NULL);
        mBackLegR = new ModelPartSkin(this, L"backLegR", modelDefinition, NULL, 0.0f, NULL);
        mFrontLegL = new ModelPartSkin(this, L"frontLegL", modelDefinition, NULL, 0.0f, NULL);
        mFrontLegR = new ModelPartSkin(this, L"frontLegR", modelDefinition, NULL, 0.0f, NULL);
        mTail1 = new ModelPartSkin(this, L"tail1", modelDefinition, NULL, 0.0f, NULL);
        mTail2 = new ModelPartSkin(this, L"tail2", modelDefinition, NULL, 0.0f, NULL);
        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, NULL);
        mBody = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);
    }
    else
    {
        mHead = new ModelPartSkin(this, 0, 0, NULL);
        mHead->addBox(-2.0f, -3.0f, -2.0f, 5, 4, 5, 0.0f);
        mHead->setPos(0.0f, 15.0f, -9.0f);

        mBody = new ModelPartSkin(this, 20, 0, NULL);
        mBody->addBox(-2.0f, -2.0f, -3.0f, 4, 16, 6, 0.0f);
        mBody->setPos(0.0f, 12.0f, -10.0f);

        mTail1 = new ModelPartSkin(this, 0, 9, NULL);
        mTail1->addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f);
        mTail1->setPos(0.0f, 15.0f, 0.0f);

        mTail2 = new ModelPartSkin(this, 4, 9, NULL);
        mTail2->addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f);
        mTail2->setPos(0.0f, 20.0f, 7.0f);

        mBackLegL = new ModelPartSkin(this, 8, 13, NULL);
        mBackLegL->addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f);
        mBackLegL->setPos(-2.0f, 16.0f, 7.0f);

        mBackLegR = new ModelPartSkin(this, 8, 13, NULL);
        mBackLegR->addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f);
        mBackLegR->setPos(2.0f, 16.0f, 7.0f);

        mFrontLegL = new ModelPartSkin(this, 8, 13, NULL);
        mFrontLegL->addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f);
        mFrontLegL->setPos(-2.0f, 16.0f, -5.0f);

        mFrontLegR = new ModelPartSkin(this, 8, 13, NULL);
        mFrontLegR->addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f);
        mFrontLegR->setPos(2.0f, 16.0f, -5.0f);
    }

    compileSkinnedDrawlist();
    Tesselator::getInstance()->getBuilder()->useDefaultShaders();
}

void OcelotModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                         float ageInTicks, float netHeadYaw, float headPitch, float scale,
                         bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale,
              entity, _SkinAdjustments());

    if (!mYoung)
    {
        mHead->render(scale, useDisplayList, false);
        mBody->render(scale, useDisplayList, false);
        mTail1->render(scale, useDisplayList, false);
        mTail2->render(scale, useDisplayList, false);
        mBackLegL->render(scale, useDisplayList, false);
        mBackLegR->render(scale, useDisplayList, false);
        mFrontLegL->render(scale, useDisplayList, false);
        mFrontLegR->render(scale, useDisplayList, false);
        callSkinnedDrawlist();
        return;
    }

    GlStateManager::pushMatrix();
    GlStateManager::scalef(0.75f, 0.75f, 0.75f);
    GlStateManager::translatef(0.0f, 10.0f * scale, 4.0f * scale);
    mHead->render(scale, useDisplayList, false);
    GlStateManager::popMatrix();
    GlStateManager::pushMatrix();
    GlStateManager::scalef(0.5f, 0.5f, 0.5f);
    GlStateManager::translatef(0.0f, 24.0f * scale, 0.0f);
    mBody->render(scale, useDisplayList, false);
    mBackLegL->render(scale, useDisplayList, false);
    mBackLegR->render(scale, useDisplayList, false);
    mFrontLegL->render(scale, useDisplayList, false);
    mFrontLegR->render(scale, useDisplayList, false);
    mTail1->render(scale, useDisplayList, false);
    mTail2->render(scale, useDisplayList, false);
    GlStateManager::popMatrix();
    callSkinnedDrawlist();
}

void OcelotModel::render(OcelotModel* model, float scale, bool useDisplayList)
{
    mHead->mYRot = model->mHead->mYRot;
    mHead->mXRot = model->mHead->mXRot;
    mHead->mY = model->mHead->mY;
    mHead->mX = model->mHead->mX;
    mBody->mYRot = model->mBody->mYRot;
    mBody->mXRot = model->mBody->mXRot;
    mTail1->mYRot = model->mBody->mYRot;
    mTail1->mY = model->mBody->mY;
    mTail1->mX = model->mBody->mX;
    mTail1->render(scale, useDisplayList, false);
    mTail2->mYRot = model->mBody->mYRot;
    mTail2->mY = model->mBody->mY;
    mTail2->mX = model->mBody->mX;
    mTail2->render(scale, useDisplayList, false);
    mBackLegL->mXRot = model->mBackLegL->mXRot;
    mBackLegR->mXRot = model->mBackLegR->mXRot;
    mBackLegL->render(scale, useDisplayList, false);
    mBackLegR->render(scale, useDisplayList, false);
    mFrontLegL->mXRot = model->mFrontLegL->mXRot;
    mFrontLegR->mXRot = model->mFrontLegR->mXRot;
    mFrontLegL->render(scale, useDisplayList, false);
    mFrontLegR->render(scale, useDisplayList, false);
    mHead->render(scale, useDisplayList, false);
    mBody->render(scale, useDisplayList, false);
}

void OcelotModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks,
                            float netHeadYaw, float headPitch, float scale,
                            boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    mHead->mYRot = netHeadYaw * 0.017453292f;
    mHead->mXRot = headPitch * 0.017453292f;

    if (mPose != 3)
    {
        mBackLegL->mXRot = Mth::cos(limbSwing * 0.6662f) * 1.0f * limbSwingAmount;
        if (mPose == 2)
        {
            mBackLegR->mXRot = Mth::cos(limbSwing * 0.6662f + 0.3f) * limbSwingAmount;
            mFrontLegL->mXRot = Mth::cos(limbSwing * 0.6662f + 3.4415927f) * limbSwingAmount;
            mFrontLegR->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * limbSwingAmount;
            mTail2->mXRot = 0.31415927f * Mth::cos(limbSwing) * limbSwingAmount + 1.7278761f;
        }
        else
        {
            mBackLegR->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * limbSwingAmount;
            mFrontLegL->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * limbSwingAmount;
            mFrontLegR->mXRot = Mth::cos(limbSwing * 0.6662f) * limbSwingAmount;
            mTail2->mXRot = (mPose == 1 ? 0.7853982f : 0.47123894f) * Mth::cos(limbSwing) * limbSwingAmount + 1.7278761f;
        }
    }
}

void OcelotModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                                  float limbSwingAmount, float partialTicks)
{
    boost::shared_ptr<Ocelot> ocelot = boost::dynamic_pointer_cast<Ocelot>(entity);

    mBody->mY = 0.0f;
    mBody->mZ = 0.0f;
    mHead->mY = 0.0f;
    mHead->mZ = 0.0f;
    mTail1->mY = 0.0f;
    mTail1->mZ = 0.0f;
    mTail2->mY = 0.0f;
    mTail2->mZ = 0.0f;
    mFrontLegR->mY = 0.0f;
    mFrontLegR->mZ = 0.0f;
    mFrontLegL->mY = 0.0f;
    mFrontLegL->mZ = 0.0f;
    mBackLegR->mY = 0.0f;
    mBackLegR->mZ = 0.0f;
    mBackLegL->mY = 0.0f;
    mBackLegL->mZ = 0.0f;
    mTail1->mXRot = 0.8999999761581421f;
    mBody->mXRot = 0.0f;

    if (ocelot->isSneaking())
    {
        mBody->mY += 1.0f;
        mHead->mY += 2.0f;
        mTail1->mY += 1.0f;
        mTail2->mY += -4.0f;
        mTail2->mZ += 2.0f;
        mTail1->mXRot = 1.5707963705062866f;
        mTail2->mXRot = 1.5707963705062866f;
        mPose = 0;
    }
    else if (ocelot->isSprinting())
    {
        mTail2->mY = mTail1->mY;
        mTail2->mZ += 2.0f;
        mTail1->mXRot = 1.5707963705062866f;
        mTail2->mXRot = 1.5707963705062866f;
        mPose = 2;
    }
    else
    {

        if (ocelot->isSitting())
        {
            mBody->mY += -4.0f;
            mBody->mXRot = -45.0f * 0.01745329238474369f;
            mBody->mZ += 5.0f;
            mHead->mY += -3.299999952316284f;
            mHead->mZ += 1.0f;
            mTail1->mY += 8.0f;
            mTail1->mZ += -2.0f;
            mTail1->mXRot = 1.72787606716156f;
            mTail2->mY += 2.0f;
            mTail2->mZ += -0.800000011920929f;
            mTail2->mXRot = 2.670353889465332f;
            mFrontLegL->mXRot = -0.15707963705062866f;
            mFrontLegL->mY = 2.0f;
            mFrontLegL->mZ = -7.0f;
            mFrontLegR->mXRot = -0.15707963705062866f;
            mFrontLegR->mY = 2.0f;
            mFrontLegR->mZ = -7.0f;
            mBackLegL->mXRot = -1.5707963705062866f;
            mBackLegL->mY = 3.0f;
            mBackLegL->mZ = -4.0f;
            mBackLegR->mXRot = -1.5707963705062866f;
            mBackLegR->mY = 3.0f;
            mBackLegR->mZ = -4.0f;
            mPose = 3;
        }
        else
        {
            mPose = 1;
        }
    }
}

std::wstring OcelotModel::getName()
{
    return L"ocelot";
}
