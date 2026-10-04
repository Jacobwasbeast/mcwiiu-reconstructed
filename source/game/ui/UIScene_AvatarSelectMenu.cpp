#include "game/ui/UIScene_AvatarSelectMenu.hpp"
#include "game/core/StringIds.hpp"
#include <cwchar>
#include <cstring>
#include <string>
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/FJ_AvatarSelectMenu.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/MiiData.hpp"
#include "game/core/GameSettings.hpp"

extern CInput* g_pInput;

extern ConsoleUIController gConsoleUIController;
extern CProfile* gProfile;

UIScene_AvatarSelectMenu::TabControls::TabControls()
    : m_tab(), m_title(), m_timer(), m_miis()
{
}

void UIScene_AvatarSelectMenu::TabControls::ShowTimer(bool show)
{
    m_timer.setVisible(show);
}

UIScene_AvatarSelectMenu::UIScene_AvatarSelectMenu(int player, void* data, UILayer* layer)
    : UIScene(player, layer), m_userTabTouch(), m_favouriteTabTouch(), m_guestTabTouch(), m_userControls(),
      m_favouriteControls(), m_guestControls()
{
    initialiseMovie();
    gApp.SetLiveLinkRequired(true);
    m_userControls.m_owner = this;
    m_pad = player;
    m_guestControls.m_owner = this;
    m_favouriteControls.m_owner = this;
    m_userControls.m_list.init(0);
    m_favouriteControls.m_list.init(1);
    m_guestControls.m_list.init(2);
    m_userControls.m_title.init(UIString(0xc835d6e5));
    m_favouriteControls.m_title.init(UIString(IDS_FAVOURITE_MIIS));
    m_guestControls.m_title.init(UIString(IDS_GUEST_MIIS));
    m_userControls.m_timer.setVisible(false);
    m_favouriteControls.m_timer.setVisible(false);
    m_guestControls.m_timer.setVisible(false);
    m_miisAdded = false;
    m_initialised = false;
    m_unknown1 = false;
    m_navigateBack = false;
    m_currentTab = 0;

    int choice = 0;
    for (int pad = 0; pad < 4; ++pad)
    {
        if (!gProfile->IsSignedIn(pad))
        {
            choice = pad;
            break;
        }
    }
    m_choice = choice;
    gConsoleUIController.m_pressStartBlocked |= 1 << m_choice;
    gConsoleUIController.HidePressStart();
    m_userTabTouch.init(3);
    m_favouriteTabTouch.init(4);
    m_guestTabTouch.init(5);
    gConsoleUIController.TouchBoxRebuild(this);
    m_activeTab = 0;
}

UIScene_AvatarSelectMenu::~UIScene_AvatarSelectMenu()
{
    gConsoleUIController.m_pressStartBlocked &= ~(1 << m_choice);
}

unsigned int UIScene_AvatarSelectMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    m_userControls.m_tab.setupControl(this, root, "UserTab");
    m_controls.push_back(&m_userControls.m_tab);
    fuiRenderNode* userTab = m_userControls.m_tab.getFuiRenderNode();
    m_userControls.m_list.setupControl(this, userTab, "UserList");
    m_controls.push_back(&m_userControls.m_list);
    m_userControls.m_timer.setupControl(this, userTab, "UserTimer");
    m_controls.push_back(&m_userControls.m_timer);
    m_userTabTouch.setupControl(this, userTab, "TouchTabUser");
    m_controls.push_back(&m_userTabTouch);
    m_favouriteControls.m_tab.setupControl(this, root, "FavouriteTab");
    m_controls.push_back(&m_favouriteControls.m_tab);
    fuiRenderNode* favouriteTab = m_favouriteControls.m_tab.getFuiRenderNode();
    m_favouriteControls.m_list.setupControl(this, favouriteTab, "FavouriteList");
    m_controls.push_back(&m_favouriteControls.m_list);
    m_favouriteControls.m_timer.setupControl(this, favouriteTab, "FavouriteTimer");
    m_controls.push_back(&m_favouriteControls.m_timer);
    m_favouriteTabTouch.setupControl(this, favouriteTab, "TouchTabFavourite");
    m_controls.push_back(&m_favouriteTabTouch);
    m_guestControls.m_tab.setupControl(this, root, "GuestTab");
    m_controls.push_back(&m_guestControls.m_tab);
    fuiRenderNode* guestTab = m_guestControls.m_tab.getFuiRenderNode();
    m_guestControls.m_list.setupControl(this, guestTab, "GuestTabList");
    m_controls.push_back(&m_guestControls.m_list);
    m_guestControls.m_timer.setupControl(this, guestTab, "GuestTimer");
    m_controls.push_back(&m_guestControls.m_timer);
    m_guestTabTouch.setupControl(this, guestTab, "TouchTabGuest");
    m_controls.push_back(&m_guestTabTouch);
    m_userControls.m_title.setupControl(this, root, "UserTabTitle");
    m_controls.push_back(&m_userControls.m_title);
    m_favouriteControls.m_title.setupControl(this, root, "FavouriteTabTitle");
    m_controls.push_back(&m_favouriteControls.m_title);
    m_guestControls.m_title.setupControl(this, root, "GuestTabTitle");
    m_controls.push_back(&m_guestControls.m_title);
    return 1;
}

void UIScene_AvatarSelectMenu::AddMiiItem(TabControls& controls, MiiDataEntry* mii)
{
    for (int i = 0; i < 4; ++i)
    {
        if (!gProfile->IsSignedIn(i)) continue;
        PlayerUID uid(0);
        gProfile->GetXUID(i, &uid, false);
        if (mii->mIsAccountMii ? uid.uuid == mii->mUuid : std::memcmp(uid.mii.createId, mii->mCreatorId, 10) == 0)
            return;
    }
    std::wstring name;
    for (int i = 0; i < 11 && mii->mName[i] != 0; ++i) name.push_back((wchar_t)mii->mName[i]);
    controls.m_list.addItem(name);
    wchar_t texture[32];
    std::swprintf(texture, 32, L"Mii_num_%d", controls.m_list.getItemCount() - 1);
    std::wstring textureName(texture);
    registerSubstitutionTexture(textureName, 0, (unsigned int)mii->mIconTexture, false);
    controls.m_list.setTextureName(controls.m_list.getItemCount() - 1, textureName);
    controls.m_miis.push_back(mii);
}

void UIScene_AvatarSelectMenu::AddMiiItems()
{
    int count = gProfile->GetNumSysUserMiis();
    for (int i = 0; i < count; ++i) AddMiiItem(m_userControls, gProfile->GetSysUserMiiData(i));
    count = gProfile->GetNumFavouriteMiis();
    for (int i = 0; i < count; ++i) AddMiiItem(m_favouriteControls, gProfile->GetFavouriteMiiData(i));
    count = gProfile->GetNumGuestMiis();
    for (int i = 0; i < count; ++i) AddMiiItem(m_guestControls, gProfile->GetGuestMiiData(i));
}

void UIScene_AvatarSelectMenu::updateTooltips() { hasFocus(m_playerId); }
void UIScene_AvatarSelectMenu::updateComponents() {}
void UIScene_AvatarSelectMenu::handleDestroy() {}
void UIScene_AvatarSelectMenu::handleLoseFocus() {}

void UIScene_AvatarSelectMenu::handleGainFocus(bool hadFocus)
{
    updateTooltips();
    if (hadFocus) { gApp.SetLiveLinkRequired(true); m_initialised = false; }
}

void UIScene_AvatarSelectMenu::handleTimerComplete(int) {}

bool UIScene_AvatarSelectMenu::handleKeyDown(int, int action, bool)
{
    if (m_initialised) return false;
    bool consumed = false;
    int tab = m_currentTab;
    switch (action)
    {
    case EControllerActions_MenuB:
    case EControllerActions_MenuCancel:
        gApp.RequestSignInUIWiiUReturnedSelected(NULL, m_choice, m_pad);
        gConsoleUIController.NavigateBack(GetPad(), false, EUIScene_None, EUILayer_Count);
        consumed = true;
        break;
    case EControllerActions_MenuRight:
    case EControllerActions_MenuRightScroll:
        tab = tab + 1;
        if (tab >= 3) tab = 0;
        m_currentTab = tab;
        SelectTab(tab);
        updateTooltips();
        break;
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuLeftScroll:
        tab = tab - 1;
        if (tab < 0) tab = 2;
        m_currentTab = tab;
        SelectTab(tab);
        updateTooltips();
        break;
    }
    return consumed;
}

void UIScene_AvatarSelectMenu::handleInput(int, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    if (m_initialised)
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    if (!pressed)
        return;
    switch (action)
    {
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        handled = true;
        break;
    default:
        handled = handleKeyDown(m_playerId, action, repeat);
        break;
    }
}

void UIScene_AvatarSelectMenu::handleFocusChange(int focus, int childFocus)
{
    switch (focus)
    {
    case 0:
        m_userFocus = childFocus;
        break;
    case 1:
        m_favouriteFocus = childFocus;
        break;
    case 2:
        m_guestFocus = childFocus;
        break;
    }
    updateTooltips();
}

void UIScene_AvatarSelectMenu::handleInitFocus(int, int) {}

void UIScene_AvatarSelectMenu::handleTouchInput(unsigned int playerId, int touchX, int touchY,
                                               int touchBox, bool pressed, bool repeat,
                                               bool released)
{
    if (!pressed) return;
    if (touchBox <= 2 || touchBox >= 6) return;
    if (m_activeTab == touchBox - 3) return;
    m_activeTab = touchBox - 3;
    updateTooltips();
    SelectTab(m_activeTab);
}

UIControl* UIScene_AvatarSelectMenu::GetMainPanel()
{
    if (m_activeTab == 1) return &m_favouriteControls.m_tab;
    if (m_activeTab == 2) return &m_guestControls.m_tab;
    return &m_userControls.m_tab;
}

void UIScene_AvatarSelectMenu::tick()
{
    UIScene::tick();
    if (g_pInput->ButtonPressed(gProfile->GetPrimaryPad(), EControllerActions_MenuCancel, 0))
    {
        gApp.RequestSignInUIWiiUReturnedSelected(NULL, m_choice, m_pad);
        gConsoleUIController.NavigateBack(GetPad(), false, EUIScene_None, EUILayer_Count);
    }
    if (m_navigateBack) navigateBack();
    if (hasFocus(m_playerId) && !m_miisAdded) { AddMiiItems(); m_miisAdded = true; }
}

void UIScene_AvatarSelectMenu::SelectTab(int tab)
{
    m_activeTab = tab;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    FJ_AvatarSelectMenu* menu = (FJ_AvatarSelectMenu*)m_movie->getRootNode()->getFuiNode();
    menu->SetActiveTab(tab);
    gConsoleUIController.TouchBoxRebuild(this);
}

std::wstring UIScene_AvatarSelectMenu::getMoviePath() { return std::wstring(L"AvatarSelectMenu"); }

void UIScene_AvatarSelectMenu::handlePress(int controlId, int childId)
{
    if (controlId == 0)
        gApp.RequestSignInUIWiiUReturnedSelected(m_userControls.m_miis[childId], m_choice, m_pad);
    else if (controlId == 1)
        gApp.RequestSignInUIWiiUReturnedSelected(m_favouriteControls.m_miis[childId], m_choice, m_pad);
    else if (controlId == 2)
        gApp.RequestSignInUIWiiUReturnedSelected(m_guestControls.m_miis[childId], m_choice, m_pad);
    else
        gApp.RequestSignInUIWiiUReturnedSelected(NULL, m_choice, m_pad);
    navigateBack();
}

int UIScene_AvatarSelectMenu::getSceneType() { return EUIScene_LoadCreateJoinMenu; }
unsigned int UIScene_AvatarSelectMenu::GetPad() { return (unsigned int)m_playerId; }
