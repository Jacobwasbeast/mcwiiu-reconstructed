#include "game/client/render/SpiderModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Mth.hpp"
#include "game/world/block/MaterialTable.hpp"
#include "game/entity/mob/Entity.hpp"

#include <cmath>

SpiderModel::SpiderModel(const ModelDefinition& modelDefinition)
    : Model(), mSpiderParts(new ModelPartSkin*[22]()), mSpiderPartCount(22), mInvisibleParts()
{
    createModelParts(modelDefinition);
    createInvisibleModelParts(modelDefinition);
}

SpiderModel::~SpiderModel()
{
    delete[] mSpiderParts;
}

void SpiderModel::createModelParts(const ModelDefinition& modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mHasOverriddenParts = true;
        Tesselator::getInstance()->getBuilder()->setVertexAndPixelShader(eMinecraftEntityMaterial_Spider);
        mSpiderParts[0] = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, NULL);
        mSpiderParts[1] = new ModelPartSkin(this, L"body0", modelDefinition, NULL, 0.0f, NULL);
        mSpiderParts[2] = new ModelPartSkin(this, L"body1", modelDefinition, NULL, 0.0f, NULL);
        mSpiderParts[3] = new ModelPartSkin(this, L"leg0", modelDefinition, NULL, 0.0f, NULL);
        mSpiderParts[4] = new ModelPartSkin(this, L"leg1", modelDefinition, NULL, 0.0f, NULL);
        mSpiderParts[5] = new ModelPartSkin(this, L"leg2", modelDefinition, NULL, 0.0f, NULL);
        mSpiderParts[6] = new ModelPartSkin(this, L"leg3", modelDefinition, NULL, 0.0f, NULL);
        mSpiderParts[7] = new ModelPartSkin(this, L"leg4", modelDefinition, NULL, 0.0f, NULL);
        mSpiderParts[8] = new ModelPartSkin(this, L"leg5", modelDefinition, NULL, 0.0f, NULL);
        mSpiderParts[9] = new ModelPartSkin(this, L"leg6", modelDefinition, NULL, 0.0f, NULL);
        mSpiderParts[10] = new ModelPartSkin(this, L"leg7", modelDefinition, NULL, 0.0f, NULL);
    }
    else
    {
        mSpiderParts[0] = new ModelPartSkin(this, 32, 4, NULL);
        mSpiderParts[0]->addBox(-4.0f, -4.0f, -8.0f, 8, 8, 8, 0.0f);
        mSpiderParts[0]->setPos(0.0f, 15.0f, -3.0f);
        mSpiderParts[1] = new ModelPartSkin(this, 0, 0, NULL);
        mSpiderParts[1]->addBox(-3.0f, -3.0f, -3.0f, 6, 6, 6, 0.0f);
        mSpiderParts[1]->setPos(0.0f, 15.0f, 0.0f);
        mSpiderParts[2] = new ModelPartSkin(this, 0, 12, NULL);
        mSpiderParts[2]->addBox(-5.0f, -4.0f, -6.0f, 10, 8, 12, 0.0f);
        mSpiderParts[2]->setPos(0.0f, 15.0f, 9.0f);
        mSpiderParts[3] = new ModelPartSkin(this, 18, 0, NULL);
        mSpiderParts[3]->addBox(-15.0f, -1.0f, -1.0f, 16, 2, 2, 0.0f);
        mSpiderParts[3]->setPos(-4.0f, 15.0f, 2.0f);
        mSpiderParts[4] = new ModelPartSkin(this, 18, 0, NULL);
        mSpiderParts[4]->addBox(-1.0f, -1.0f, -1.0f, 16, 2, 2, 0.0f);
        mSpiderParts[4]->setPos(4.0f, 15.0f, 2.0f);
        mSpiderParts[5] = new ModelPartSkin(this, 18, 0, NULL);
        mSpiderParts[5]->addBox(-15.0f, -1.0f, -1.0f, 16, 2, 2, 0.0f);
        mSpiderParts[5]->setPos(-4.0f, 15.0f, 1.0f);
        mSpiderParts[6] = new ModelPartSkin(this, 18, 0, NULL);
        mSpiderParts[6]->addBox(-1.0f, -1.0f, -1.0f, 16, 2, 2, 0.0f);
        mSpiderParts[6]->setPos(4.0f, 15.0f, 1.0f);
        mSpiderParts[7] = new ModelPartSkin(this, 18, 0, NULL);
        mSpiderParts[7]->addBox(-15.0f, -1.0f, -1.0f, 16, 2, 2, 0.0f);
        mSpiderParts[7]->setPos(-4.0f, 15.0f, 0.0f);
        mSpiderParts[8] = new ModelPartSkin(this, 18, 0, NULL);
        mSpiderParts[8]->addBox(-1.0f, -1.0f, -1.0f, 16, 2, 2, 0.0f);
        mSpiderParts[8]->setPos(4.0f, 15.0f, 0.0f);
        mSpiderParts[9] = new ModelPartSkin(this, 18, 0, NULL);
        mSpiderParts[9]->addBox(-15.0f, -1.0f, -1.0f, 16, 2, 2, 0.0f);
        mSpiderParts[9]->setPos(-4.0f, 15.0f, -1.0f);
        mSpiderParts[10] = new ModelPartSkin(this, 18, 0, NULL);
        mSpiderParts[10]->addBox(-1.0f, -1.0f, -1.0f, 16, 2, 2, 0.0f);
        mSpiderParts[10]->setPos(4.0f, 15.0f, -1.0f);
        compileSkinnedDrawlist();
        Tesselator::getInstance()->getBuilder()->useDefaultShaders();
    }
}

void SpiderModel::createInvisibleModelParts(const ModelDefinition& modelDefinition)
{
    if (!modelDefinition.DoesOverrideModel())
        return;
    Tesselator::getInstance()->getBuilder()->setVertexAndPixelShader(eMinecraftEntityMaterial_SpiderInvisible);
    mSpiderParts[11] = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, &mInvisibleParts);
    mSpiderParts[12] = new ModelPartSkin(this, L"body0", modelDefinition, NULL, 0.0f, &mInvisibleParts);
    mSpiderParts[13] = new ModelPartSkin(this, L"body1", modelDefinition, NULL, 0.0f, &mInvisibleParts);
    mSpiderParts[14] = new ModelPartSkin(this, L"leg0", modelDefinition, NULL, 0.0f, &mInvisibleParts);
    mSpiderParts[15] = new ModelPartSkin(this, L"leg1", modelDefinition, NULL, 0.0f, &mInvisibleParts);
    mSpiderParts[16] = new ModelPartSkin(this, L"leg2", modelDefinition, NULL, 0.0f, &mInvisibleParts);
    mSpiderParts[17] = new ModelPartSkin(this, L"leg3", modelDefinition, NULL, 0.0f, &mInvisibleParts);
    mSpiderParts[18] = new ModelPartSkin(this, L"leg4", modelDefinition, NULL, 0.0f, &mInvisibleParts);
    mSpiderParts[19] = new ModelPartSkin(this, L"leg5", modelDefinition, NULL, 0.0f, &mInvisibleParts);
    mSpiderParts[20] = new ModelPartSkin(this, L"leg6", modelDefinition, NULL, 0.0f, &mInvisibleParts);
    mSpiderParts[21] = new ModelPartSkin(this, L"leg7", modelDefinition, NULL, 0.0f, &mInvisibleParts);
    mInvisibleParts.compile(0.0625f);
    Tesselator::getInstance()->getBuilder()->useDefaultShaders();
}

void SpiderModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                         float ageInTicks, float netHeadYaw, float headPitch, float scale,
                         bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());
    int i;
    if (!entity->isInvisible())
    {
        for (i = 0; i < 11; ++i)
            mSpiderParts[i]->render(scale, useDisplayList, false);
        callSkinnedDrawlist();
    }
    else if (hasOverriddenParts())
    {
        for (i = 11; i < 22; ++i)
            mSpiderParts[i]->render(scale, useDisplayList, false);
        mInvisibleParts.render();
    }
}

void SpiderModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks,
                            float netHeadYaw, float headPitch, float scale,
                            boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    const float radians = 0.01745329238474369f;
    mSpiderParts[0]->mYRot = netHeadYaw * radians;
    mSpiderParts[0]->mXRot = headPitch * radians;
    const float quarterPi = 0.785398185f;
    const float middleRoll = 0.58119464f;
    const float eighthPi = 0.39269909f;
    mSpiderParts[3]->mZRot = -quarterPi;
    mSpiderParts[4]->mZRot = quarterPi;
    mSpiderParts[5]->mZRot = -middleRoll;
    mSpiderParts[6]->mZRot = middleRoll;
    mSpiderParts[7]->mZRot = -middleRoll;
    mSpiderParts[8]->mZRot = middleRoll;
    mSpiderParts[9]->mZRot = -quarterPi;
    mSpiderParts[10]->mZRot = quarterPi;
    mSpiderParts[3]->mYRot = quarterPi;
    mSpiderParts[4]->mYRot = -quarterPi;
    mSpiderParts[5]->mYRot = eighthPi;
    mSpiderParts[6]->mYRot = -eighthPi;
    mSpiderParts[7]->mYRot = -eighthPi;
    mSpiderParts[8]->mYRot = eighthPi;
    mSpiderParts[9]->mYRot = -quarterPi;
    mSpiderParts[10]->mYRot = quarterPi;
    const float swing = limbSwing * 0.6662f;
    const float yaw0 = -(Mth::cos(swing * 2.0f + 0.0f) * 0.4f) * limbSwingAmount;
    const float yaw1 = -(Mth::cos(swing * 2.0f + 3.141592741f) * 0.4f) * limbSwingAmount;
    const float yaw2 = -(Mth::cos(swing * 2.0f + 1.57079637f) * 0.4f) * limbSwingAmount;
    const float yaw3 = -(Mth::cos(swing * 2.0f + 4.71238899f) * 0.4f) * limbSwingAmount;
    const float roll0 = std::fabs(Mth::sin(swing + 0.0f) * 0.4f) * limbSwingAmount;
    const float roll1 = std::fabs(Mth::sin(swing + 3.141592741f) * 0.4f) * limbSwingAmount;
    const float roll2 = std::fabs(Mth::sin(swing + 1.57079637f) * 0.4f) * limbSwingAmount;
    const float roll3 = std::fabs(Mth::sin(swing + 4.71238899f) * 0.4f) * limbSwingAmount;
    mSpiderParts[3]->mYRot += yaw0;
    mSpiderParts[4]->mYRot -= yaw0;
    mSpiderParts[5]->mYRot += yaw1;
    mSpiderParts[6]->mYRot -= yaw1;
    mSpiderParts[7]->mYRot += yaw2;
    mSpiderParts[8]->mYRot -= yaw2;
    mSpiderParts[9]->mYRot += yaw3;
    mSpiderParts[10]->mYRot -= yaw3;
    mSpiderParts[3]->mZRot += roll0;
    mSpiderParts[4]->mZRot -= roll0;
    mSpiderParts[5]->mZRot += roll1;
    mSpiderParts[6]->mZRot -= roll1;
    mSpiderParts[7]->mZRot += roll2;
    mSpiderParts[8]->mZRot -= roll2;
    mSpiderParts[9]->mZRot += roll3;
    mSpiderParts[10]->mZRot -= roll3;
}

