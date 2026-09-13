#include "game/ui/UIControl_PlayerSkinPreview.hpp"

#include <cmath>

#include "game/client/render/EntityRenderer.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/client/render/PlayerModel.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
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
    mPreviousRotation = 0.0f;
    mCurrentRotation = 0.0f;
    mTargetRotation = 0.0f;
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
        {
            mPitch = 22;
        }
        if (mRotateBack)
        {
            mPitch -= 2;
            if (mPitch < -22)
            {
                mPitch = -22;
            }
        }
    }
    else if (mRotateBack)
    {
        mPitch -= 2;
        if (mPitch < -22)
        {
            mPitch = -22;
        }
    }
    bool turningLeft;
    if (mRotateForward)
    {
        mRotation += 4;
        if (mRotation > 179)
        {
            mRotation = -180;
            return;
        }
    }
    else if (mRotateBackward)
    {
        mRotation -= 4;
        if (mRotation < -179)
        {
            mRotation = 180;
            return;
        }
    }
    if (!mCapeVisible)
    {
        return;
    }
    ++mCapeSwingTicks;
    if ((mCapeSwingTicks & 3) != 0)
    {
        return;
    }
    if (mRotation < 45)
    {
        if (mRotation < -44)
        {
            mFacingLeft = true;
            turningLeft = true;
        }
        else if (mFacingLeft)
        {
            turningLeft = true;
        }
        else
        {
            turningLeft = false;
        }
    }
    else
    {
        mFacingLeft = false;
        turningLeft = false;
    }
    if (turningLeft)
    {
        mRotation += 4;
        if (mRotation > 179)
        {
            mRotation = -180;
        }
    }
    else
    {
        mRotation -= 4;
        if (mRotation < -179)
        {
            mRotation = 180;
        }
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
    if (renderer == NULL)
    {
        return;
    }
    GlStateManager::pushMatrix();
    GlStateManager::disableCull();
    GlStateManager::translatef((float)x, (float)y, (float)z);
    GlStateManager::rotatef(180.0f - (float)mRotation, 0.0f, 1.0f, 0.0f);
    GlStateManager::enableRescaleNormal();
    GlStateManager::scalef(-1.0f, 0.9375f, 0.9375f);
    bindTexture(mTexturePath, mTextureName);
    PlayerModel* model = static_cast<PlayerModel*>(renderer->getModel());
    if (model != NULL)
    {
        model->setAllVisible(true);
        model->SetSkinAdjustments(mSkinAdjustments);
        model->renderCloak(0.0625f);
    }
    GlStateManager::disableRescaleNormal();
    GlStateManager::enableCull();
    GlStateManager::popMatrix();
}

void UIControl_PlayerSkinPreview::render(fuiRect* rect)
{
    if (rect == NULL)
    {
        return;
    }
    GlStateManager::enableRescaleNormal();
    GlStateManager::enableColorMaterial();
    GlStateManager::pushMatrix();
    GlStateManager::translatef((rect->xMax - rect->xMin) * 0.5f, rect->yMax - rect->yMin, 50.0f);
    Lighting::turnOn();
    Minecraft* minecraft = Minecraft::GetInstance();
    EntityRenderDispatcher* dispatcher = minecraft->getEntityRenderDispatcher();
    PlayerRenderer* renderer = NULL;
    if (dispatcher != NULL)
    {
        renderer = dispatcher->getPlayerRenderer(
            static_cast<EPlayerModelType>(Player::GetModelTypeFromTextureId(mTextureName)));
    }
    render(renderer, 0.0, 0.0, 0.0, 0.0f, 0.0f);
    Lighting::turnOff();
    GlStateManager::popMatrix();
    GlStateManager::disableRescaleNormal();
}

void UIControl_PlayerSkinPreview::SetTexture(const std::wstring& texture, _TEXTURE_NAME textureName)
{
    mTexturePath = texture;
    mTextureName = (int)textureName;
    unsigned int skinId = CMinecraftApp::getSkinIdFromPath(mTexturePath);
    mSkinAdjustments = Player::getSkinAdjustmentsById(skinId);
    if (GameSettings::GetGameSettings((eGameSetting)0x19) == 0 &&
        (mSkinAdjustments.mGameFlags & 0x1f1f810) == 0)
    {
        mSkinAdjustments.mAnimFlags = 0;
    }
    mAdditionalModelParts = NULL;
    if (g_pApp != NULL)
    {
        mAdditionalModelParts = (std::vector<PlayerModel*>*)g_pApp->GetAdditionalModelParts(skinId);
    }
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
