#include "game/ui/FJ_LoadOrJoinMenu.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/UIScene.hpp"

class FJ_Base
{
public:
    unsigned int getId();
};

class FJ_Button : public FJ_Base
{
public:
    FJ_FuiNode* getObjListParent();
    void setButtonPressed(bool pressed);
};

FJ_LoadOrJoinMenu::FJ_LoadOrJoinMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
}

FJ_Document* FJ_LoadOrJoinMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_LoadOrJoinMenu(renderNode);
}

unsigned int FJ_LoadOrJoinMenu::getType()
{
    return 65;
}

void FJ_LoadOrJoinMenu::handlePress(FJ_Event* event)
{
    FJ_Button* button = event->m_pTarget->asFJ_Button();
    if (button != NULL)
    {
        FJ_FuiNode* parent = button->getObjListParent();
        m_pCallbackScene->handlePress(parent->asFJ_Base()->getId(), button->getId());
        button->setButtonPressed(false);
    }
}

FJ_LoadOrJoinMenu::~FJ_LoadOrJoinMenu()
{
}
