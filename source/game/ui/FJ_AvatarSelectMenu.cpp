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
    FJ_FuiNode* activeList = userList;
    int focusId = m_userTabFocus;

    if (tab == 0)
    {
        userTab->setVisible(true);
        favouriteTab->setVisible(false);
        guestTab->setVisible(false);
    }
    else if (tab == 1)
    {
        activeTab = favouriteTab;
        activeList = favouriteList;
        focusId = m_favouriteTabFocus;
        userTab->setVisible(false);
        favouriteTab->setVisible(true);
        guestTab->setVisible(false);
    }
    else if (tab == 2)
    {
        activeTab = guestTab;
        activeList = guestList;
        focusId = m_guestTabFocus;
        userTab->setVisible(false);
        favouriteTab->setVisible(false);
        guestTab->setVisible(true);
    }
    else
    {
        m_currentTab = tab;
        m_activeTab = tab;
        return;
    }

    userList->setVisible(userTab->isVisible());
    favouriteList->setVisible(favouriteTab->isVisible());
    guestList->setVisible(guestTab->isVisible());
    gotoAndStop(tab == 0 ? "UserTab" : (tab == 1 ? "FavouriteTab" : "GuestTab"));

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

    removeEventListener(eFJEventType_Press, this, FJ_Document::_handlePress);
    activeTab->addEventListener(eFJEventType_Press, this, FJ_Document::_handlePress, false);
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
