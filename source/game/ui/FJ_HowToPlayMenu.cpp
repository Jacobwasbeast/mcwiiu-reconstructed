#include "game/ui/FJ_HowToPlayMenu.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_MenuButton.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_HowToPlayMenu::FJ_HowToPlayMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
}

void FJ_HowToPlayMenu::handlePress(FJ_Event* event)
{
    FJ_MenuButton* button = event->m_pTarget->asFJ_MenuButton();
    if (button != NULL)
    {
        FJ_FuiNode* parent = button->getObjListParent();
        m_pCallbackScene->handlePress(parent->asFJ_Base()->getId(), button->getId());
        button->setButtonPressed(false);
    }
}

FJ_Document* FJ_HowToPlayMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_HowToPlayMenu(renderNode);
}

unsigned int FJ_HowToPlayMenu::getType()
{
    return 9;
}
