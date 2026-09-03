#include "game/client/render/WitchModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/entity/mob/Entity.hpp"
#include <cmath>

WitchModel::WitchModel(float scale, const ModelDefinition& modelDefinition)
    : VillagerModel(scale, modelDefinition)
{
    if (!modelDefinition.DoesOverrideModel())
    {
        mHat = new ModelPartSkin(this);
        mHat->setTexSize(64, 128);
        mHat->setPos(0.0f, -4.0f, 0.0f);
        mHat->texOffs(0, 0)->addBox(-4.0f, -4.0f, -4.0f, 8, 2, 8);
        mHat2 = new ModelPartSkin(this);
        mHat2->setTexSize(64, 128);
        mHat2->setPos(0.0f, -10.03125f, 0.0f);
        mHat2->texOffs(0, 64)->addBox(-5.0f, 0.0f, -5.0f, 10, 2, 10);
        mHat3 = new ModelPartSkin(this);
        mHat3->setTexSize(64, 128);
        mHat3->setPos(1.75f, -6.75f, 2.0f);
        mHat3->texOffs(0, 76)->addBox(-3.5f, 0.0f, -3.5f, 7, 4, 7);
        mHat3->mXRot = -3.0f * 0.01745329238474369f;
        mHat3->mZRot = 1.5f * 0.01745329238474369f;
        mHat4 = new ModelPartSkin(this);
        mHat4->setTexSize(64, 128);
        mHat4->setPos(1.75f, -4.0f, 2.0f);
        mHat4->texOffs(0, 87)->addBox(-3.0f, 0.0f, -3.0f, 6, 2, 6);
        mHat4->mXRot = -12.0f * 0.01745329238474369f;
        mHat4->mZRot = 6.0f * 0.01745329238474369f;
        mWitchNose = new ModelPartSkin(this);
        mWitchNose->setTexSize(64, 128);
        mWitchNose->setPos(0.0f, -2.0f, 0.0f);
        mWitchNose->texOffs(0, 95)->addBox(0.0f, 0.0f, 0.25f, 1, 2, 1, 0.25f);
        mHead->addChild(mHat);
        mHat->addChild(mHat2);
        mHat2->addChild(mHat3);
        mHat3->addChild(mHat4);
        mHat4->addChild(mWitchNose);
        compileSkinnedDrawlist();
    }
    else
    {
        mHat = new ModelPartSkin(this, L"bottom", modelDefinition, NULL, 0.0f, NULL);
        mHat2 = new ModelPartSkin(this, L"hat", modelDefinition, NULL, 0.0f, NULL);
        mHat3 = new ModelPartSkin(this, L"hat2", modelDefinition, NULL, 0.0f, NULL);
        mHat4 = new ModelPartSkin(this, L"hat3", modelDefinition, NULL, 0.0f, NULL);
        mWitchNose = new ModelPartSkin(this, L"hat4", modelDefinition, NULL, 0.0f, NULL);
        mHead->addChild(mHat);
        mBody->addChild(mHat2);
        mHat2->addChild(mHat3);
        mHat3->addChild(mHat4);
        mHat4->addChild(mWitchNose);
        compileSkinnedDrawlist();
    }
}

WitchModel::~WitchModel()
{
}

void WitchModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    VillagerModel::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, skinAdjustments);
    mHead->mXRot = 0.0f;
    mHead->mYRot = 0.0f;
    mHead->mZRot = 0.0f;
    if (entity)
    {
        float phase = ((float)(entity->getId() % 10) + (float)entity->mTickCount - 176.0f) * 0.01f;
        mHead->mXRot = std::sin(phase) * 4.5f * 0.01745329238474369f;
        mHead->mYRot = std::cos(phase) * 2.5f * 0.01745329238474369f;
    }
    if (mHasCustomModel)
    {
        mHead->mXRot = -0.9f;
        mHead->mZRot = -0.09375f;
        mHead->mYRot = 0.0f;
    }
}

std::wstring WitchModel::getName()
{
    return L"villager_witch";
}

int WitchModel::getModelVersion()
{
    return 0x17;
}
