#include "game/ui/FJ_CreateWorldMenu.hpp"

#include <string>

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_CreateWorldMenu::FJ_CreateWorldMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
    addEventListener(eFJEventType_CheckboxToggled, this, _handleCheckboxToggled, false);
}

FJ_CreateWorldMenu::~FJ_CreateWorldMenu()
{
}

void FJ_CreateWorldMenu::handleCheckboxToggled(FJ_Event* event)
{
    FJ_CheckBox* checkBox = event->m_pTarget->asFJ_CheckBox();
    if (checkBox != NULL)
    {
        m_pCallbackScene->handleCheckboxToggled(checkBox->getId(), checkBox->getChecked());
    }
}

void FJ_CreateWorldMenu::_handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_CreateWorldMenu*>(node)->handleCheckboxToggled(event);
}

FJ_Document* FJ_CreateWorldMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_CreateWorldMenu(renderNode);
}

unsigned int FJ_CreateWorldMenu::getType()
{
    return 22;
}

void FJ_CreateWorldMenu::init(FJ_Event* event)
{
    std::string name("MainPanel");
    m_root = getChildByName(name);
    FJ_Document::init(event);
}
