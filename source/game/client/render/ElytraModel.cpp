#include "game/client/render/ElytraModel.hpp"

#include <math.h>

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"

ElytraModel::ElytraModel(const ModelDefinition& modelDefinition)
{
    mOverrideAnim = false;

    mLeftWing = new ModelPartSkin(this, 22, 0, NULL);
    mLeftWing->addBox(-10.0f, 0.0f, 0.0f, 10, 20, 2, 1.0f);

    mRightWing = new ModelPartSkin(this, 22, 0, NULL);
    mRightWing->mMirror = true;
    mRightWing->addBox(0.0f, 0.0f, 0.0f, 10, 20, 2, 1.0f);

    mLeftWing->mX = 5.0f;
    mLeftWing->mY = 0.0f;
    mLeftWing->mXRot = 15.0f * 0.01745329238474369f;
    mLeftWing->mYRot = 0.0f;
    mLeftWing->mZRot = -15.0f * 0.01745329238474369f;

    mRightWing->mX = -mLeftWing->mX;
    mRightWing->mY = mLeftWing->mY;
    mRightWing->mXRot = mLeftWing->mXRot;
    mRightWing->mYRot = -mLeftWing->mYRot;
    mRightWing->mZRot = -mLeftWing->mZRot;

    compileSkinnedDrawlist();
}

ElytraModel::~ElytraModel()
{
}

void ElytraModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    GlStateManager::disableRescaleNormal();
    GlStateManager::disableCull();

    bool isBaby = false;

    if (Entity::instanceof(entity, ENTITY_TYPE_LIVING_ENTITY))
    {
        boost::shared_ptr<LivingEntity> livingEntity = boost::static_pointer_cast<LivingEntity>(entity);

        if (livingEntity->isBaby())
        {
            isBaby = true;
        }
    }

    if (isBaby)
    {
        GlStateManager::pushMatrix();
        GlStateManager::scalef(0.5f, 0.5f, 0.5f);
        GlStateManager::translatef(0.0f, 1.5f, -0.10000000149011612f);

        mLeftWing->render(scale, useDisplayList, false);
        mRightWing->render(scale, useDisplayList, false);

        GlStateManager::popMatrix();

        callSkinnedDrawlist();
    }
    else
    {
        mLeftWing->render(scale, useDisplayList, false);
        mRightWing->render(scale, useDisplayList, false);

        callSkinnedDrawlist();
    }
}

void ElytraModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    Model::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, skinAdjustments);

    float xRot = 15.0f * 0.01745329238474369f;
    float zRot = -15.0f * 0.01745329238474369f;
    float yRot = 0.0f;
    float y = 0.0f;

    bool isFallFlying = false;

    if (!mOverrideAnim && Entity::instanceof(entity, ENTITY_TYPE_LIVING_ENTITY))
    {
        boost::shared_ptr<LivingEntity> livingEntity = boost::dynamic_pointer_cast<LivingEntity>(entity);

        if (livingEntity->isFallFlying())
        {
            isFallFlying = true;
        }
    }

    if (isFallFlying)
    {
        float flyAmount = 1.0f;

        if (entity->mYd < 0.0)
        {
            Vec3* movement = Vec3::newTemp(entity->mXd, entity->mYd, entity->mZd)->normalize();

            flyAmount = 1.0f - (float)pow(-movement->y, 1.5);
        }

        xRot = flyAmount * (20.0f * 0.01745329238474369f) + (1.0f - flyAmount) * xRot;
        zRot = flyAmount * (-90.0f * 0.01745329238474369f) + (1.0f - flyAmount) * zRot;
    }
    else if (entity && entity->isSneaking())
    {
        xRot = 40.0f * 0.01745329238474369f;
        zRot = -45.0f * 0.01745329238474369f;
        yRot = 5.0f * 0.01745329238474369f;
        y = 3.0f;
    }

    mLeftWing->mX = 5.0f;
    mLeftWing->mY = y;

    if (!mOverrideAnim && boost::dynamic_pointer_cast<AbstractClientPlayer>(entity))
    {
        boost::shared_ptr<AbstractClientPlayer> player = boost::dynamic_pointer_cast<AbstractClientPlayer>(entity);

        player->mElytraRotX += (xRot - player->mElytraRotX) * 0.10000000149011612f;
        player->mElytraRotY += (yRot - player->mElytraRotY) * 0.10000000149011612f;
        player->mElytraRotZ += (zRot - player->mElytraRotZ) * 0.10000000149011612f;

        mLeftWing->mXRot = player->mElytraRotX;
        mLeftWing->mYRot = player->mElytraRotY;
        mLeftWing->mZRot = player->mElytraRotZ;
    }
    else
    {
        mLeftWing->mXRot = xRot;
        mLeftWing->mYRot = yRot;
        mLeftWing->mZRot = zRot;
    }

    mRightWing->mX = -mLeftWing->mX;
    mRightWing->mY = mLeftWing->mY;
    mRightWing->mXRot = mLeftWing->mXRot;
    mRightWing->mYRot = -mLeftWing->mYRot;
    mRightWing->mZRot = -mLeftWing->mZRot;
}

void ElytraModel::overrideAnim(float flyAmount, float netHeadYaw, float headPitch, float scale)
{
    _SkinAdjustments skinAdjustments;

    Model::setupAnim(0.0f, 0.0f, 0.0f, netHeadYaw, headPitch, scale, boost::shared_ptr<Entity>(), skinAdjustments);

    mLeftWing->mX = 5.0f;
    mLeftWing->mY = 0.0f;
    mLeftWing->mXRot = flyAmount * (20.0f * 0.01745329238474369f) + (1.0f - flyAmount) * (15.0f * 0.01745329238474369f);
    mLeftWing->mYRot = 0.0f;
    mLeftWing->mZRot = flyAmount * (-90.0f * 0.01745329238474369f) + (1.0f - flyAmount) * (-15.0f * 0.01745329238474369f);

    mRightWing->mX = -mLeftWing->mX;
    mRightWing->mY = mLeftWing->mY;
    mRightWing->mXRot = mLeftWing->mXRot;
    mRightWing->mYRot = -mLeftWing->mYRot;
    mRightWing->mZRot = -mLeftWing->mZRot;
}
