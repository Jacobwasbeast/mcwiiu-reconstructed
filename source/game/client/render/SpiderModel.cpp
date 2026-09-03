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
        mSpiderParts[1]->addBox(-4.0f, -3.0f, -2.0f, 8, 8, 4, 0.0f);
        mSpiderParts[1]->setPos(0.0f, 15.0f, 0.0f);
        mSpiderParts[2] = new ModelPartSkin(this, 0, 12, NULL);
        mSpiderParts[2]->addBox(-4.0f, -3.0f, -2.0f, 8, 8, 4, 0.0f);
        mSpiderParts[2]->setPos(0.0f, 15.0f, 0.0f);
        mSpiderParts[3] = new ModelPartSkin(this, 18, 0, NULL);
        mSpiderParts[4] = new ModelPartSkin(this, 18, 0, NULL);
        mSpiderParts[5] = new ModelPartSkin(this, 18, 0, NULL);
        mSpiderParts[6] = new ModelPartSkin(this, 18, 0, NULL);
        mSpiderParts[7] = new ModelPartSkin(this, 18, 0, NULL);
        mSpiderParts[8] = new ModelPartSkin(this, 18, 0, NULL);
        mSpiderParts[9] = new ModelPartSkin(this, 18, 0, NULL);
        mSpiderParts[10] = new ModelPartSkin(this, 18, 0, NULL);
        int i;
        for (i = 3; i <= 10; ++i)
        {
            mSpiderParts[i]->addBox(-1.0f, -1.0f, -1.0f, 16, 2, 2, 0.0f);
        }
        mSpiderParts[3]->setPos(-4.0f, 15.0f, 9.0f);
        mSpiderParts[4]->setPos(4.0f, 15.0f, 9.0f);
        mSpiderParts[5]->setPos(-4.0f, 15.0f, 4.0f);
        mSpiderParts[6]->setPos(4.0f, 15.0f, 4.0f);
        mSpiderParts[7]->setPos(-4.0f, 15.0f, -1.0f);
        mSpiderParts[8]->setPos(4.0f, 15.0f, -1.0f);
        mSpiderParts[9]->setPos(-4.0f, 15.0f, -6.0f);
        mSpiderParts[10]->setPos(4.0f, 15.0f, -6.0f);
        mSpiderParts[3]->mYRot = -0.785398185f;
        mSpiderParts[4]->mYRot = 0.785398185f;
        mSpiderParts[5]->mYRot = -0.58119464f;
        mSpiderParts[6]->mYRot = 0.58119464f;
        mSpiderParts[7]->mYRot = -0.39269909f;
        mSpiderParts[8]->mYRot = 0.39269909f;
        mSpiderParts[9]->mYRot = -0.19634955f;
        mSpiderParts[10]->mYRot = 0.19634955f;
        mSpiderParts[3]->mZRot = 0.785398185f;
        mSpiderParts[4]->mZRot = -0.785398185f;
        mSpiderParts[5]->mZRot = 0.39269909f;
        mSpiderParts[6]->mZRot = -0.39269909f;
        mSpiderParts[7]->mZRot = 0.19634955f;
        mSpiderParts[8]->mZRot = -0.19634955f;
        mSpiderParts[9]->mZRot = 0.0f;
        mSpiderParts[10]->mZRot = 0.0f;
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
    const float swing = limbSwing * 0.6661999821662903f;
    const float headSwing = Mth::cos(swing) * 0.4f * limbSwingAmount;
    const float oppositeSwing = Mth::cos(swing + 3.141592741f) * 0.4f * limbSwingAmount;
    const float quarterSwing = Mth::cos(swing + 1.57079637f) * 0.4f * limbSwingAmount;
    const float threeQuarterSwing = Mth::cos(swing + 4.71238899f) * 0.4f * limbSwingAmount;
    const float z0 = static_cast<float>(fabs(Mth::sin(limbSwing * 0.666199982f) * 0.4f * limbSwingAmount));
    const float z1 = static_cast<float>(fabs(Mth::sin(limbSwing * 0.666199982f + 3.141592741f) * 0.4f * limbSwingAmount));
    const float z2 = static_cast<float>(fabs(Mth::sin(limbSwing * 0.666199982f + 1.57079637f) * 0.4f * limbSwingAmount));
    const float z3 = static_cast<float>(fabs(Mth::sin(limbSwing * 0.666199982f + 4.71238899f) * 0.4f * limbSwingAmount));
    int i;
    mSpiderParts[3]->mYRot = -0.785398185f;
    mSpiderParts[4]->mYRot = 0.785398185f;
    mSpiderParts[5]->mYRot = -0.58119464f;
    mSpiderParts[6]->mYRot = 0.58119464f;
    mSpiderParts[7]->mYRot = -0.39269909f;
    mSpiderParts[8]->mYRot = 0.39269909f;
    mSpiderParts[9]->mYRot = -0.19634955f;
    mSpiderParts[10]->mYRot = 0.19634955f;
    mSpiderParts[3]->mZRot = 0.785398185f;
    mSpiderParts[4]->mZRot = -0.785398185f;
    mSpiderParts[5]->mZRot = 0.39269909f;
    mSpiderParts[6]->mZRot = -0.39269909f;
    mSpiderParts[7]->mZRot = 0.19634955f;
    mSpiderParts[8]->mZRot = -0.19634955f;
    mSpiderParts[9]->mZRot = 0.0f;
    mSpiderParts[10]->mZRot = 0.0f;
    mSpiderParts[3]->mYRot += headSwing;
    mSpiderParts[4]->mYRot -= headSwing;
    mSpiderParts[5]->mYRot += oppositeSwing;
    mSpiderParts[6]->mYRot -= oppositeSwing;
    mSpiderParts[7]->mYRot += quarterSwing;
    mSpiderParts[8]->mYRot -= quarterSwing;
    mSpiderParts[9]->mYRot += threeQuarterSwing;
    mSpiderParts[10]->mYRot -= threeQuarterSwing;
    mSpiderParts[3]->mZRot += z0;
    mSpiderParts[4]->mZRot -= z0;
    mSpiderParts[5]->mZRot += z1;
    mSpiderParts[6]->mZRot -= z1;
    mSpiderParts[7]->mZRot += z2;
    mSpiderParts[8]->mZRot -= z2;
    mSpiderParts[9]->mZRot += z3;
    mSpiderParts[10]->mZRot -= z3;
}

std::wstring SpiderModel::getName()
{
    return L"spider";
}
