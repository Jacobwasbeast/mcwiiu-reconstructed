#ifndef GAME_CORE_INPUT_HPP
#define GAME_CORE_INPUT_HPP

class LocalPlayer;

class Input
{
public:
    Input();
    virtual ~Input();

    virtual void tick(LocalPlayer* player);
    virtual void getMoveVector();

    float mMoveStrafe;
    float mMoveForward;
    bool mWasJumping;
    bool mJumping;
    bool mSneaking;
    bool mUp;
    bool mDown;
    bool mLeft;
    bool mRight;
    bool mSwimming;
    bool mMovementReady;
    bool mLookReady;
};

#endif
