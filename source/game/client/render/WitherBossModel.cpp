#include "game/client/render/WitherBossModel.hpp"

#include <cmath>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/entity/mob/Entity.hpp"

WitherBossModel::WitherBossModel(float scale, const ModelDefinition& modelDefinition)
    : mUpperBodyParts(NULL)
    , mNumUpperBodyParts(0)
    , mHeads(NULL)
    , mNumHeads(0)
{
    mNumUpperBodyParts = 3;
    mUpperBodyParts = new ModelPartSkin*[3]();
    mNumHeads = 3;
    mHeads = new ModelPartSkin*[3]();

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    if (modelDefinition.DoesOverrideModel())
    {
        mUpperBodyParts[0] = new ModelPartSkin(this, L"upperBodyPart1", modelDefinition, NULL, scale, NULL);
        mUpperBodyParts[1] = new ModelPartSkin(this, L"upperBodyPart2", modelDefinition, NULL, scale, NULL);
        mUpperBodyParts[2] = new ModelPartSkin(this, L"upperBodyPart3", modelDefinition, NULL, scale, NULL);
        mHeads[0] = new ModelPartSkin(this, L"head1", modelDefinition, NULL, scale, NULL);
        mHeads[1] = new ModelPartSkin(this, L"head2", modelDefinition, NULL, scale, NULL);
        mHeads[2] = new ModelPartSkin(this, L"head3", modelDefinition, NULL, scale, NULL);

        builder->setVertexAndPixelShader(eMinecraftEntityMaterial_WitherBoss);
        compileSkinnedDrawlist();
        builder->useDefaultShaders();
    }
    else
    {
        mTexWidth = 64;
        mTexHeight = 64;

        mUpperBodyParts[0] = new ModelPartSkin(this, 0, 16, NULL);
        mUpperBodyParts[0]->addBox(-10.0f, 3.9f, -0.5f, 20, 3, 3, scale);

        mUpperBodyParts[1] = new ModelPartSkin(this);
        mUpperBodyParts[1]->setTexSize(64, 64);
        mUpperBodyParts[1]->setPos(-2.0f, 6.9f, -0.5f);
        mUpperBodyParts[1]->texOffs(0, 22)->addBox(0.0f, 0.0f, 0.0f, 3, 10, 3, scale);
        mUpperBodyParts[1]->texOffs(24, 22)->addBox(-4.0f, 1.5f, 0.5f, 11, 2, 2, scale);
        mUpperBodyParts[1]->texOffs(24, 22)->addBox(-4.0f, 4.0f, 0.5f, 11, 2, 2, scale);
        mUpperBodyParts[1]->texOffs(24, 22)->addBox(-4.0f, 6.5f, 0.5f, 11, 2, 2, scale);

        mUpperBodyParts[2] = new ModelPartSkin(this, 12, 22, NULL);
        mUpperBodyParts[2]->addBox(0.0f, 0.0f, 0.0f, 3, 6, 3, scale);

        mHeads[0] = new ModelPartSkin(this, 0, 0, NULL);
        mHeads[0]->addBox(-4.0f, -4.0f, -4.0f, 8, 8, 8, scale);

        mHeads[1] = new ModelPartSkin(this, 32, 0, NULL);
        mHeads[1]->addBox(-4.0f, -4.0f, -4.0f, 6, 6, 6, scale);
        mHeads[1]->mX = -8.0f;
        mHeads[1]->mY = 4.0f;

        mHeads[2] = new ModelPartSkin(this, 32, 0, NULL);
        mHeads[2]->addBox(-4.0f, -4.0f, -4.0f, 6, 6, 6, scale);
        mHeads[2]->mX = 10.0f;
        mHeads[2]->mY = 4.0f;

        compileSkinnedDrawlist();
        builder->useDefaultShaders();
    }
}

void WitherBossModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                             float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    for (unsigned int i = 0; i < mNumHeads; ++i)
    {
        mHeads[i]->render(scale, useDisplayList, false);
    }

    for (unsigned int i = 0; i < mNumUpperBodyParts; ++i)
    {
        mUpperBodyParts[i]->render(scale, useDisplayList, false);
    }

    callSkinnedDrawlist();
}

void WitherBossModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                                float headPitch, float scale, boost::shared_ptr<Entity> entity,
                                _SkinAdjustments skinAdjustments)
{
    float angle = ageInTicks * 0.1f;
    float headRotation = (cosf(angle) * 0.05f + 0.065f) * 3.1415927f +
                         headPitch * 0.01745329238474369f;

    mUpperBodyParts[1]->mXRot = headRotation;
    mUpperBodyParts[2]->setPos(-2.0f, cosf(headRotation) * 10.0f + 6.9f, sinf(headRotation) * 10.0f - 0.5f);
    mUpperBodyParts[2]->mXRot = (cosf(angle) * 0.1f + 0.265f) * 3.1415927f;
    mHeads[0]->mXRot = headPitch * 0.01745329238474369f;
    mHeads[0]->mYRot = netHeadYaw * 0.01745329238474369f;
}

void WitherBossModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                                      float limbSwingAmount, float partialTicks)
{
    boost::shared_ptr<WitherBoss> wither = boost::dynamic_pointer_cast<WitherBoss>(entity);
    mHeads[1]->mYRot = (wither->getHeadYRot(0) - entity->mYBodyRot) * 0.01745329238474369f;
    mHeads[1]->mXRot = wither->getHeadXRot(0) * 0.01745329238474369f;
    mHeads[2]->mYRot = (wither->getHeadYRot(1) - entity->mYBodyRot) * 0.01745329238474369f;
    mHeads[2]->mXRot = wither->getHeadXRot(1) * 0.01745329238474369f;
}

