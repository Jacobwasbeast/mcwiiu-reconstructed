#include "game/ui/FJ_LoadCreateJoinMenu.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_LoadCreateJoinMenu::FJ_LoadCreateJoinMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
    m_activeTab = 0;
    m_currentTab = -1;
    m_loadTabFocus = -1;
    m_newTabFocus = -1;
    m_joinTabFocus = -1;
    m_loadListHeight = 0;
    m_loadListRecessHeight = 0;
    m_loadTimerY = 0;
    m_touchEnabled = false;
    m_newTabOffset = 0;
    m_joinTabOffset = 0;
    m_loadTabOffset = 0;
    m_newTabWidth = 0;

    getChildByName(std::string("LoadGame.TabLeft"))->getChildAt(0)->gotoAndStop(1);
    getChildByName(std::string("NewGame.TabMiddle"))->getChildAt(0)->gotoAndStop(2);
    getChildByName(std::string("JoinGame.TabRight"))->getChildAt(0)->gotoAndStop(3);

    FJ_FuiNode* loadList = getChildByName(std::string("LoadGame"))->getChildByName(std::string("LoadGameList"));
    if (loadList != NULL)
        m_loadListHeight = static_cast<int>(loadList->getHeight());
    FJ_FuiNode* recess = getChildByName(std::string("LoadGame"))->getChildByName(std::string("LoadGameListRecessPanel"));
    if (recess != NULL && recess->getChildAt(0) != NULL)
        m_loadListRecessHeight = static_cast<int>(recess->getChildAt(0)->getHeight());
    FJ_FuiNode* timer = getChildByName(std::string("LoadGame.LoadGameTimer"));
    if (timer != NULL)
        m_loadTimerY = static_cast<int>(timer->getY());
    FJ_FuiNode* loadTitle = getChildByName(std::string("LoadGameTabTitle"));
    if (loadTitle != NULL)
        m_loadTabOffset = static_cast<int>(loadTitle->getY());
    FJ_FuiNode* newTitle = getChildByName(std::string("NewGameTabTitle"));
    if (newTitle != NULL)
        m_newTabOffset = static_cast<int>(newTitle->getY());
    FJ_FuiNode* newList = getChildByName(std::string("NewGame"))->getChildByName(std::string("NewGameList"));
    FJ_FuiNode* joinTitle = getChildByName(std::string("JoinGameTabTitle"));
    if (joinTitle != NULL)
        m_joinTabOffset = static_cast<int>(joinTitle->getY());
    if (newList != NULL && joinTitle != NULL)
        m_newTabWidth = static_cast<int>(-(joinTitle->getWidth() * 0.5f - (newList->getChildAt(0)->getWidth() * 0.5f + newTitle->getX())));

    FJ_FuiNode* loadTab = getChildByName(std::string("LoadGame.TabLeft"));
    FJ_FuiNode* newTab = getChildByName(std::string("NewGame.TabMiddle"));
    FJ_FuiNode* joinTab = getChildByName(std::string("JoinGame.TabRight"));
    if (loadTab != NULL && newTab != NULL && joinTab != NULL)
    {
        const float scale = 0.5f;
        loadTab->getChildAt(0)->setWidth(loadTab->getChildAt(0)->getWidth() * scale);
        newTab->getChildAt(0)->setWidth(newTab->getChildAt(0)->getWidth() * scale);
        joinTab->getChildAt(0)->setWidth(joinTab->getChildAt(0)->getWidth() * scale);
    }
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
    FJ_FuiNode* load = getChildByName(std::string("LoadGame"));
    FJ_FuiNode* create = getChildByName(std::string("NewGame"));
    FJ_FuiNode* join = getChildByName(std::string("JoinGame"));
    FJ_FuiNode* loadList = load->getChildByName(std::string("LoadGameList"));
    FJ_FuiNode* createList = create->getChildByName(std::string("NewGameList"));
    FJ_FuiNode* joinList = join->getChildByName(std::string("JoinGameGamesList"));

    if (tab == 0)
    {
        load->setVisible(true);
        create->setVisible(false);
        join->setVisible(false);
        loadList->setVisible(load->isVisible());
        createList->setVisible(false);
        joinList->setVisible(false);
        gotoAndStop(std::string("LoadGame"));
    }
    else if (tab == 1)
    {
        load->setVisible(false);
        create->setVisible(true);
        join->setVisible(false);
        loadList->setVisible(false);
        createList->setVisible(create->isVisible());
        joinList->setVisible(false);
        gotoAndStop(std::string("NewGame"));
    }
    else if (tab == 2)
    {
        load->setVisible(false);
        create->setVisible(false);
        join->setVisible(true);
        loadList->setVisible(false);
        createList->setVisible(false);
        joinList->setVisible(join->isVisible());
        gotoAndStop(std::string("JoinGame"));
    }
    else
    {
        m_activeTab = tab;
        m_currentTab = tab;
        return;
    }

    FJ_FuiNodeStage* stage = getStage();
    FJ_FuiNode* focus = stage == NULL ? NULL : stage->getFocus();
    if (focus != NULL && focus->isFJ_Base())
    {
        int focusId = focus->asFJ_Base()->getId();
        if (m_currentTab == 0)
            m_loadTabFocus = focusId;
        else if (m_currentTab == 1)
            m_newTabFocus = focusId;
        else if (m_currentTab == 2)
            m_joinTabFocus = focusId;
    }

    SetFocus(tab == 0 ? m_loadTabFocus : (tab == 1 ? m_newTabFocus : m_joinTabFocus));
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
    FJ_FuiNode* saveBar = getChildByName(std::string("LoadGame.LoadGameList"))->getChildByName(std::string("SaveSizeBar"));
    if (saveBar != NULL)
        saveBar->setVisible(show);
    if (show)
    {
        FJ_FuiNode* list = getChildByName(std::string("NewGame.NewGameList"));
        if (list != NULL && saveBar != NULL)
            list->setY(saveBar->getHeight());
        FJ_FuiNode* recess = getChildByName(std::string("LoadGame.LoadGameListRecessPanel"));
        if (recess != NULL && saveBar != NULL)
            recess->setY(saveBar->getHeight());
    }
    else
    {
        FJ_FuiNode* list = getChildByName(std::string("NewGame.NewGameList"));
        if (list != NULL)
            list->setY(static_cast<float>(m_loadListHeight));
        FJ_FuiNode* recess = getChildByName(std::string("LoadGame.LoadGameListRecessPanel"));
        if (recess != NULL)
            recess->setY(static_cast<float>(m_loadListRecessHeight));
        FJ_FuiNode* timer = getChildByName(std::string("LoadGame.LoadGameTimer"));
        if (timer != NULL)
            timer->setY(static_cast<float>(m_loadTimerY));
    }
}

void FJ_LoadCreateJoinMenu::HideLoadTab()
{
    if (!m_touchEnabled)
    {
        getChildByName(std::string("LoadGame"))->setVisible(false);
        getChildByName(std::string("LoadGameTabTitle"))->setVisible(false);
        getChildByName(std::string("TabOff_Load"))->setVisible(false);
        getChildByName(std::string("TabOff_Create"))->getChildAt(0)->gotoAndStop(1);
        getChildByName(std::string("NewGame.TabMiddle"))->getChildAt(0)->gotoAndStop(3);
        getChildByName(std::string("TabOff_Create"))->setX(getChildByName(std::string("LoadGame"))->getX());
        getChildByName(std::string("TabOff_Create"))->getChildAt(0)->setWidth(getChildByName(std::string("LoadGame"))->getChildAt(0)->getWidth() * 0.5f);
        getChildByName(std::string("TabOff_Create"))->getChildAt(0)->setWidth(getChildByName(std::string("NewGame.TabMiddle"))->getChildAt(0)->getWidth() * 0.5f);
        getChildByName(std::string("TouchTabLoad"))->setVisible(false);
        getChildByName(std::string("TouchTabCreate"))->setVisible(false);
    }
}
