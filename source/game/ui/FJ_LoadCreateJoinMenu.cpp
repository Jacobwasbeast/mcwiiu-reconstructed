#include "game/ui/FJ_LoadCreateJoinMenu.hpp"

#include <string>

#include "game/ui/FJ_Button.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Label_Blink.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_LoadCreateJoinMenu::FJ_LoadCreateJoinMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
    , m_activeTab(0)
    , m_currentTab(-1)
    , m_loadTabFocus(-1)
    , m_newTabFocus(-1)
    , m_joinTabFocus(-1)
    , m_loadListHeight(0)
    , m_loadListRecessHeight(0)
    , m_loadTimerY(0)
    , m_touchEnabled(false)
    , m_loadTabTitleY(0)
    , m_joinTabTitleY(0)
    , m_newTabX(0)
    , m_joinTabX(0)
{
    getChildByName("LoadGame.TabLeft")->getChildAt(0)->gotoAndStop(1);
    getChildByName("NewGame.TabMiddle")->getChildAt(0)->gotoAndStop(2);
    getChildByName("JoinGame.TabRight")->getChildAt(0)->gotoAndStop(3);

    FJ_FuiNode* loadGame = getChildByName("LoadGame");
    FJ_FuiNode* loadList = loadGame->getChildByName("LoadGameList");
    if (loadList != NULL)
        m_loadListHeight = static_cast<int>(loadList->getHeight());

    FJ_FuiNode* loadListRecess = loadGame->getChildByName("LoadGameListRecessPanel");
    if (loadListRecess != NULL)
        m_loadListRecessHeight = static_cast<int>(loadListRecess->getChildAt(0)->getHeight());

    if (loadList != NULL)
        m_loadTimerY = static_cast<int>(getChildByName("LoadGame.LoadGameTimer")->getY());

    m_loadTabTitleY = static_cast<int>(getChildByName("LoadGameTabTitle")->getY());
    m_joinTabTitleY = static_cast<int>(getChildByName("JoinGameTabTitle")->getY());

    float loadX = loadGame->getX();
    float quarterWidth = loadGame->getChildAt(0)->getWidth() * 0.25f;
    m_newTabX = static_cast<int>(-(getChildByName("NewGameTabTitle")->getWidth() * 0.5f -
                                   (quarterWidth + loadX)));
    m_joinTabX = static_cast<int>(-(getChildByName("JoinGameTabTitle")->getWidth() * 0.5f -
                                    (quarterWidth * 3.0f + loadX)));
    SetActiveTab(m_activeTab);
}

FJ_LoadCreateJoinMenu::~FJ_LoadCreateJoinMenu()
{
}

FJ_LoadCreateJoinMenu* FJ_LoadCreateJoinMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_LoadCreateJoinMenu(renderNode);
}

unsigned int FJ_LoadCreateJoinMenu::getType()
{
    return 8;
}

void FJ_LoadCreateJoinMenu::SetActiveTab(int tab)
{
    FJ_FuiNode* loadTab = getChildByName("LoadGame");
    FJ_FuiNode* newTab = getChildByName("NewGame");
    FJ_FuiNode* joinTab = getChildByName("JoinGame");
    FJ_FuiNode* loadList = getChildByName("LoadGame.LoadGameList");
    FJ_FuiNode* newList = getChildByName("NewGame.NewGameList");
    FJ_FuiNode* joinList = getChildByName("JoinGame.JoinGameGamesList");
    FJ_FuiNode* activeTab;

    if (tab == 0)
    {
        activeTab = loadTab;
        loadTab->setVisible(true);
        newTab->setVisible(false);
        joinTab->setVisible(false);
        loadList->setVisible(loadTab->isVisible());
        newList->setVisible(newTab->isVisible());
        joinList->setVisible(joinTab->isVisible());
        gotoAndStop("LoadGame");
    }
    else if (tab == 1)
    {
        activeTab = newTab;
        loadTab->setVisible(false);
        newTab->setVisible(true);
        joinTab->setVisible(false);
        loadList->setVisible(loadTab->isVisible());
        newList->setVisible(newTab->isVisible());
        joinList->setVisible(joinTab->isVisible());
        gotoAndStop("NewGame");
    }
    else if (tab == 2)
    {
        activeTab = joinTab;
        loadTab->setVisible(false);
        newTab->setVisible(false);
        joinTab->setVisible(true);
        loadList->setVisible(loadTab->isVisible());
        newList->setVisible(newTab->isVisible());
        joinList->setVisible(joinTab->isVisible());
        gotoAndStop("JoinGame");
    }
    else
    {
        m_activeTab = tab;
        m_currentTab = tab;
        return;
    }

    FJ_FuiNode* focus = getStage()->getFocus();
    if (focus->isFJ_Base() && m_currentTab != tab)
    {
        int focusId = focus->asFJ_Base()->getId();
        if (m_currentTab == 0)
            m_loadTabFocus = focusId;
        else if (m_currentTab == 1)
            m_newTabFocus = focusId;
        else if (m_currentTab == 2)
            m_joinTabFocus = focusId;
    }

    m_root->removeEventListener(eFJEventType_Press, this, FJ_Document::_handlePress);
    m_root = activeTab;
    m_root->addEventListener(eFJEventType_Press, this, FJ_Document::_handlePress, false);
    int focusId = tab == 0 ? m_loadTabFocus :
                  (tab == 1 ? m_newTabFocus : m_joinTabFocus);
    SetFocus(focusId);
#if defined(RECON_NATIVE)
    if (tab == 2 && focusId < 0 && joinList->asFJ_List() != NULL)
        getStage()->setFocus(joinList);
#endif
    if (focusId >= 0)
        setVisible(true);

    if (tab == 2)
        getChildByName("JoinGameTabTitle")->asFJ_Label_Blink()->EnableBlink(false);

    if (m_touchEnabled)
    {
        if (tab == 1)
        {
            getChildByName("NewGameTabTitle")->setY(static_cast<float>(m_loadTabTitleY));
            getChildByName("JoinGameTabTitle")->setY(static_cast<float>(m_joinTabTitleY));
        }
        else if (tab == 2)
        {
            getChildByName("NewGameTabTitle")->setY(static_cast<float>(m_joinTabTitleY));
            getChildByName("JoinGameTabTitle")->setY(static_cast<float>(m_loadTabTitleY));
        }
    }

    m_currentTab = tab;
    m_activeTab = tab;
}

void FJ_LoadCreateJoinMenu::handlePress(FJ_Event* event)
{
    FJ_Button* button = event->m_pTarget->asFJ_Button();
    if (button != NULL)
    {
        FJ_FuiNode* parent = button->getObjListParent();
        m_pCallbackScene->handlePress(parent->asFJ_List()->getId(), button->getId());
        button->setButtonPressed(false);
    }
}

void FJ_LoadCreateJoinMenu::SetMatchesAvailable(bool available)
{
    FJ_Label_Blink* label = getChildByName(std::string("JoinGameTabTitle"))->asFJ_Label_Blink();
    label->EnableBlink(m_activeTab == 2 ? false : available);
}

void FJ_LoadCreateJoinMenu::ShowSaveSizeBar(bool show)
{
    FJ_FuiNode* loadGame = getChildByName("LoadGame");
    FJ_FuiNode* saveSizeBar = loadGame->getChildByName("SaveSizeBar");
    if (saveSizeBar != NULL)
        saveSizeBar->setVisible(show);

    FJ_FuiNode* loadList = getChildByName("LoadGame.LoadGameList");
    FJ_FuiNode* loadListRecess = getChildByName("LoadGame.LoadGameListRecessPanel");
    FJ_FuiNode* loadTimer = getChildByName("LoadGame.LoadGameTimer");
    if (!show)
    {
        FJ_FuiNode* newList = getChildByName("NewGame.NewGameList");
        if (loadList != NULL)
            loadList->setHeight(newList->getHeight());

        FJ_FuiNode* newListRecess = getChildByName("NewGame.NewGameListRecessPanel");
        if (loadListRecess != NULL)
            loadListRecess->getChildAt(0)->setHeight(newListRecess->getChildAt(0)->getHeight());

        if (loadList != NULL)
            loadTimer->setY(getChildByName("NewGame.NewGameTimer")->getY());
    }
    else
    {
        if (loadList != NULL)
            loadList->setHeight(static_cast<float>(m_loadListHeight));
        if (loadListRecess != NULL)
            loadListRecess->getChildAt(0)->setHeight(static_cast<float>(m_loadListRecessHeight));
        if (loadList != NULL)
            loadTimer->setY(static_cast<float>(m_loadTimerY));
    }
}

void FJ_LoadCreateJoinMenu::HideLoadTab()
{
    if (m_touchEnabled)
        return;

    FJ_FuiNode* loadGame = getChildByName("LoadGame");
    loadGame->setVisible(false);
    getChildByName("LoadGameTabTitle")->setVisible(false);
    getChildByName("TabOff_Load")->setVisible(false);
    getChildByName("NewGame.TabMiddle")->getChildAt(0)->gotoAndStop(1);
    getChildByName("JoinGame.TabRight")->getChildAt(0)->gotoAndStop(3);

    float loadX = loadGame->getX();
    float tabWidth = loadGame->getChildAt(0)->getWidth() * 0.5f;

    FJ_FuiNode* createTabOff = getChildByName("TabOff_Create");
    createTabOff->setX(loadX);
    createTabOff->getChildAt(0)->setWidth(tabWidth);

    FJ_FuiNode* newTab = getChildByName("NewGame.TabMiddle");
    newTab->setX(0.0f);
    newTab->getChildAt(0)->setWidth(tabWidth);

    FJ_FuiNode* touchLoadTab = getChildByName("LoadGame.TouchTabLoad");
    if (touchLoadTab != NULL)
    {
        touchLoadTab->setVisible(false);
        FJ_FuiNode* touchCreateTab = getChildByName("NewGame.TouchTabCreate");
        touchCreateTab->setX(0.0f);
        touchCreateTab->setWidth(newTab->getChildAt(0)->getWidth());
    }

    FJ_FuiNode* joinTabOff = getChildByName("TabOff_Join");
    joinTabOff->setX(loadX + tabWidth);
    joinTabOff->getChildAt(0)->setWidth(tabWidth);

    FJ_FuiNode* joinTab = getChildByName("JoinGame.TabRight");
    joinTab->setX(tabWidth);
    joinTab->getChildAt(0)->setWidth(tabWidth);

    if (touchLoadTab != NULL)
    {
        FJ_FuiNode* touchJoinTab = getChildByName("JoinGame.TouchTabJoin");
        touchJoinTab->setX(tabWidth);
        touchJoinTab->setWidth(tabWidth);
    }
}
