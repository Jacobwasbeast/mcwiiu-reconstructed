#include "game/ui/UIScene_FullscreenProgress.hpp"

#include <cwchar>
#include <vector>

#include "game/client/render/ProgressRenderer.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/FJ_FullScreenProgress.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/Tooltips.hpp"
#include "game/ui/UIGroup.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

namespace
{
struct FullscreenProgressData
{
    int (*threadEntry)(void*);
    void* threadData;
    int playerId;
    int sceneType;
    void (*destroyCallback)(int);
    void (*cancelCallback)(int);
    int destroyData;
    int cancelData;
    bool threadReady;
    EMiniGameId miniGameId;
};
}

UIScene_FullscreenProgress::UIScene_FullscreenProgress(int type, void* data, UILayer* layer)
    : UIScene(type, layer), m_thread(NULL), m_data(NULL), m_destroyCallback(NULL),
      m_cancelCallback(NULL), m_destroyCallbackData(0), m_cancelCallbackData(0),
      m_miniGameId(static_cast<EMiniGameId>(0)), m_threadReady(false), m_miniGame(false),
      m_titleText(), m_statusText(), m_progress(0), m_confirmationVisible(false),
      m_wasCancelled(false), m_confirmationPressed(false)
{
    initialiseMovie();
    m_miniGame = gApp.GetGameHostOption(static_cast<eGameHostOption>(0x1f)) != 0;
    SetLoadingScreenType(m_miniGame ? 1 : 0);
    if (m_miniGame)
    {
        setMiniGameLogoTexture();
    }
    layer->addComponent(type, static_cast<EUIScene>(0x42), NULL);
    layer->addComponent(type, static_cast<EUIScene>(0x43), NULL);
    layer->showComponent(type, static_cast<EUIScene>(0x43), true);
    layer->showComponent(type, static_cast<EUIScene>(0x48), false);
    m_timerTitleControl.setVisible(false);
    m_titleText = L"FullscreenProgress";
    m_statusText = L"FullscreenProgress";
    m_confirmationButton.init(UIString(L10N::GetString(0xb9a1bf4d)), 0);
    ShowConfirmationButton(false);
    m_wasCancelled = false;

    FullscreenProgressData* progressData = static_cast<FullscreenProgressData*>(data);
    if (progressData != NULL)
    {
        m_data = progressData->threadData;
        m_destroyCallback = progressData->destroyCallback;
        m_cancelCallback = progressData->cancelCallback;
        m_destroyCallbackData = progressData->destroyData;
        m_cancelCallbackData = progressData->cancelData;
        m_miniGameId = progressData->miniGameId;
        m_threadReady = progressData->threadReady;
        m_miniGame = progressData->miniGameId != static_cast<EMiniGameId>(0);
        if (progressData->threadEntry != NULL)
        {
            m_thread = new C4JThreadImpl(progressData->threadEntry, progressData->threadData,
                                          "FullscreenProgress", 0);
            m_thread->Run();
        }
    }
    m_confirmationPressed = false;
    gConsoleUIController.TouchBoxRebuild(this);
    if (m_data != NULL && *static_cast<int*>(m_data) == 3)
    {
        layer->closeAllScenes();
    }
}

UIScene_FullscreenProgress::~UIScene_FullscreenProgress()
{
    if (m_layer != NULL)
    {
        m_layer->removeComponent(static_cast<EUIScene>(0x42));
        m_layer->removeComponent(static_cast<EUIScene>(0x43));
    }
    if (m_thread != NULL)
    {
        if (m_thread->isRunning() != 0)
        {
            m_thread->WaitForCompletion(1);
        }
        delete m_thread;
        m_thread = NULL;
    }
}

void UIScene_FullscreenProgress::SetLoadingScreenType(int type)
{
    if (m_movie == NULL || m_movie->getRootNode() == NULL) return;
    FJ_FullScreenProgress* document = static_cast<FJ_FullScreenProgress*>(
        m_movie->getRootNode()->getFuiNode()->asFJ_Document());
    if (document != NULL) document->SetLoadingScreenType(type);
}

void UIScene_FullscreenProgress::setMiniGameLogoTexture()
{
    const MiniGameDef* game = Minecraft::GetInstance()->GetMiniGame();
    std::wstring icon;
    if (game == NULL || game->GetId() == 15 || game->GetId() == 0)
    {
        game = Minecraft::GetInstance()->GetFirstMiniGame();
    }
    if (game != NULL)
    {
        icon = game->getTextureName();
    }
    if (icon.empty() || !gApp.hasArchiveFile(icon, false))
    {
        icon = L"</p>";
    }
    else
    {
        arrayWithLength<unsigned char> file = gApp.getArchiveFile(icon, false);
        if (file.data != NULL && file.length != 0)
        {
            registerSubstitutionTexture(icon, file.data, file.length, true);
        }
    }
    m_logoControl.setTextureName(icon, false);
}

void UIScene_FullscreenProgress::ShowConfirmationButton(bool show)
{
    if (m_movie == NULL || m_movie->getRootNode() == NULL) return;
    FJ_FullScreenProgress* document = static_cast<FJ_FullScreenProgress*>(
        m_movie->getRootNode()->getFuiNode()->asFJ_Document());
    if (document != NULL) document->ShowConfirmationButton(show);
}

void UIScene_FullscreenProgress::SetTipText(bool initial)
{
    EMiniGameId id = m_miniGameId;
    if (id == static_cast<EMiniGameId>(15))
    {
        id = static_cast<EMiniGameId>(gApp.GetFirstMiniGameType());
    }
    unsigned int tip = gApp.GetNextTip(true, id);
    std::wstring text(L10N::GetString(static_cast<int>(tip)));
    gApp.FormatHTMLString(m_resolution, text, -1);
    unsigned int colour = gApp.GetHTMLColour(static_cast<eMinecraftColour>(0x1b5));
    wchar_t formatted[64];
    swprintf(formatted, 64, L"<font color=\"#%08x\"><p align=center", colour);
    std::wstring result(formatted);
    result += text;
    result += L"</p>";
    if (initial)
    {
        m_tipLabel.init(UIString(result));
    }
    else
    {
        m_tipLabel.setLabel(UIString(result), false, false);
    }
}

unsigned int UIScene_FullscreenProgress::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie == NULL ? NULL : m_movie->getRootNode();
    std::vector<UIControl*>* controls = &m_controls;
    m_progressControl.setupControl(this, root, "ProgressBar");
    controls->push_back(&m_progressControl);
    m_titleLabel.setupControl(this, root, "Title");
    controls->push_back(&m_titleLabel);
    m_tipLabel.setupControl(this, root, "Tip");
    controls->push_back(&m_tipLabel);
    m_timerTitleControl.setupControl(this, root, "Timer");
    controls->push_back(&m_timerTitleControl);
    m_logoControl.setupControl(this, m_timerTitleControl.getFuiRenderNode(), "MG_Logo");
    controls->push_back(&m_logoControl);
    m_confirmationButton.setupControl(this, root, "Confirm");
    controls->push_back(&m_confirmationButton);
    m_timerControl.setupControl(this, root, "Timer");
    controls->push_back(&m_timerControl);
    return 1;
}

void UIScene_FullscreenProgress::handleReload()
{
    m_tipLabel.setVisible(m_data != NULL && static_cast<unsigned char*>(m_data)[3] != 0);
    m_miniGame = gApp.GetGameHostOption(static_cast<eGameHostOption>(0x1f)) != 0;
    SetLoadingScreenType(m_miniGame ? 1 : 0);
    if (m_miniGame)
    {
        setMiniGameLogoTexture();
    }
    ShowConfirmationButton(false);
    SetTipText(false);
}

void UIScene_FullscreenProgress::updateTooltips()
{
    Tooltips::ForceAllPlayerTooltipReset();
    int second = m_miniGame ? static_cast<int>(0x8d6323ed) : -1;
    int third = m_miniGame ? -1 : static_cast<int>(m_miniGameId);
    gConsoleUIController.SetTooltips(m_resolution, second, third, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1,
                                     false, m_layer != NULL && m_layer->IsFullscreenGroup());
}

void UIScene_FullscreenProgress::handleDestroy()
{
    if (m_thread != NULL && m_thread->GetExitCode() == 0x103 && m_destroyCallback != NULL && !m_wasCancelled)
    {
        m_wasCancelled = true;
        m_destroyCallback(m_destroyCallbackData);
    }
}

void UIScene_FullscreenProgress::tick()
{
    UIScene::tick();
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != NULL && minecraft->mProgressRenderer != NULL)
    {
        int percent = minecraft->mProgressRenderer->getCurrentPercent();
        if (percent < 0) percent = 0;
        if (percent != m_progress)
        {
            m_progress = percent;
            m_progressControl.setProgress(percent);
        }
        std::wstring title = minecraft->mProgressRenderer->getCurrentTitleString();
        if (title != m_titleText)
        {
            m_titleLabel.setLabel(UIString(title), false, false);
            m_titleText = title;
        }
        std::wstring status = minecraft->mProgressRenderer->getCurrentStatusString();
        if (status != m_statusText)
        {
            m_progressControl.setLabel(UIString(status), false, false);
            m_statusText = status;
        }
    }
    if (m_thread == NULL) return;
    int state = m_thread->GetExitCode();
    if (state == 0 && m_data != NULL && *static_cast<int*>(m_data) != 0)
    {
        m_miniGame = true;
        ShowConfirmationButton(true);
        if (m_confirmationButton.getVisible()) gConsoleUIController.TouchBoxRebuild(this);
    }
    if (state == 0 && m_cancelCallback != NULL && !m_wasCancelled && m_data != NULL)
    {
        m_wasCancelled = true;
        m_cancelCallback(m_cancelCallbackData);
    }
}

std::wstring UIScene_FullscreenProgress::getMoviePath()
{
    return L"FullscreenProgress";
}

void UIScene_FullscreenProgress::handleInput(int playerId, int action, bool repeat, bool released,
                                             bool pressed, bool& handled)
{
    if (action == 20 && pressed)
    {
        sendInputToMovie(action, repeat, released, pressed);
        return;
    }
    if ((action == 1 || action == 21) && pressed && m_cancelCallback != NULL && !m_wasCancelled)
    {
        m_wasCancelled = true;
        m_cancelCallback(m_cancelCallbackData);
    }
}

void UIScene_FullscreenProgress::handlePress(int first, int second)
{
    if (!m_miniGame || second != 0) return;
    gConsoleUIController.AnimateKeyPress(m_resolution, 0, false, false, true);
    if (m_cancelCallback != NULL) m_cancelCallback(m_cancelCallbackData);
    int mode = m_data == NULL ? 0 : static_cast<int*>(m_data)[1];
    if (mode == 1)
    {
        gConsoleUIController.UpdatePlayerBasePositions();
        navigateBack();
    }
    else if (mode == 2)
    {
        gConsoleUIController.CloseUIScenes(m_resolution, false);
        gConsoleUIController.UpdatePlayerBasePositions();
    }
    else if (mode == 3)
    {
        gConsoleUIController.CloseAllPlayersScenes(false);
        gConsoleUIController.UpdatePlayerBasePositions();
    }
    else if (mode == 4)
    {
        gConsoleUIController.NavigateToHomeMenu(false);
        gConsoleUIController.UpdatePlayerBasePositions();
    }
    else if (mode == 6 && m_layer != NULL)
    {
        gConsoleUIController.UpdatePlayerBasePositions();
        m_layer->NavigateBack(m_resolution, static_cast<EUIScene>(m_data == NULL ? 0 : static_cast<int*>(m_data)[3]));
        gConsoleUIController.FixupMenuDisplayedAfterNavigateBack();
    }
}

void UIScene_FullscreenProgress::handleTimerComplete(int timerId)
{
    if (timerId == 1) SetTipText(false);
}

void UIScene_FullscreenProgress::SetWasCancelled(bool cancelled)
{
    m_wasCancelled = cancelled;
}

bool UIScene_FullscreenProgress::isReadyToDelete()
{
    return !m_threadReady || m_thread == NULL || m_thread->isRunning() == 0;
}

int UIScene_FullscreenProgress::getSceneType()
{
    return 4;
}

unsigned int UIScene_FullscreenProgress::getDefaultGtcButtons()
{
    return 0;
}
