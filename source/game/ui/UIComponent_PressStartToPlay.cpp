#include "game/ui/UIComponent_PressStartToPlay.hpp"

#include <cwchar>
#include <string>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/FJ_PressStartToPlay.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UIScene_AchievementsMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/CProfile.hpp"

namespace
{
void replaceAll(std::wstring& result, const std::wstring& source, const std::wstring& search,
                const std::wstring& replacement)
{
    result = source;
    std::wstring::size_type position = 0;
    while ((position = result.find(search, position)) != std::wstring::npos)
    {
        result.replace(position, search.length(), replacement);
        ++position;
    }
}

struct AvatarRequest
{
    UIComponent_PressStartToPlay* scene;
    int achievement;
    int player;
};

struct AchievementInfo
{
    int id;
    const wchar_t* icon;
    int description;
    int title;
    int reserved;
};

FJ_PressStartToPlay* GetDocument(fuiRenderNode* root)
{
    return static_cast<FJ_PressStartToPlay*>(root->getFuiNode()->asFJ_Document());
}
}

UIComponent_PressStartToPlay::Toast::Toast(const Toast& other)
    : title(other.title), description(other.description), gamerPic(other.gamerPic),
      achievementIcon(other.achievementIcon)
{
}

UIComponent_PressStartToPlay::UIComponent_PressStartToPlay(int type, void* data, UILayer* layer)
    : UIScene(type, layer), m_showSaveIcon(false), m_showAutosaveTimer(false),
      m_showTrialTimer(false), m_showPressStart(), m_trialTimer(), m_autosaveTimer(),
      m_trialTimerLabel(), m_playerNameLabel(), m_saveIcon(), m_mainPanel(), m_player(0),
      m_achievementToastShowing(false), m_achievement(0), m_holdToShowAchievements(false),
      m_holdActionCancelled(false), m_holdToShowAchievementsActive(false), m_achievementToasts()
{
    initialiseMovie();
    m_trialTimerLabel.init(UIString(L"PressStartToPlay"));
    m_trialTimerLabel.setVisible(false);
    SetText(UIString(L10N::GetString(0x0bcc1ead)), false);
    m_saveIcon.setVisible(false);
    m_playerNameLabel.init(UIString(L"PressStartToPlay"));
    m_mainPanel.setVisible(false);
    m_playerNameLabel.setVisible(false);
}

UIComponent_PressStartToPlay::~UIComponent_PressStartToPlay()
{
}

unsigned int UIComponent_PressStartToPlay::GetAchievementToastShowing(int player)
{
    return (m_achievementToastShowing || m_holdToShowAchievementsActive) && player == m_achievement;
}

unsigned char UIComponent_PressStartToPlay::GetHoldToShowAchievementsActive(bool includeCancelled)
{
    if (includeCancelled)
        m_holdActionCancelled = true;
    return m_holdToShowAchievements;
}

unsigned int UIComponent_PressStartToPlay::handleHold(int player, int action, bool repeat,
                                                       bool released, bool pressed, bool& handled)
{
    if (GetAchievementToastShowing(player) && (action == 0x25 || action == 3) && released && !repeat)
    {
        m_holdToShowAchievements = true;
        m_holdActionCancelled = false;
        addTimer(2, 1000);
        handled = true;
        return 1;
    }
    if (m_holdToShowAchievements && !handled && (action == 0x25 || action == 3) && pressed)
    {
        if (m_holdToShowAchievementsActive)
        {
            m_holdToShowAchievementsActive = false;
            m_holdActionCancelled = true;
        }
        m_holdToShowAchievements = false;
        killTimer(2);
        handled = false;
        return 0;
    }
    return 1;
}

void UIComponent_PressStartToPlay::HideChestRefillAnimation()
{
    if (hasMovie())
        GetDocument(m_movie->getRootNode())->HideChestRefillAnimation();
}

void UIComponent_PressStartToPlay::ResetUIComponents()
{
    m_achievementToastShowing = false;
    if (hasMovie())
        GetDocument(m_movie->getRootNode())->HideChestRefillAnimation();
}

void UIComponent_PressStartToPlay::showTrialTimer(bool show)
{
    m_showTrialTimer = show;
    if (!gConsoleUIController.IsExpectingOrReloadingSkin() && hasMovie())
        m_trialTimerLabel.setVisible(show);
}

void UIComponent_PressStartToPlay::setTrialTimer(const std::wstring& timer)
{
    m_trialTimer = timer;
    if (!gConsoleUIController.IsExpectingOrReloadingSkin() && hasMovie())
        m_trialTimerLabel.setLabel(UIString(timer), false, false);
}

void UIComponent_PressStartToPlay::showSaveIcon(bool show)
{
    m_showSaveIcon = show;
    if (!gConsoleUIController.IsExpectingOrReloadingSkin() && hasMovie() &&
        GameSettings::GetGameSettings(m_playerId, static_cast<eGameSetting>(0x27)))
        m_saveIcon.setVisible(show);
}

void UIComponent_PressStartToPlay::ShowChestRefillAnimation(bool show, int yOffset)
{
    if (hasMovie())
    {
        GetDocument(m_movie->getRootNode())->ShowChestRefillAnimation(show, yOffset);
        addTimer(3, 2500);
    }
}

void UIComponent_PressStartToPlay::showPlayerDisplayName(bool show)
{
    m_playerNameLabel.setVisible(false);
}

void UIComponent_PressStartToPlay::SetText(UIString text, bool multiLine)
{
    if (!hasMovie())
        return;
    FJ_PressStartToPlay* document = GetDocument(m_movie->getRootNode());
    if (multiLine)
        document->SetMultiLineText(text.getString());
    else
        document->SetText(text.getString());
}

void UIComponent_PressStartToPlay::showPressStart(int player, bool show)
{
    if (gConsoleUIController.IsSceneInStack(0xff, static_cast<EUIScene>(0x4b)))
        return;
    m_showPressStart[player] = show;
    if (!gConsoleUIController.IsExpectingOrReloadingSkin() && hasMovie())
    {
        if (!gApp.IsDrcPreventingSplitscreen())
            SetText(UIString(L10N::GetString(0x0bcc1ead)), false);
        else
            SetText(UIString(L10N::GetString(0x9a0686fb)), true);
        m_mainPanel.setVisible(show);
        if (show)
        {
            addTimer(0, 3000);
            GetDocument(m_movie->getRootNode())->ShowController(player, false);
        }
    }
}

void UIComponent_PressStartToPlay::registerTexture(int achievement)
{
    AchievementInfo* info = (AchievementInfo*)(unsigned int)UIScene_AchievementsMenu::GetInfo(achievement);
    wchar_t path[64];
    std::swprintf(path, 64, L"Graphics_Achievements_%ls.png", info->icon);
    std::wstring name(path);
    if (g_pApp->hasArchiveFile(name, false))
    {
        arrayWithLength<unsigned char> data = g_pApp->getArchiveFile(name, false);
        if (data.data != 0 && data.length != 0)
            registerSubstitutionTexture(std::wstring(info->icon), data.data, data.length, true);
    }
}

void UIComponent_PressStartToPlay::ShowAchievementToast(const Toast& toast)
{
    addTimer(1, 4000);
    m_achievementToastShowing = true;
    GetDocument(m_movie->getRootNode())->ShowAchievementToast(toast.title, toast.description,
                                                              toast.gamerPic, toast.achievementIcon);
}

int UIComponent_PressStartToPlay::AvatarReturned(void* context, unsigned char* data, unsigned int size)
{
    AvatarRequest* request = static_cast<AvatarRequest*>(context);
    if (request != 0 && data == 0 && size != 0)
    {
        request->scene->m_player = request->player;
        request->scene->m_achievement = request->achievement;
        request->scene->registerTexture(request->achievement);
        wchar_t suffix[32];
        std::swprintf(suffix, 32, L"%d", request->player);
        std::wstring texture(L"PlayerIcon");
        texture += suffix;
        request->scene->registerSubstitutionTexture(texture, 0, size, true);
        AchievementInfo* info = (AchievementInfo*)(unsigned int)UIScene_AchievementsMenu::GetInfo(request->achievement);
        Toast toast;
        toast.title = L10N::GetString(info->title);
        toast.description = L10N::GetString(0x4523129e);
        replaceAll(toast.description, toast.description, L"__CONTROLLER_VK_X__",
                   L"__CONTROLLER_VK_Y__");
        gApp.FormatHTMLString(request->player, toast.description, -1);
        toast.gamerPic = texture;
        toast.achievementIcon = info->icon;
        if (!request->scene->m_achievementToastShowing)
            request->scene->ShowAchievementToast(toast);
        else
            request->scene->m_achievementToasts.push_back(toast);
    }
    delete request;
    return 0;
}

void UIComponent_PressStartToPlay::SetAchievementUnlocked(int achievement, int player)
{
    AvatarRequest* request = new AvatarRequest;
    request->scene = this;
    request->achievement = achievement;
    request->player = player;
    if (gProfile != 0)
        gProfile->GetProfileAvatar(player, AvatarReturned, request);
    else
        delete request;
}

unsigned char UIComponent_PressStartToPlay::GetHoldActionCancelled(int player)
{
    if (GetAchievementToastShowing(player))
    {
        unsigned char cancelled = m_holdActionCancelled;
        m_holdActionCancelled = false;
        return cancelled;
    }
    return 0;
}

unsigned int UIComponent_PressStartToPlay::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    m_trialTimerLabel.setupControl(this, root, "TrialTimer");
    m_controls.push_back(&m_trialTimerLabel);
    m_saveIcon.setupControl(this, root, "SaveIcon");
    m_controls.push_back(&m_saveIcon);
    m_playerNameLabel.setupControl(this, root, "PlayerName");
    m_controls.push_back(&m_playerNameLabel);
    m_mainPanel.setupControl(this, root, "MainPanel");
    m_controls.push_back(&m_mainPanel);
    return 1;
}

std::wstring UIComponent_PressStartToPlay::getMoviePath()
{
    return std::wstring(L"PressStartToPlay");
}

void UIComponent_PressStartToPlay::handleReload()
{
    bool saveVisible = m_showSaveIcon && GameSettings::GetGameSettings(m_playerId, static_cast<eGameSetting>(0x27));
    m_saveIcon.setVisible(saveVisible);
    m_trialTimerLabel.setVisible(m_showAutosaveTimer);
    m_trialTimerLabel.setLabel(UIString(m_autosaveTimer), false, false);
    m_trialTimerLabel.setVisible(m_showTrialTimer);
    m_trialTimerLabel.setLabel(UIString(m_trialTimer), false, false);
    SetText(UIString(L10N::GetString(0x0bcc1ead)), false);
    bool any = false;
    for (int i = 0; i < 4; ++i)
    {
        if (m_showPressStart[i])
        {
            any = true;
            addTimer(0, 3000);
            GetDocument(m_movie->getRootNode())->ShowController(i, false);
        }
    }
    m_mainPanel.setVisible(any);
}

void UIComponent_PressStartToPlay::HideAchievementToast()
{
    m_achievementToastShowing = false;
    GetDocument(m_movie->getRootNode())->HideAchievementToast();
    if (!m_achievementToasts.empty())
    {
        ShowAchievementToast(m_achievementToasts.front());
        m_achievementToasts.erase(m_achievementToasts.begin());
    }
}

void UIComponent_PressStartToPlay::ShowAchievementMenu()
{
    m_achievementToastShowing = false;
    m_holdToShowAchievements = false;
    if (g_pApp->GetGameStarted() != 0)
        gConsoleUIController.CloseAllPlayersScenes(false);
    gConsoleUIController.NavigateToScene(m_player, static_cast<EUIScene>(0x51),
                                         (void*)(unsigned int)m_achievement,
                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
}

void UIComponent_PressStartToPlay::handleTimerComplete(int timerId)
{
    if (gConsoleUIController.IsExpectingOrReloadingSkin() || !hasMovie())
        return;
    if (timerId == 0)
    {
        m_mainPanel.setVisible(false);
        for (int i = 0; i < 4; ++i)
            m_showPressStart[i] = false;
        gConsoleUIController.ClearPressStart();
    }
    else if (timerId == 3)
    {
        HideChestRefillAnimation();
        killTimer(3);
    }
    else if (timerId == 1)
    {
        killTimer(1);
        HideAchievementToast();
    }
    else if (timerId == 2)
    {
        ShowAchievementMenu();
        killTimer(2);
    }
}

void UIComponent_PressStartToPlay::setAutosaveTimer(const std::wstring& timer)
{
    m_autosaveTimer = timer;
    if (!gConsoleUIController.IsExpectingOrReloadingSkin() && hasMovie())
        m_trialTimerLabel.setLabel(UIString(timer), false, false);
}

void UIComponent_PressStartToPlay::showAutosaveTimer(bool show)
{
    m_showAutosaveTimer = show;
    if (!gConsoleUIController.IsExpectingOrReloadingSkin() && hasMovie())
        m_trialTimerLabel.setVisible(show);
}

void UIComponent_PressStartToPlay::ShowBetaWatermark()
{
    if (hasMovie())
        GetDocument(m_movie->getRootNode())->ShowBetaWatermark();
}

void UIComponent_PressStartToPlay::tick()
{
    UIScene::tick();
    Minecraft* minecraft = Minecraft::GetInstance();
    if (!gConsoleUIController.IsExpectingOrReloadingSkin() && hasMovie())
    {
        for (int i = 0; i < 4; ++i)
            if (m_showPressStart[i] && minecraft->mConnectionFailed[i] != 0)
                showPressStart(i, false);
    }
}

int UIComponent_PressStartToPlay::getSceneType()
{
    return 0x47;
}

bool UIComponent_PressStartToPlay::stealsFocus()
{
    return false;
}

bool UIComponent_PressStartToPlay::hasFocus(int sceneType)
{
    return false;
}

bool UIComponent_PressStartToPlay::hidesLowerScenes()
{
    return false;
}
