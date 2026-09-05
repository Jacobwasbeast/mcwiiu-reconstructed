#include "game/ui/FJ_LaunchMoreOptionsMenu.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/fuiRenderNode.hpp"

class UIScene
{
public:
    virtual void handleCheckboxElementToggled(int first, int second, bool checked);
};

class FJ_HtmlText
{
public:
    void setText(const std::wstring& text);
};

static FJ_FuiNode* child(FJ_FuiNode* node, const char* name)
{
    return node->getChildByName(std::string(name));
}

static void clearText(FJ_FuiNode* node)
{
    static_cast<FJ_HtmlText*>(node->asFJ_HtmlText())->setText(L"");
}

static void setListPosition(FJ_FuiNode* list, FJ_FuiNode* parent, int height)
{
    FJ_FuiNode* item = list->getChildAt(0);
    item->setY(static_cast<float>(height) + (list->getY() - parent->getY()) * 2.0f);
}

static void setPanelPosition(FJ_FuiNode* panel, FJ_FuiNode* list, FJ_FuiNode* description)
{
    FJ_FuiNode* item = panel->getChildAt(0);
    item->setY(panel->getHeight() + (list->getY() - description->getY()) * 2.0f);
}

FJ_LaunchMoreOptionsMenu::FJ_LaunchMoreOptionsMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
    addEventListener(eFJEventType_CheckboxToggled, this, _handleCheckboxToggled, false);
    m_activeTab = 0;
    m_worldOptionsFocus = -1;
    m_worldTabHidden = false;
    m_gameOptionsFocus = -1;
    m_descriptionUpdated = false;
    m_previousTab = -1;
    m_menuType = 0;
}

FJ_LaunchMoreOptionsMenu::~FJ_LaunchMoreOptionsMenu()
{
}

void FJ_LaunchMoreOptionsMenu::handleCheckboxToggled(FJ_Event* event)
{
    FJ_CheckBox* checkBox = event->m_pTarget->asFJ_CheckBox();
    if (checkBox != NULL)
    {
        FJ_Base* listItem = static_cast<FJ_Base*>(checkBox->getObjListParent());
        m_pCallbackScene->handleCheckboxElementToggled(listItem->getId(), checkBox->getId(), checkBox->getId());
    }
}

void FJ_LaunchMoreOptionsMenu::_handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_LaunchMoreOptionsMenu*>(node)->handleCheckboxToggled(event);
}

FJ_Document* FJ_LaunchMoreOptionsMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_LaunchMoreOptionsMenu(renderNode);
}

unsigned int FJ_LaunchMoreOptionsMenu::getType()
{
    return 23;
}

void FJ_LaunchMoreOptionsMenu::UpdateDescription()
{
    FJ_FuiNode* gameOptions = child(this, "GameOptions");
    FJ_FuiNode* worldOptions = child(this, "WorldOptions");
    FJ_FuiNode* gameDescription = child(gameOptions, "Description_GameOptions");
    FJ_FuiNode* worldDescription = child(worldOptions, "Description_WorldOptions");

    if (!m_descriptionUpdated)
    {
        clearText(worldDescription);
        clearText(gameDescription);
        worldDescription->setVisible(false);
        gameDescription->setVisible(false);
    }

    if (m_activeTab == 0)
    {
        worldDescription->setVisible(false);
        gameDescription->setVisible(true);
    }
    else
    {
        worldDescription->setVisible(true);
        gameDescription->setVisible(false);
    }
}

void FJ_LaunchMoreOptionsMenu::SetActiveTab(int tab, bool resize)
{
    FJ_FuiNode* gameOptions = child(this, "GameOptions");
    FJ_FuiNode* worldOptions = child(this, "WorldOptions");
    FJ_FuiNode* gameList = child(gameOptions, "GameOptionsList");
    FJ_FuiNode* worldList = child(worldOptions, "WorldOptionsList");
    FJ_FuiNode* gameBackground = child(gameOptions, "BackgroundPanel");
    FJ_FuiNode* worldBackground = child(worldOptions, "BackgroundPanel");
    FJ_FuiNode* gameDescription = child(gameOptions, "Description_GameOptions");
    FJ_FuiNode* worldDescription = child(worldOptions, "Description_WorldOptions");
    FJ_FuiNode* textPanel = child(this, "TextPanelBackground");

    FJ_FuiNode* current = getStage()->getFocus();
    if (current->asFJ_Base() != NULL)
    {
        int id = current->asFJ_Base()->getId();
        if (m_previousTab == 0)
            m_worldOptionsFocus = id;
        else
            m_gameOptionsFocus = id;
    }

    bool world = tab != 0;
    gameOptions->setVisible(!world);
    worldOptions->setVisible(world);
    gameList->setVisible(gameOptions->isVisible());
    worldList->setVisible(worldOptions->isVisible());
    gameDescription->setVisible(gameOptions->isVisible());
    worldDescription->setVisible(worldOptions->isVisible());
    gotoAndStop(world ? "WorldOptions" : "GameOptions");

    if (resize)
    {
        setListPosition(worldList, worldBackground, worldList->asFJ_List()->GetListHeight());
        setPanelPosition(worldBackground, worldList, worldDescription);
        setListPosition(gameList, gameBackground, gameList->asFJ_List()->GetListHeight());
        setPanelPosition(gameBackground, gameList, gameDescription);
        UpdateDescription();
        m_descriptionUpdated = true;
    }

    if (world)
    {
        m_activeTab = world ? 1 : 0;
        SetFocus(m_worldOptionsFocus);
        if (m_worldOptionsFocus >= 0)
            setVisible(true);
    }
    else
    {
        m_activeTab = 0;
        SetFocus(m_gameOptionsFocus);
        if (m_gameOptionsFocus >= 0)
            setVisible(true);
    }
    m_previousTab = m_activeTab;
}

void FJ_LaunchMoreOptionsMenu::ChangeTab()
{
    if (!m_worldTabHidden)
    {
        SetActiveTab(m_activeTab == 0, false);
        UpdateDescription();
    }
}

void FJ_LaunchMoreOptionsMenu::SetMenuType(int type)
{
    m_menuType = type;
    m_activeTab = type == 0;
    SetActiveTab(m_activeTab, false);
}

void FJ_LaunchMoreOptionsMenu::HideWorldTab()
{
    FJ_FuiNode* gameOptions = child(this, "GameOptions");
    FJ_FuiNode* worldOptions = child(this, "WorldOptions");
    FJ_FuiNode* worldList = child(worldOptions, "WorldOptionsList");
    FJ_FuiNode* gameLabel = child(this, "LabelGame");
    FJ_FuiNode* worldLabel = child(this, "LabelWorld");
    FJ_FuiNode* backTabs = child(this, "BackTabs");
    FJ_FuiNode* gameBack = child(backTabs, "BackTabOffGameOptions");
    FJ_FuiNode* worldBack = child(backTabs, "BackTabOffWorldOptions");
    FJ_FuiNode* worldDescription = child(worldOptions, "Tab");
    FJ_FuiNode* gameDescription = child(gameOptions, "Tab");

    worldList->asFJ_List()->removeAllItems();
    SetActiveTab(0, false);
    gotoAndStop("WorldOptions");
    clearText(gameLabel);
    clearText(worldLabel);
    gameLabel->setVisible(false);
    gameBack->setVisible(false);
    worldBack->setVisible(false);
    gameOptions->setVisible(true);
    worldList->setVisible(false);
    gameDescription->setVisible(false);
    worldDescription->setVisible(false);
    m_worldTabHidden = true;
}
