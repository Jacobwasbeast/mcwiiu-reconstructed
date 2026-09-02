#ifndef GAME_UI_METHODACTION_HPP
#define GAME_UI_METHODACTION_HPP

#include "game/core/TickAction.hpp"

template <class T>
class MethodAction : public TickAction
{
public:
    typedef void (T::*Func)();

    MethodAction(T* pTarget, Func pFunc);
    virtual ~MethodAction();

    virtual void Execute();

private:
    T* m_pTarget;
    Func m_pFunc;
};

#endif
