#ifndef GAME_UI_UICONTROL_PLAYERSKINPREVIEW_HPP
#define GAME_UI_UICONTROL_PLAYERSKINPREVIEW_HPP

#include <string>
#include <vector>

#include "game/ui/UIControl.hpp"
#include "game/client/render/Textures.hpp"
#include "vendor/_SkinAdjustments.hpp"

class EntityRenderer;
class PlayerModel;
class fuiRect;

class UIControl_PlayerSkinPreview : public UIControl
{
public:
    enum ESkinPreviewFacing
    {
        eFacingFront,
        eFacingRight,
        eFacingLeft
    };

    UIControl_PlayerSkinPreview();
    virtual ~UIControl_PlayerSkinPreview();

    virtual void tick();
    unsigned int bindTexture(const std::wstring& name, int texture);
    unsigned int bindTexture(const std::wstring& name, const std::wstring& path);
    virtual void render(EntityRenderer* renderer, double x, double y, double z, float yaw, float partialTick);
    virtual void render(fuiRect* rect);
    void SetTexture(const std::wstring& texture, _TEXTURE_NAME textureName);
    void SetFacing(ESkinPreviewFacing facing, bool animate);
    void CycleNextAnimation();
    void CyclePreviousAnimation();
    void SetSkinAdjustments(_SkinAdjustments adjustments);

private:
    int mUnknown50;
    float mUnknown54;
    float mUnknown58;
    std::wstring mTexturePath;
    int mTextureName;
    std::wstring mCapePath;
    _SkinAdjustments mSkinAdjustments;
    std::vector<PlayerModel*>* mAdditionalModelParts;
    float mWidth;
    float mHeight;
    float mWidthScale;
    float mHeightScale;
    int mRotation;
    int mPitch;
    int mInitialRotation;
    int mFacingRotation;
    float mPreviousRotation;
    float mCurrentRotation;
    float mTargetRotation;
    float mAnimationRotation;
    bool mCapeVisible;
    bool mFacingLeft;
    bool mAnimating;
    int mAnimationTicks;
    bool mAnimatingFacing;
    float mAnimationAngle;
    int mAnimation;
    std::vector<PlayerModel*>* mModelParts;
    bool mRotating;
    bool mRotateBack;
    bool mRotateForward;
    bool mHasModelParts;
};

#endif
