#ifndef GAME_CORE_JUMPCONTROL_HPP
#define GAME_CORE_JUMPCONTROL_HPP

class Mob;

class JumpControl
{
public:
    explicit JumpControl(Mob* mob);

    virtual ~JumpControl() {}
    virtual void tick();

    void jump();

protected:
    Mob* mMob;
    bool mJump;
};

#endif
