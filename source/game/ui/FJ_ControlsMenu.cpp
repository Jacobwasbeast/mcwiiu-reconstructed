#include "game/ui/FJ_ControlsMenu.hpp"

#include <string>

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_HtmlTextPanel.hpp"
#include "game/ui/FJ_HtmlTextPanelLarge.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

class FJ_List
{
public:
    int GetActiveItem();
    FJ_FuiNode* ReturnItem(int index);
};

FJ_ControlsMenu::FJ_ControlsMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
    addEventListener(eFJEventType_CheckboxToggled, this, _handleCheckboxToggled, false);
}

FJ_ControlsMenu::~FJ_ControlsMenu()
{
}

void FJ_ControlsMenu::handleCheckboxToggled(FJ_Event* event)
{
    FJ_CheckBox* checkBox = event->m_pTarget->asFJ_CheckBox();
    if (checkBox != NULL)
        m_pCallbackScene->handleCheckboxToggled(checkBox->getId(), checkBox->getChecked());
}

void FJ_ControlsMenu::_handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_ControlsMenu*>(node)->handleCheckboxToggled(event);
}

FJ_Document* FJ_ControlsMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_ControlsMenu(renderNode);
}

unsigned int FJ_ControlsMenu::getType()
{
    return 37;
}

void FJ_ControlsMenu::init(FJ_Event* event)
{
    FJ_Document::init(event);

    FJ_FuiNode* controlsList = getChildByName(std::string("ControlsMultiList"));
    FJ_FuiNode* helpPanel = getChildByName(std::string("ControlsHelpPanel"));
    if (helpPanel->isFJ_HtmlTextPanelLarge())
    {
        helpPanel->asFJ_HtmlTextPanelLarge()->SetFixedWidth(Math::round(controlsList->getWidth() * 0.5f));
    }
    else if (helpPanel->isFJ_HtmlTextPanel())
    {
        helpPanel->asFJ_HtmlTextPanel()->SetFixedWidth(Math::round(controlsList->getWidth() * 0.5f));
    }
}

void FJ_ControlsMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    FJ_FuiNode* version = getChildByName(std::string("Version"));
    if (version != NULL)
    {
        version->setX(getStage()->getStageWidth() - version->getWidth() - right);
        version->setY(getStage()->getStageHeight() - version->getHeight() - top - version->getHeight());
    }
}

void FJ_ControlsMenu::PositionControlsHelp()
{
    FJ_FuiNode* controlsListNode = getChildByName(std::string("ControlsMultiList"));
    FJ_List* controlsList = controlsListNode->asFJ_List();
    FJ_FuiNode* activeItem = controlsList->ReturnItem(controlsList->GetActiveItem());
    if (activeItem == NULL)
        return;

    FJ_FuiNode* helpPanel = getChildByName(std::string("ControlsHelpPanel"));
    FJ_FuiNode* backgroundPanel = getChildByName(std::string("BackgroundPanel"));

    helpPanel->setX(Math::round(backgroundPanel->getX() + backgroundPanel->getWidth() - 5.0f));
    helpPanel->setY(Math::round(activeItem->getY() + activeItem->getHeight() * 0.5f - helpPanel->getHeight() * 0.5f));

    if (helpPanel->getY() + helpPanel->getHeight() <= backgroundPanel->getY() + backgroundPanel->getHeight())
    {
        if (helpPanel->getY() < backgroundPanel->getY())
            helpPanel->setY(Math::round(backgroundPanel->getY() + 5.0f));
    }
    else
    {
        const int overflow = Math::round(helpPanel->getY() + helpPanel->getHeight() -
                                         backgroundPanel->getY() - backgroundPanel->getHeight());
        helpPanel->setY(Math::round(helpPanel->getY() - overflow - 5.0f));
    }
}

void FJ_ControlsMenu::SetControlsHelp(int id, const std::wstring& help)
{
    FJ_FuiNode* helpPanel = getChildByName(std::string("ControlsHelpPanel"));
    if (helpPanel->isFJ_HtmlTextPanelLarge())
        helpPanel->asFJ_HtmlTextPanelLarge()->SetLabel(help);
    else if (helpPanel->isFJ_HtmlTextPanel())
        helpPanel->asFJ_HtmlTextPanel()->SetLabel(help);

    PositionControlsHelp();
}
