#include "game/ui/UIScene_AvatarSelectMenu.hpp"
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

static unsigned int s_avatarSelectSignInMask = 0;

UIScene_AvatarSelectMenu::TabControls::TabControls()
    : UIControl(), m_list(), m_title(), m_timer(), m_miis()
{
}

void UIScene_AvatarSelectMenu::TabControls::ShowTimer(bool show)
{
    m_timer.setVisible(show);
}

UIScene_AvatarSelectMenu::UIScene_AvatarSelectMenu(int player, void* data, UILayer* layer)
    : UIScene(player, layer), m_userTabTouch(), m_favouriteTabTouch(), m_guestTabTouch(),
      m_currentTab(0), m_initialised(false), m_unknown1(false), m_navigateBack(false),
      m_userControls(), m_favouriteControls(), m_guestControls(), m_miisAdded(false),
      m_userFocus(0), m_favouriteFocus(0), m_guestFocus(0), m_pad(0), m_choice(0), m_activeTab(0)
{
    initialiseMovie();
    gApp.SetLiveLinkRequired(true);
    m_userControls.m_list.init(0);
    m_favouriteControls.m_list.init(1);
    m_guestControls.m_list.init(2);
    m_userControls.m_title.init(UIString(0xc835d6e5));
    m_favouriteControls.m_title.init(UIString(0x9917e6e4));
    m_guestControls.m_title.init(UIString(0x9dba0db5));
    m_userControls.m_timer.setVisible(false);
    m_favouriteControls.m_timer.setVisible(false);
    m_guestControls.m_timer.setVisible(false);
    for (int i = 0; i < 4; ++i)
    {
        if (gProfile->IsSignedIn(i))
        {
            m_pad = i;
            m_choice = i;
            gConsoleUIController.HidePressStart();
            m_userTabTouch.init(3);
            m_favouriteTabTouch.init(4);
            m_guestTabTouch.init(5);
            gConsoleUIController.TouchBoxRebuild(this);
            break;
        }
    }
}

UIScene_AvatarSelectMenu::~UIScene_AvatarSelectMenu()
{
    s_avatarSelectSignInMask &= ~(1 << m_choice);
}

unsigned int UIScene_AvatarSelectMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    m_userControls.setupControl(this, root, "UserTab");
    m_controls.push_back(&m_userControls);
    m_userControls.m_list.setupControl(this, m_userControls.getFuiRenderNode(), "UserList");
    m_controls.push_back(&m_userControls.m_list);
    m_userControls.m_timer.setupControl(this, m_userControls.getFuiRenderNode(), "UserTimer");
    m_controls.push_back(&m_userControls.m_timer);
    m_userTabTouch.setupControl(this, m_userControls.getFuiRenderNode(), "TouchTabUser");
    m_controls.push_back(&m_userTabTouch);
    m_favouriteControls.setupControl(this, root, "FavouriteTab");
    m_controls.push_back(&m_favouriteControls);
    m_favouriteControls.m_list.setupControl(this, m_favouriteControls.getFuiRenderNode(), "FavouriteList");
    m_controls.push_back(&m_favouriteControls.m_list);
    m_favouriteControls.m_timer.setupControl(this, m_favouriteControls.getFuiRenderNode(), "FavouriteTimer");
    m_controls.push_back(&m_favouriteControls.m_timer);
    m_favouriteTabTouch.setupControl(this, m_favouriteControls.getFuiRenderNode(), "TouchTabFavourite");
    m_controls.push_back(&m_favouriteTabTouch);
    m_guestControls.setupControl(this, root, "GuestTab");
    m_controls.push_back(&m_guestControls);
    m_guestControls.m_list.setupControl(this, m_guestControls.getFuiRenderNode(), "GuestTabList");
    m_controls.push_back(&m_guestControls.m_list);
    m_guestControls.m_timer.setupControl(this, m_guestControls.getFuiRenderNode(), "GuestTimer");
    m_controls.push_back(&m_guestControls.m_timer);
    m_guestTabTouch.setupControl(this, m_guestControls.getFuiRenderNode(), "TouchTabGuest");
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

void UIScene_AvatarSelectMenu::updateTooltips() { hasFocus(m_pad); }
void UIScene_AvatarSelectMenu::updateComponents() {}
void UIScene_AvatarSelectMenu::handleDestroy() {}
void UIScene_AvatarSelectMenu::handleLoseFocus() {}

void UIScene_AvatarSelectMenu::handleGainFocus(bool hadFocus)
{
    updateTooltips();
    if (hadFocus) { gApp.SetLiveLinkRequired(true); m_initialised = false; }
}

void UIScene_AvatarSelectMenu::handleTimerComplete(int) {}

unsigned int UIScene_AvatarSelectMenu::handleKeyDown(int, int action, bool)
{
    if (m_initialised) return 0;
    if (action == EControllerActions_MenuB || action == EControllerActions_MenuCancel)
    {
        gApp.RequestSignInUIWiiUReturnedSelected(NULL, m_choice, m_pad);
        gConsoleUIController.NavigateBack(GetPad(), false, EUIScene_None, EUILayer_Count);
        return 1;
    }
    if (action == EControllerActions_MenuRight || action == EControllerActions_MenuLeft || action == EControllerActions_MenuRightScroll)
    {
        int tab = m_currentTab + ((action == EControllerActions_MenuRightScroll || action == EControllerActions_MenuRight) ? 1 : -1);
        if (tab > 2) tab = 0;
        if (tab < 0) tab = 2;
        m_currentTab = tab;
        SelectTab(tab);
        updateTooltips();
    }
    return 0;
}

void UIScene_AvatarSelectMenu::handleInput(int, int action, bool repeat, bool released, bool pressed, bool& handled)
{
    if (m_initialised || !released)
        return;
    gConsoleUIController.AnimateKeyPress(m_pad, action, repeat, released, pressed);
    switch (action)
    {
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, released, pressed);
        handled = true;
        break;
    default:
        handled = handleKeyDown(m_pad, action, repeat) != 0;
        break;
    }
}

void UIScene_AvatarSelectMenu::handleFocusChange(int focus, int childFocus)
{
    if (focus == 0) m_userFocus = childFocus;
    else if (focus == 1) m_favouriteFocus = childFocus;
    else if (focus == 2) m_guestFocus = childFocus;
    else updateTooltips();
}

void UIScene_AvatarSelectMenu::handleInitFocus(int, int) {}

void UIScene_AvatarSelectMenu::handleTouchInput(unsigned int playerId, int touchX, int touchY,
                                               int touchBox, bool repeat, bool released,
                                               bool pressed)
{
    if (repeat && touchBox > 2 && touchBox < 6 && m_activeTab != touchBox - 3)
    {
        m_activeTab = touchBox - 3;
        updateTooltips();
        SelectTab(m_activeTab);
    }
}

UIControl* UIScene_AvatarSelectMenu::GetMainPanel()
{
    if (m_activeTab == 1) return &m_favouriteControls;
    if (m_activeTab == 2) return &m_guestControls;
    return &m_userControls;
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
    if (hasFocus(m_pad) && !m_miisAdded) { AddMiiItems(); m_miisAdded = true; }
}

void UIScene_AvatarSelectMenu::SelectTab(int tab)
{
    m_activeTab = tab;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    FJ_AvatarSelectMenu* menu = (FJ_AvatarSelectMenu*)m_movie->getRootNode()->getFuiNode();
    if (menu != NULL) menu->SetActiveTab(tab);
    gConsoleUIController.TouchBoxRebuild(this);
}

std::wstring UIScene_AvatarSelectMenu::getMoviePath() { return std::wstring(L"AvatarSelectMenu"); }

void UIScene_AvatarSelectMenu::handlePress(int controlId, int childId)
{
    MiiDataEntry* mii = NULL;
    if (controlId == 0 && childId >= 0 && childId < (int)m_userControls.m_miis.size()) mii = m_userControls.m_miis[childId];
    else if (controlId == 1 && childId >= 0 && childId < (int)m_favouriteControls.m_miis.size()) mii = m_favouriteControls.m_miis[childId];
    else if (controlId == 2 && childId >= 0 && childId < (int)m_guestControls.m_miis.size()) mii = m_guestControls.m_miis[childId];
    gApp.RequestSignInUIWiiUReturnedSelected(mii, m_choice, m_pad);
    navigateBack();
}

int UIScene_AvatarSelectMenu::getSceneType() { return EUIScene_LoadCreateJoinMenu; }
unsigned int UIScene_AvatarSelectMenu::GetPad() { return (unsigned int)m_pad; }
