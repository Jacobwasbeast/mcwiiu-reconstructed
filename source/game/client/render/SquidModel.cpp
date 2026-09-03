#include "game/client/render/SquidModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/Mth.hpp"

SquidModel::SquidModel(const ModelDefinition& modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mBody = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);
        mBody->setYOffset(-16.0f);

        mTentacles[0] = new ModelPartSkin(this, L"tentacle1", modelDefinition, NULL, 0.0f, NULL);
        mTentacles[1] = new ModelPartSkin(this, L"tentacle2", modelDefinition, NULL, 0.0f, NULL);
        mTentacles[2] = new ModelPartSkin(this, L"tentacle3", modelDefinition, NULL, 0.0f, NULL);
        mTentacles[3] = new ModelPartSkin(this, L"tentacle4", modelDefinition, NULL, 0.0f, NULL);
        mTentacles[4] = new ModelPartSkin(this, L"tentacle5", modelDefinition, NULL, 0.0f, NULL);
        mTentacles[5] = new ModelPartSkin(this, L"tentacle6", modelDefinition, NULL, 0.0f, NULL);
        mTentacles[6] = new ModelPartSkin(this, L"tentacle7", modelDefinition, NULL, 0.0f, NULL);
        mTentacles[7] = new ModelPartSkin(this, L"tentacle8", modelDefinition, NULL, 0.0f, NULL);

        mTentacles[0]->setYOffset(-16.0f);
        mTentacles[0]->mYRot = 1.5707964f;

        mTentacles[1]->setYOffset(-16.0f);
        mTentacles[1]->mYRot = 0.78539819f;

        mTentacles[2]->setYOffset(-16.0f);
        mTentacles[2]->mYRot = 0.0f;

        mTentacles[3]->setYOffset(-16.0f);
        mTentacles[3]->mYRot = -0.78539813f;

        mTentacles[4]->setYOffset(-16.0f);
        mTentacles[4]->mYRot = -1.5707964f;

        mTentacles[5]->setYOffset(-16.0f);
        mTentacles[5]->mYRot = -2.3561945f;

        mTentacles[6]->setYOffset(-16.0f);
        mTentacles[6]->mYRot = -3.1415925f;

        mTentacles[7]->setYOffset(-16.0f);
        mTentacles[7]->mYRot = -3.926991f;

        compileSkinnedDrawlist();
    }
    else
    {
        mBody = new ModelPartSkin(this, 0, 0, NULL);
        mBody->addBox(-6.0f, -8.0f, -6.0f, 12, 16, 12);
        mBody->mY += 8.0f;

        for (int i = 0; i < 8; ++i)
        {
            mTentacles[i] = new ModelPartSkin(this, 48, 0, NULL);

            float sweep = i * 3.1415927f;
            double angle = sweep * 2.0 / 8.0;

            mTentacles[i]->addBox(-1.0f, 0.0f, -1.0f, 2, 18, 2);
            mTentacles[i]->mX = Mth::cos((float)angle) * 5.0f;
            mTentacles[i]->mZ = Mth::sin((float)angle) * 5.0f;
            mTentacles[i]->mY = 15.0f;
            mTentacles[i]->mYRot = (float)(sweep * -2.0 / 8.0 + 3.1415927f / 2.0);
        }

        compileSkinnedDrawlist();
    }
}

SquidModel::~SquidModel()
{
}

void SquidModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    mBody->render(scale, useDisplayList, false);

    for (int i = 0; i < 8; ++i)
    {
        mTentacles[i]->render(scale, useDisplayList, false);
    }

    callSkinnedDrawlist();
}

void SquidModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    for (int i = 0; i < 8; ++i)
    {
        mTentacles[i]->mXRot = ageInTicks;
    }
}

std::wstring SquidModel::getName()
{
    return L"squid";
}
