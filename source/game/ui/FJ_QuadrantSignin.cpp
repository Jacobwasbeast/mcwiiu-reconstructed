#include "game/ui/FJ_QuadrantSignin.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_QuadrantSignin::FJ_QuadrantSignin(fuiRenderNode* renderNode) : FJ_Document(renderNode), mABSwap(false)
{
    mControllers[0] = 0;
    mControllers[1] = 0;
    mControllers[2] = 0;
    mControllers[3] = 0;
}
FJ_QuadrantSignin::~FJ_QuadrantSignin() {}
FJ_Document* FJ_QuadrantSignin::Create(fuiRenderNode* renderNode) { return new FJ_QuadrantSignin(renderNode); }
unsigned int FJ_QuadrantSignin::getType() { return 24; }
void FJ_QuadrantSignin::_init(FJ_FuiNode* node, FJ_Event* event)
{
    FJ_Document* document = dynamic_cast<FJ_Document*>(node);
    if (document != 0) document->init(event);
}
void FJ_QuadrantSignin::ShowControllerTimer(int controller, bool visible)
{
    if (controller < 0 || controller >= 4 || mControllers[controller] == 0) return;
    FJ_FuiNode* timer = mControllers[controller]->getChildByName("Timer");
    if (timer != 0) timer->setVisible(visible);
}
void FJ_QuadrantSignin::SetControllerStatus(int controller, int status, int frame)
{
    if (controller < 0 || controller >= 4 || mControllers[controller] == 0) return;
    mControllers[controller]->gotoAndStop(status == 0 ? "StopO" : "StopX");
    FJ_FuiNode* controllers = mControllers[controller]->getChildByName("Controllers");
    if (controllers != 0)
    {
        FJ_FuiNode* button = controllers->getChildAt(0);
        if (button != 0) button->gotoAndStop(frame + 1);
    }
}
void FJ_QuadrantSignin::ShowTimerAnimation(FJ_Event* event)
{
    if (event != 0 && event->m_pTarget != 0) event->m_pTarget->gotoAndStop("StopO");
}
void FJ_QuadrantSignin::_ShowTimerAnimation(FJ_FuiNode* node, FJ_Event* event)
{
    if (node != 0)
    {
        FJ_QuadrantSignin* document = dynamic_cast<FJ_QuadrantSignin*>(node->getParent());
        if (document != 0) document->ShowTimerAnimation(event);
    }
}
void FJ_QuadrantSignin::init(FJ_Event* event)
{
    FJ_Document::init(event);
    mControllers[0] = getChildByName("Controller0");
    mControllers[1] = getChildByName("Controller1");
    mControllers[2] = getChildByName("Controller2");
    mControllers[3] = getChildByName("Controller3");
    for (int i = 0; i < 4; ++i)
    {
        ShowControllerTimer(i, false);
        SetControllerStatus(i, 0, 1);
    }
}
void FJ_QuadrantSignin::SetABSwap(bool swap)
{
    mABSwap = swap;
    for (int i = 0; i < 4; ++i)
    {
        if (mControllers[i] == 0) continue;
        FJ_FuiNode* graphic = mControllers[i]->getChildByName("ButtonGraphic");
        if (graphic != 0) graphic->gotoAndStop(swap ? "StopX" : "StopO");
    }
}
void FJ_QuadrantSignin::JoinButtonPressed(int controller)
{
    SetControllerStatus(controller, 1, 1);
}
