#include "game/ui/FJ_AvatarSelectMenu.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Button.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_AvatarSelectMenu::FJ_AvatarSelectMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
    , m_activeTab(0)
    , m_currentTab(-1)
    , m_userTabFocus(-1)
    , m_favouriteTabFocus(-1)
    , m_guestTabFocus(-1)
{
    getChildByName("UserTab.TabLeft")->getChildAt(0)->gotoAndStop(1);
    getChildByName("FavouriteTab.TabMiddle")->getChildAt(0)->gotoAndStop(2);
    getChildByName("GuestTab.TabRight")->getChildAt(0)->gotoAndStop(3);
    getChildByName("TabLeftOff")->getChildAt(0)->gotoAndStop(1);
    getChildByName("TabMiddleOff")->getChildAt(0)->gotoAndStop(2);
    getChildByName("TabRightOff")->getChildAt(0)->gotoAndStop(3);
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
    FJ_FuiNode* userTab = getChildByName("UserTab");
    FJ_FuiNode* userList = userTab->getChildByName("UserList");
    FJ_FuiNode* favouriteTab = getChildByName("FavouriteTab");
    FJ_FuiNode* favouriteList = favouriteTab->getChildByName("FavouriteList");
    FJ_FuiNode* guestTab = getChildByName("GuestTab");
    FJ_FuiNode* guestList = guestTab->getChildByName("GuestList");
    FJ_FuiNode* activeTab = userTab;

    if (tab == 0)
    {
        userTab->setVisible(true);
        favouriteTab->setVisible(false);
        guestTab->setVisible(false);
        userList->setVisible(userTab->isVisible());
        favouriteList->setVisible(favouriteTab->isVisible());
        guestList->setVisible(guestTab->isVisible());
        gotoAndStop("UserTab");
    }
    else if (tab == 1)
    {
        activeTab = favouriteTab;
        userTab->setVisible(false);
        favouriteTab->setVisible(true);
        guestTab->setVisible(false);
        userList->setVisible(userTab->isVisible());
        favouriteList->setVisible(favouriteTab->isVisible());
        guestList->setVisible(guestTab->isVisible());
        gotoAndStop("FavouriteTab");
    }
    else if (tab == 2)
    {
        activeTab = guestTab;
        userTab->setVisible(false);
        favouriteTab->setVisible(false);
        guestTab->setVisible(true);
        userList->setVisible(userTab->isVisible());
        favouriteList->setVisible(favouriteTab->isVisible());
        guestList->setVisible(guestTab->isVisible());
        gotoAndStop("GuestTab");
    }
    else
    {
        m_currentTab = tab;
        m_activeTab = tab;
        return;
    }

    FJ_FuiNode* focus = getStage()->getFocus();
    if (focus->isFJ_Base() && m_currentTab != tab)
    {
        int focusId = focus->asFJ_Base()->getId();
        if (m_currentTab == 0)
            m_userTabFocus = focusId;
        else if (m_currentTab == 1)
            m_favouriteTabFocus = focusId;
        else if (m_currentTab == 2)
            m_guestTabFocus = focusId;
    }

    m_root->removeEventListener(eFJEventType_Press, this, FJ_Document::_handlePress);
    m_root = activeTab;
    m_root->addEventListener(eFJEventType_Press, this, FJ_Document::_handlePress, false);
    int focusId = tab == 0 ? m_userTabFocus :
                  (tab == 1 ? m_favouriteTabFocus : m_guestTabFocus);
    SetFocus(focusId);
    if (focusId >= 0)
        setVisible(true);
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
