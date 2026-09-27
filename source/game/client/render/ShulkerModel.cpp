#include "game/client/render/ShulkerModel.hpp"

#include <cmath>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/ModelPartsSkinned.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"

ShulkerModel::ShulkerModel(const ModelDefinition& modelDefinition)
    : mBase(NULL), mLid(NULL), mHead(NULL), mHeadSkin(NULL), mSkinnedParts()
{
    mTexWidth = 64;
    mTexHeight = 128;

    if (!modelDefinition.DoesOverrideModel())
    {
        mLid = new ModelPart(this);
        mBase = new ModelPart(this);
        mHead = new ModelPart(this);
        mLid->texOffs(0, 0)->addBox(-16.0f, -8.0f, -16.0f, 16, 12, 16);
        mLid->setPos(0.0f, 24.0f, 0.0f);
        mBase->texOffs(0, 28)->addBox(-16.0f, -16.0f, -16.0f, 16, 8, 16);
        mBase->setPos(0.0f, 24.0f, 0.0f);
        mHead->texOffs(0, 116)->addBox(-3.0f, -4.0f, -3.0f, 6, 6, 6);
        mHead->setPos(0.0f, 12.0f, 0.0f);
        mLid->compile(0.0625f);
        mBase->compile(0.0625f);
        mHead->compile(0.0625f);
    }
    else
    {
        mHasOverriddenParts = true;
        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, NULL);
        mLid = new ModelPartSkin(this, L"lid", modelDefinition, NULL, 0.0f, NULL);
        mBase = new ModelPartSkin(this, L"base", modelDefinition, NULL, 0.0f, NULL);
        mHeadSkin = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, &mSkinnedParts);
        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();
        builder->setVertexAndPixelShader(eMinecraftEntityMaterial_Shulker);
        compileSkinnedDrawlist();
        mSkinnedParts.compile(0.125f);
        builder->useDefaultShaders();
    }
}

ShulkerModel::~ShulkerModel()
{
    delete mBase;
    delete mLid;
    delete mHead;
    delete mHeadSkin;
}

void ShulkerModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                             float headPitch, float scale, boost::shared_ptr<Entity> entity,
                             _SkinAdjustments skinAdjustments)
{
    Model::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, skinAdjustments);
    const float radians = 0.01745329238474369f;
    const float peek = 0.5f + 0.5f * std::sin(ageInTicks * 0.1f);
    mHead->setPos(0.0f, 12.0f + 8.0f * std::sin(ageInTicks * 0.1f), 0.0f);
    mHead->mXRot = headPitch * radians;
    mHead->mYRot = netHeadYaw * radians;
    mLid->mXRot = headPitch * radians;
    mLid->mYRot = netHeadYaw * radians;
    mLid->mY = 24.0f - 16.0f * peek;
    if (mHeadSkin != NULL)
    {
        mHeadSkin->mXRot = mHead->mXRot;
        mHeadSkin->mYRot = mHead->mYRot;
        mHeadSkin->mY = mHead->mY;
    }
}

void ShulkerModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                          float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());
    mBase->render(scale, useDisplayList, false);
    mLid->render(scale, useDisplayList, false);
    if (mHasOverriddenParts)
    {
        mHead->render(scale, useDisplayList, false);
        mSkinnedParts.render();
    }
    else
    {
        mHead->render(scale, useDisplayList, false);
    }
    GlStateManager::color3f(1.0f, 1.0f, 1.0f);
}

void ShulkerModel::renderWhenInvisible(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                                       float ageInTicks, float netHeadYaw, float headPitch, float scale,
                                       bool useDisplayList)
{
    if (mHasOverriddenParts)
    {
        mHeadSkin->render(scale, useDisplayList, false);
        mSkinnedParts.render();
    }
    else
    {
        render(entity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, useDisplayList);
    }
    GlStateManager::color3f(1.0f, 1.0f, 1.0f);
}

std::wstring ShulkerModel::getName()
{
    std::wstring result;
    result.assign(L"shulker");
    return result;
}
