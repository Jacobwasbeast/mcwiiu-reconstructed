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
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/MiiData.hpp"

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
    m_userControls.m_list.setupControl(this, m_userControls.getFuiRenderNode(), "UserList");
    m_userControls.m_timer.setupControl(this, m_userControls.getFuiRenderNode(), "UserTimer");
    m_userTabTouch.setupControl(this, m_userControls.getFuiRenderNode(), "TouchTabUser");
    m_favouriteControls.setupControl(this, root, "FavouriteTab");
    m_favouriteControls.m_list.setupControl(this, m_favouriteControls.getFuiRenderNode(), "FavouriteList");
    m_favouriteControls.m_timer.setupControl(this, m_favouriteControls.getFuiRenderNode(), "FavouriteTimer");
    m_favouriteTabTouch.setupControl(this, m_favouriteControls.getFuiRenderNode(), "TouchTabFavourite");
    m_guestControls.setupControl(this, root, "GuestTab");
    m_guestControls.m_list.setupControl(this, m_guestControls.getFuiRenderNode(), "GuestTabList");
    m_guestControls.m_timer.setupControl(this, m_guestControls.getFuiRenderNode(), "GuestTimer");
    m_guestTabTouch.setupControl(this, m_guestControls.getFuiRenderNode(), "TouchTabGuest");
    m_userControls.m_title.setupControl(this, root, "UserTabTitle");
    m_favouriteControls.m_title.setupControl(this, root, "FavouriteTabTitle");
    m_guestControls.m_title.setupControl(this, root, "GuestTabTitle");
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
    if (action == 1 || action == 0x15)
    {
        gApp.RequestSignInUIWiiUReturnedSelected(NULL, m_choice, m_pad);
        gConsoleUIController.NavigateBack(GetPad(), false, (EUIScene)0x53, (EUILayer)8);
        return 1;
    }
    if (action == 6 || action == 7 || action == 10)
    {
        int tab = m_currentTab + ((action == 10 || action == 6) ? 1 : -1);
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
    if (!m_initialised && released)
    {
        gConsoleUIController.AnimateKeyPress(m_pad, action, repeat, released, pressed);
        if (action < 4 || (action > 5 && action < 8) || (action > 9 && action != 0x14))
            handled = handleKeyDown(m_pad, action, repeat) != 0;
        else { sendInputToMovie(action, repeat, released, pressed); handled = true; }
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
    if (gProfile->QuerySigninStatus())
    {
        gApp.RequestSignInUIWiiUReturnedSelected(NULL, m_choice, m_pad);
        gConsoleUIController.NavigateBack(GetPad(), false, (EUIScene)0x53, (EUILayer)8);
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

void UIScene_AvatarSelectMenu::handlePress(int first, int second)
{
    MiiDataEntry* mii = NULL;
    if (first == 0 && second >= 0 && second < (int)m_userControls.m_miis.size()) mii = m_userControls.m_miis[second];
    else if (first == 1 && second >= 0 && second < (int)m_favouriteControls.m_miis.size()) mii = m_favouriteControls.m_miis[second];
    else if (first == 2 && second >= 0 && second < (int)m_guestControls.m_miis.size()) mii = m_guestControls.m_miis[second];
    gApp.RequestSignInUIWiiUReturnedSelected(mii, m_choice, m_pad);
    navigateBack();
}

int UIScene_AvatarSelectMenu::getSceneType() { return 0x1b; }
unsigned int UIScene_AvatarSelectMenu::GetPad() { return (unsigned int)m_pad; }
