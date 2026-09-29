#include "game/ui/UIControl_PlayerSkinPreview.hpp"

#include <cmath>

#include "game/client/render/EntityRenderer.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/PlayerModel.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/Options.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/ScreenSizeCalculator.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/world/generation/Player.hpp"

UIControl_PlayerSkinPreview::UIControl_PlayerSkinPreview()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    ScreenSizeCalculator calculator(minecraft->mOptions, minecraft->mWidth, minecraft->mHeight, -1);
    mUnknown50 = 0;
    mUnknown54 = 1.0f;
    mUnknown58 = 1.0f;
    mTexturePath = L"default";
    mTextureName = 0;
    mCapePath = L"default";
    mWidth = 0.0f;
    mHeight = 0.0f;
    mWidthScale = 0.0f;
    mHeightScale = 0.0f;
    mRotation = 0;
    mPitch = 0;
    mInitialRotation = 0;
    mFacingRotation = 0;
    mUnknown110 = 0.0f;
    mLimbSwingAmountOld = 0.0f;
    mLimbSwingAmount = 0.0f;
    mLimbSwing = 0.0f;
    mCapeVisible = false;
    mFacingLeft = false;
    mAnimationTicks = 0;
    mAnimating = false;
    mAnimationAngle = 0.0f;
    mAnimation = 0;
    mAdditionalModelParts = NULL;
    mRotating = false;
    mRotateBack = false;
    mRotateForward = false;
    mRotateBackward = false;
    mWidth = (float)minecraft->mWidth;
    mHeight = (float)minecraft->mHeight;
    mWidthScale = (float)calculator.getWidth();
    mHeightScale = (float)calculator.getHeight();
}

UIControl_PlayerSkinPreview::~UIControl_PlayerSkinPreview()
{
}

void UIControl_PlayerSkinPreview::tick()
{
    if (mAnimating)
    {
        ++mAnimationTicks;
        mRotation = mInitialRotation + mAnimationTicks * ((mFacingRotation - mInitialRotation) / 15);
        return;
    }

    if (mRotating)
    {
        mPitch += 2;
        if (mPitch > 22)
            mPitch = 22;
    }
    if (mRotateBack)
    {
        mPitch -= 2;
        if (mPitch < -22)
            mPitch = -22;
    }
    if (mRotateForward)
    {
        mRotation += 4;
        if (mRotation > 179)
            mRotation = -180;
    }
    if (mRotateBackward)
    {
        mRotation -= 4;
        if (mRotation < -179)
            mRotation = 180;
    }

    if (!mCapeVisible)
        return;

    ++mCapeSwingTicks;
    if ((mCapeSwingTicks & 3) != 0)
        return;

    bool turningLeft;
    if (mRotation >= 45)
    {
        mFacingLeft = false;
        turningLeft = false;
    }
    else if (mRotation < -44)
    {
        mFacingLeft = true;
        turningLeft = true;
    }
    else
    {
        turningLeft = mFacingLeft;
    }

    if (turningLeft)
    {
        mRotation += 4;
        if (mRotation > 179)
            mRotation = -180;
    }
    else
    {
        mRotation -= 4;
        if (mRotation < -179)
            mRotation = 180;
    }
}

unsigned int UIControl_PlayerSkinPreview::bindTexture(const std::wstring& name, int texture)
{
    Textures* textures = Minecraft::GetInstance()->getTextureManager();
    int loaded = textures->loadMemTexture(name, texture);
    if (loaded >= 0)
    {
        textures->bind(loaded);
        return 1;
    }
    return 0;
}

unsigned int UIControl_PlayerSkinPreview::bindTexture(const std::wstring& name, const std::wstring& path)
{
    Textures* textures = Minecraft::GetInstance()->getTextureManager();
    int loaded = textures->loadMemTexture(name, path);
    if (loaded >= 0)
    {
        textures->bind(loaded);
        return 1;
    }
    return 0;
}

void UIControl_PlayerSkinPreview::render(EntityRenderer* renderer, double x, double y, double z, float yaw, float partialTick)
{
    GlStateManager::pushMatrix();
    GlStateManager::disableCull();

    PlayerModel* model = static_cast<PlayerModel*>(renderer->getModel());
    model->mCrouching = false;
    model->mAttackTime = 0.0f;
    model->mSwimming = false;
    model->setArmPose(HumanoidArm::RIGHT, HumanoidModel::eArmPose_Empty);
    model->setArmPose(HumanoidArm::LEFT, HumanoidModel::eArmPose_Empty);
    model->mUseItemProgress = 0.0f;
    model->mUseItemRemainingTicks = 0.0f;
    model->mSitting = false;
    model->mEating = false;
    model->mYoung = false;
    model->mRiding = false;
    model->mSwimAmount = 0.0f;
    model->mSpearThrowProgress = 0.0f;
    model->mHead->mXRot = 0.0f;
    model->mSkinAdjustments = mSkinAdjustments;

    if (!mAnimating && mAnimation == 1)
    {
        model->mCrouching = true;
        model->mHead->mYRot = 0.1f;
    }
    else if (!mAnimating && mAnimation == 2)
    {
        model->mHead->mYRot = 0.0f;
        model->setArmPose(HumanoidArm::RIGHT, HumanoidModel::eArmPose_Item);
        float swingTicks = mAnimationAngle + 1.0f;
        if (swingTicks >= 18.0f)
            swingTicks = 0.0f;
        mAnimationAngle = swingTicks;
        model->mAttackTime = swingTicks / 18.0f;
    }
    else
    {
        model->mHead->mYRot = 0.0f;
    }

    float bodyRotation = (float)mRotation;
    float headRotation = bodyRotation;
    GlStateManager::translatef((float)x, (float)y, (float)z);
    GlStateManager::rotatef(180.0f - bodyRotation, 0.0f, 1.0f, 0.0f);
    GlStateManager::enableRescaleNormal();
    GlStateManager::scalef(-1.0f, -1.0f, 1.0f);
    GlStateManager::scalef(0.9375f, 0.9375f, 0.9375f);
    GlStateManager::translatef(0.0f, -1.5078125f, 0.0f);

    float limbSwingAmountOld = mLimbSwingAmount;
    mLimbSwingAmountOld = mLimbSwingAmount;
    mLimbSwingAmount += (0.1f - mLimbSwingAmount) * 0.4f;
    mLimbSwing += mLimbSwingAmount;
    float limbSwingAmount = limbSwingAmountOld + (mLimbSwingAmount - limbSwingAmountOld) * partialTick;
    if (limbSwingAmount > 1.0f)
        limbSwingAmount = 1.0f;

    MemSect(0x1f);
    bindTexture(mTexturePath, mTextureName);
    MemSect(0);
    GlStateManager::enableAlphaTest();
    model->setAllVisible(true);
    boost::shared_ptr<Entity> noEntity;
    model->render(noEntity, mLimbSwing - mLimbSwingAmount * (1.0f - partialTick), limbSwingAmount, 0.0f,
                  headRotation - bodyRotation, 0.0f, 0.0625f, true);

    if (bindTexture(mCapePath, std::wstring(L"")) != 0)
    {
        GlStateManager::pushMatrix();
        GlStateManager::translatef(0.0f, 0.0f, 0.125f);
        double motionX = 0.0;
        double motionZ = 0.0;
        float sinYaw = sinf(0.017453292f);
        float cosYaw = cosf(0.017453292f);
        float capeSwing = (float)(motionX * sinYaw + motionZ * -cosYaw) * 100.0f;
        if (capeSwing < 0.0f)
            capeSwing = 0.0f;
        float capeSway = (float)(motionX * -cosYaw - motionZ * sinYaw) * 100.0f;
        float capeLift = model->mCrouching ? 26.0f : 1.0f;
        GlStateManager::rotatef(6.0f + capeSwing * 0.5f + capeLift, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(capeSway * 0.5f, 0.0f, 0.0f, 1.0f);
        GlStateManager::rotatef(-(capeSway * 0.5f), 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(180.0f, 0.0f, 1.0f, 0.0f);
        model->renderCloak(0.0625f);
        GlStateManager::popMatrix();
    }

    GlStateManager::disableRescaleNormal();
    GlStateManager::enableCull();
    GlStateManager::popMatrix();
    MemSect(0x1f);
    MemSect(0);

    model->mCrouching = false;
    model->mAttackTime = 0.0f;
    model->setArmPose(HumanoidArm::RIGHT, HumanoidModel::eArmPose_Empty);
    model->setArmPose(HumanoidArm::LEFT, HumanoidModel::eArmPose_Empty);
    model->clearAllOffsets();
}

void UIControl_PlayerSkinPreview::render(fuiRect* rect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    GlStateManager::enableRescaleNormal();
    GlStateManager::enableColorMaterial();
    GlStateManager::pushMatrix();

    float rectWidth = rect->xMax - rect->xMin;
    bool upsideDown = (mSkinAdjustments.mAnimFlags & 0x80000000) != 0;
    if (!upsideDown)
        GlStateManager::translatef(rectWidth * 0.5f, (rect->yMax - rect->yMin) - 3.5f, 50.0f);
    else
        GlStateManager::translatef(rectWidth * 0.5f, (rect->yMax - rect->yMin) + -253.5f, 50.0f);
    float scale = rectWidth / (mWidth / mHeight);
    GlStateManager::scalef(-scale, scale, scale);
    if (!upsideDown)
        GlStateManager::rotatef(180.0f, 0.0f, 0.0f, 1.0f);

    Lighting::turnOn();
    GlStateManager::rotatef(-(float)mPitch, 1.0f, 0.0f, 0.0f);

    bool hideGui = minecraft->mOptions->mHideGui;
    minecraft->mOptions->mHideGui = true;

    EPlayerModelType modelType;
    if (mTexturePath.empty())
    {
        modelType = static_cast<EPlayerModelType>(Player::GetModelTypeFromTextureId(mTextureName));
    }
    else
    {
        unsigned int skinId = CMinecraftApp::getSkinIdFromPath(mTexturePath);
        _SkinAdjustments adjustments = gApp.GetSkinAdjustments(skinId);
        modelType = static_cast<EPlayerModelType>(Player::GetModelTypeFromAnimBitmask(adjustments.mAnimFlags));
    }
    PlayerRenderer* renderer = Minecraft::GetInstance()->getEntityRenderDispatcher()->getPlayerRenderer(modelType);

    if (renderer != NULL)
    {
        if (mAdditionalModelParts != NULL && !mAdditionalModelParts->empty())
        {
            for (std::vector<ModelPart*>::iterator it = mAdditionalModelParts->begin();
                 it != mAdditionalModelParts->end(); ++it)
            {
                (*it)->mShow = true;
            }
        }
        render(renderer, 0.0, 0.0, 0.0, 0.0f, 1.0f);
        if (mAdditionalModelParts != NULL && !mAdditionalModelParts->empty())
        {
            for (std::vector<ModelPart*>::iterator it = mAdditionalModelParts->begin();
                 it != mAdditionalModelParts->end(); ++it)
            {
                (*it)->mShow = false;
            }
        }
    }

    minecraft->mOptions->mHideGui = hideGui;
    GlStateManager::popMatrix();
    Lighting::turnOff();
    GlStateManager::disableRescaleNormal();
}

void UIControl_PlayerSkinPreview::SetTexture(const std::wstring& texture, _TEXTURE_NAME textureName)
{
    mTexturePath = texture;
    mTextureName = (int)textureName;
    mSkinAdjustments = Player::getSkinAdjustmentsById(CMinecraftApp::getSkinIdFromPath(mTexturePath));
    if (GameSettings::GetGameSettings(eGameSetting_CustomSkinAnim) == 0 &&
        (mSkinAdjustments.mAnimFlags & 0x1f1f810) == 0)
    {
        mSkinAdjustments.mAnimFlags = 0;
    }
    mAdditionalModelParts = gApp.GetAdditionalModelParts(CMinecraftApp::getSkinIdFromPath(mTexturePath));
}

void UIControl_PlayerSkinPreview::SetFacing(ESkinPreviewFacing facing, bool animate)
{
    if (facing == eFacingFront)
    {
        mFacingRotation = 0;
        mFacingLeft = true;
    }
    else if (facing == eFacingRight)
    {
        mFacingRotation = 45;
        mFacingLeft = false;
    }
    else if (facing == eFacingLeft)
    {
        mFacingRotation = -45;
        mFacingLeft = true;
    }
    if (!animate)
    {
        mAnimating = false;
        mRotation = mFacingRotation;
        return;
    }
    mAnimating = true;
    mAnimationTicks = 0;
    mInitialRotation = mRotation;
}

void UIControl_PlayerSkinPreview::CycleNextAnimation()
{
    ++mAnimation;
    if (mAnimation > 2) mAnimation = 0;
    mAnimationAngle = 0.0f;
}

void UIControl_PlayerSkinPreview::CyclePreviousAnimation()
{
    --mAnimation;
    if (mAnimation < 0) mAnimation = 2;
    mAnimationAngle = 0.0f;
}
