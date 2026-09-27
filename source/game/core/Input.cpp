#include "game/core/Input.hpp"

#include "platform/wiiu/CInput.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/PowerupItems.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/util/phys__Vec2.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"

Input::Input()
{
    mMoveStrafe = 0.0f;
    mMoveForward = 0.0f;
    mWasJumping = false;
    mJumping = false;
    mSneaking = false;
    mUp = false;
    mDown = false;
    mLeft = false;
    mRight = false;
    mSwimming = false;
    mMovementReady = false;
    mLookReady = false;
}

Input::~Input()
{
}

void Input::tick(LocalPlayer* player)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    int playerIndex = player->mUserIndex;
    MultiPlayerGameMode* gameMode = minecraft->mGameModes[playerIndex];

    mLeft = false;
    mRight = false;

    if (gameMode->isInputAllowed(EControllerActions_MoveLeft) || gameMode->isInputAllowed(EControllerActions_MoveRight))
    {
        mMoveStrafe = static_cast<float>(-g_pInput->GetJoypadStick_LX(playerIndex, true));
    }
    else
    {
        mMoveStrafe = 0.0f;
    }

    if (mMoveStrafe > 0.5f)
    {
        mLeft = true;
    }
    else if (mMoveStrafe < -0.5f)
    {
        mRight = true;
    }

    mUp = false;
    mDown = false;

    if (gameMode->isInputAllowed(EControllerActions_MoveForward) || gameMode->isInputAllowed(EControllerActions_MoveBackward))
    {
        mMoveForward = static_cast<float>(g_pInput->GetJoypadStick_LY(playerIndex, true));
    }
    else
    {
        mMoveForward = 0.0f;
    }

    if (mMoveForward > 0.0f)
    {
        mUp = true;
    }
    else if (mMoveForward < 0.0f)
    {
        mDown = true;
    }

    if (!mMovementReady)
    {
        if (mMoveStrafe * mMoveStrafe + mMoveForward * mMoveForward == 0.0f)
        {
            mMovementReady = true;
        }
        mMoveStrafe = 0.0f;
        mMoveForward = 0.0f;
    }

    bool sneakButtonDown = (player->mButtonsDown & 0x80) != 0;
    bool inWater = player->isInWater();
    bool onGround = player->mOnGround;
    bool flying = player->mAbilities.mFlying;
    mSwimming = inWater && !onGround && !flying && sneakButtonDown;
    bool canToggleSneak = ((!inWater || onGround) && !flying) || player->isPassenger();

    if (canToggleSneak && sneakButtonDown && gameMode->isInputAllowed(EControllerActions_Sneak))
    {
        mSneaking = !mSneaking;
    }

    if (mSneaking)
    {
        mMoveStrafe *= 0.3f;
        mMoveForward *= 0.3f;
    }

    float turnX = 0.0f;
    float turnY = 0.0f;

    if (gameMode->isInputAllowed(EControllerActions_LookUp) || gameMode->isInputAllowed(EControllerActions_LookDown))
    {
        turnX = static_cast<float>(g_pInput->GetJoypadStick_RX(playerIndex, true));
        turnX *= static_cast<float>(GameSettings::GetGameSettings(playerIndex, eGameSetting_GameSensitivity)) / 100.0f;
    }

    if (gameMode->isInputAllowed(EControllerActions_LookLeft) || gameMode->isInputAllowed(EControllerActions_LookRight))
    {
        turnY = static_cast<float>(g_pInput->GetJoypadStick_RY(playerIndex, true));
        turnY *= static_cast<float>(GameSettings::GetGameSettings(playerIndex, eGameSetting_GameSensitivity)) / 100.0f;
    }

    if (GameSettings::GetGameSettings(playerIndex, eGameSetting_InvertLook) != 0)
    {
        turnY = -turnY;
    }

    if (gApp.GetFirstMiniGameType() == eMiniGameId_Glide &&
        player->CheckPowerup(PowerupItems::eGlide_Timed_Powerup_ID_InvertControls))
    {
        turnX = -turnX;
        turnY = -turnY;
    }

    if (!mLookReady)
    {
        if (turnX * turnX + turnY * turnY == 0.0f)
        {
            mLookReady = true;
        }
        turnX = 0.0f;
        turnY = 0.0f;
    }

    player->interpolateTurn(turnX * (turnX < 0.0f ? -turnX : turnX) * 50.0f,
                            turnY * (turnY < 0.0f ? -turnY : turnY) * 50.0f);

    mJumping = g_pInput->GetValue(playerIndex, EControllerActions_Jump, false) != 0 &&
               gameMode->isInputAllowed(EControllerActions_Jump);
}

void Input::getMoveVector()
{
    phys::Vec2::newTemp(mMoveStrafe, mMoveForward);
}
