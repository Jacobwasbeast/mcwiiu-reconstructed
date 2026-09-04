#include "game/ui/FJ_DLCOffersMenu.hpp"

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
    FJ_Base* getObjListParent();
    void setButtonPressed(bool pressed);
};

FJ_DLCOffersMenu::FJ_DLCOffersMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
}

FJ_Document* FJ_DLCOffersMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_DLCOffersMenu(renderNode);
}

unsigned int FJ_DLCOffersMenu::getType()
{
    return 60;
}

void FJ_DLCOffersMenu::handlePress(FJ_Event* event)
{
    FJ_Button* button = event->m_pTarget->asFJ_Button();
    if (button != NULL)
    {
        FJ_Base* parent = button->getObjListParent();
        m_pCallbackScene->handlePress(parent->getId(), button->getId());
        button->setButtonPressed(false);
    }
}

FJ_DLCOffersMenu::~FJ_DLCOffersMenu()
{
}
