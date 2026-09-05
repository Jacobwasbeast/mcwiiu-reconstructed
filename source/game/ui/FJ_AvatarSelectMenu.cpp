#include "game/ui/FJ_AvatarSelectMenu.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

namespace
{
FJ_FuiNode* child(FJ_FuiNode* node, const char* name)
{
    return node->getChildByName(std::string(name));
}

void setTabState(FJ_FuiNode* tab, FJ_FuiNode* list, bool visible)
{
    tab->setVisible(visible);
    list->setVisible(visible && tab->isVisible());
}
}

FJ_AvatarSelectMenu::FJ_AvatarSelectMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
    , m_activeTab(0)
    , m_currentTab(-1)
    , m_userTabFocus(-1)
    , m_favouriteTabFocus(-1)
    , m_guestTabFocus(-1)
{
    child(this, "UserTab.TabLeft")->getChildAt(0)->gotoAndStop(1);
    child(this, "FavouriteTab.TabMiddle")->getChildAt(0)->gotoAndStop(2);
    child(this, "GuestTab.TabRight")->getChildAt(0)->gotoAndStop(3);
    child(this, "TabLeftOff")->getChildAt(0)->gotoAndStop(1);
    child(this, "TabMiddleOff")->getChildAt(0)->gotoAndStop(2);
    child(this, "TabRightOff")->getChildAt(0)->gotoAndStop(3);
    SetActiveTab(m_activeTab);
}

FJ_AvatarSelectMenu::~FJ_AvatarSelectMenu()
{
}

FJ_AvatarSelectMenu* FJ_AvatarSelectMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_AvatarSelectMenu(renderNode);
}

unsigned int FJ_AvatarSelectMenu::getType()
{
    return 0x50;
}

void FJ_AvatarSelectMenu::SetActiveTab(int tab)
{
    FJ_FuiNode* userTab = child(this, "UserTab");
    FJ_FuiNode* userList = child(userTab, "UserList");
    FJ_FuiNode* favouriteTab = child(this, "FavouriteTab");
    FJ_FuiNode* favouriteList = child(favouriteTab, "FavouriteList");
    FJ_FuiNode* guestTab = child(this, "GuestTab");
    FJ_FuiNode* guestList = child(guestTab, "GuestList");

    if (tab == 0)
    {
        setTabState(userTab, userList, true);
        setTabState(favouriteTab, favouriteList, false);
        setTabState(guestTab, guestList, false);
        gotoAndStop(std::string("UserTab"));
    }
    else if (tab == 1)
    {
        setTabState(userTab, userList, false);
        setTabState(favouriteTab, favouriteList, true);
        setTabState(guestTab, guestList, false);
        gotoAndStop(std::string("FavouriteTab"));
    }
    else if (tab == 2)
    {
        setTabState(userTab, userList, false);
        setTabState(favouriteTab, favouriteList, false);
        setTabState(guestTab, guestList, true);
        gotoAndStop(std::string("GuestTab"));
    }
    else
    {
        m_currentTab = tab;
        m_activeTab = tab;
        return;
    }

    FJ_FuiNodeStage* stage = getStage();
    FJ_FuiNode* focus = stage == NULL ? NULL : stage->getFocus();
    if (focus != NULL && focus->isFJ_Base())
    {
        int focusId = focus->asFJ_Base()->getId();
        if (m_currentTab == 0)
            m_userTabFocus = focusId;
        else if (m_currentTab == 1)
            m_favouriteTabFocus = focusId;
        else if (m_currentTab == 2)
            m_guestTabFocus = focusId;
    }

    int focusId = m_userTabFocus;
    if (tab == 1)
        focusId = m_favouriteTabFocus;
    else if (tab == 2)
        focusId = m_guestTabFocus;
    SetFocus(focusId);
    m_currentTab = tab;
    m_activeTab = tab;
}

void FJ_AvatarSelectMenu::handlePress(FJ_Event* event)
{
    FJ_Button* button = event->m_pTarget->asFJ_Button();
    if (button != NULL)
    {
        FJ_FuiNode* parent = button->getObjListParent();
        m_pCallbackScene->handlePress(parent->asFJ_Base()->getId(), button->getId());
        button->setButtonPressed(false);
    }
}
