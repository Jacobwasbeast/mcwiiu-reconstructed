#include "game/ui/UIScene_FullscreenProgress.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>
#include <vector>

#include "game/client/render/ProgressRenderer.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/EMiniGameId.hpp"
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
#include "vendor/_LoadingInputParams.hpp"
#include "game/core/GameSettings.hpp"

UIScene_FullscreenProgress::UIScene_FullscreenProgress(int type, void* data, UILayer* layer)
    : UIScene(type, layer), m_thread(NULL), m_threadStarted(false), m_completionData(NULL),
      m_confirmationVisible(false), m_cancelFunc(NULL), m_completeFunc(NULL),
      m_cancelFuncParam(NULL), m_completeFuncParam(NULL), m_waitForThreadToDelete(false),
      m_miniGameId(eMiniGameId_Lobby), m_miniGame(false), m_titleText(),
      m_statusText(), m_progress(0), m_cancelTooltip(0), m_wasCancelled(false),
      m_confirmationButtonHidden(false)
{
    initialiseMovie();
    m_miniGame = gApp.GetGameHostOption(eGameHostOption_MiniGame) != 0;
    SetLoadingScreenType(m_miniGame ? 1 : 0);
    if (m_miniGame)
    {
        setMiniGameLogoTexture();
    }
    layer->addComponent(type, EUIScene_Panorama, NULL);
    layer->addComponent(type, EUIScene_Logo, NULL);
    layer->showComponent(type, EUIScene_Logo, true);
    layer->showComponent(type, EUIScene_MenuBackground, false);
    m_timerControl.setVisible(false);
    m_titleText = L"";
    m_statusText = L"";
    m_progress = 0;
    m_confirmationButton.init(UIString(L10N::GetString(IDS_CONFIRM_OK)), 0);
    ShowConfirmationButton(false);
    m_wasCancelled = false;

    _LoadingInputParams* params = static_cast<_LoadingInputParams*>(data);
    m_completionData = static_cast<FullscreenProgressState*>(params->m_completionData);
    m_cancelTooltip = params->m_cancelText;
    m_cancelFunc = params->m_cancelFunc;
    m_completeFunc = params->m_completeFunc;
    m_cancelFuncParam = params->m_cancelFuncParam;
    m_completeFuncParam = params->m_completeFuncParam;
    m_miniGameId = params->m_miniGameId;
    m_waitForThreadToDelete = params->m_waitForThreadToDelete;
    m_playerId = m_completionData->playerId;
    Minecraft* minecraft = Minecraft::GetInstance();
    minecraft->mProgressRenderer->progressStart(-1);
    minecraft->mProgressRenderer->progressStage(-1);
    m_progressControl.init(UIString(L""), 0, 0, 100, 0);
    SetTipText(true);
    addTimer(1, 7000);
    m_titleLabel.init(UIString(L""));
    m_tipLabel.setVisible(m_completionData->showTip);
    m_thread = new C4JThreadImpl(params->m_threadProc, params->m_threadParam, "FullscreenProgress", 0);
    m_thread->SetDeleteOnExit(false);
    m_thread->Run();
    m_threadStarted = true;
    delete params;
    m_confirmationButtonHidden = false;
    gConsoleUIController.TouchBoxRebuild(this);
    if (m_completionData->closeMode == 3)
    {
        layer->closeAllScenes();
    }
}

UIScene_FullscreenProgress::~UIScene_FullscreenProgress()
{
    if (m_layer != NULL)
    {
        m_layer->removeComponent(EUIScene_Panorama);
        m_layer->removeComponent(EUIScene_Logo);
    }
    if (m_thread != NULL)
    {
        if (m_thread->isRunning() == 0)
        {
            delete m_thread;
        }
        else
        {
            m_thread->SetDeleteOnExit(true);
        }
        m_thread = NULL;
    }
    delete m_completionData;
    m_completionData = NULL;
}

void UIScene_FullscreenProgress::SetLoadingScreenType(int type)
{
    FJ_FuiNode* root = m_movie->getRootNode()->getFuiNode();
    FJ_FullScreenProgress* document = static_cast<FJ_FullScreenProgress*>(root->asFJ_Document());
    document->SetLoadingScreenType(type);
}

void UIScene_FullscreenProgress::setMiniGameLogoTexture()
{
    const MiniGameDef* game = Minecraft::GetInstance()->GetMiniGame();
    std::wstring icon;
    if (game->GetId() == 15 || game->GetId() == 0)
    {
        game = Minecraft::GetInstance()->GetFirstMiniGame();
    }
    icon = game->getIcon();
    if (!gApp.hasArchiveFile(icon, false))
    {
        icon = L"";
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
    FJ_FuiNode* root = m_movie->getRootNode()->getFuiNode();
    FJ_FullScreenProgress* document = static_cast<FJ_FullScreenProgress*>(root->asFJ_Document());
    document->ShowConfirmationButton(show);
}

void UIScene_FullscreenProgress::SetTipText(bool initial)
{
    unsigned int tip;
    if (m_miniGameId == eMiniGameId_Any)
    {
        tip = gApp.GetNextTip(false, eMiniGameId_Lobby);
    }
    else
    {
        EMiniGameId id = m_miniGameId;
        if (id == eMiniGameId_Lobby) id = gApp.GetFirstMiniGameType();
        tip = gApp.GetNextTip(true, id);
    }
    std::wstring text = gApp.FormatHTMLString(
        m_playerId, std::wstring(L10N::GetString(static_cast<int>(tip))), -1);
    unsigned int colour = gApp.GetHTMLColour(eMinecraftColour_HTMLColor_White);
    wchar_t formatted[64];
    swprintf(formatted, 64, L"<font color=\"#%08x\"><p align=center>", colour);
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
    m_timerTitleControl.setupControl(this, root, "GMIcon");
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
    m_tipLabel.setVisible(m_completionData != NULL && m_completionData->showTip);
    m_miniGame = gApp.GetGameHostOption(eGameHostOption_MiniGame) != 0;
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
    int second = m_confirmationVisible ? static_cast<int>(IDS_TOOLTIPS_SELECT) : -1;
    int third = m_confirmationVisible ? -1 : m_cancelTooltip;
    gConsoleUIController.SetTooltips(m_playerId, second, third, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1,
                                     false, m_layer != NULL && m_layer->IsFullscreenGroup());
}

void UIScene_FullscreenProgress::handleDestroy()
{
    if (m_thread != NULL && m_thread->GetExitCode() == 0x103 && m_cancelFunc != NULL && !m_wasCancelled)
    {
        m_wasCancelled = true;
        m_cancelFunc(m_cancelFuncParam);
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
    FullscreenProgressState* progressState = m_completionData;
    int threadState = m_thread->GetExitCode();
    if (threadState != 0x103)
    {
        if (threadState == 0)
        {
            bool finish = true;
            if (!m_wasCancelled && progressState != NULL && progressState->showConfirmation)
            {
                m_confirmationVisible = true;
                if (m_confirmationButtonHidden)
                {
                    gConsoleUIController.TouchBoxRebuild(this);
                }
                m_confirmationButtonHidden = !m_confirmationButton.getVisible();
                ShowConfirmationButton(true);
                updateTooltips();
                finish = false;
            }
            else if (m_wasCancelled)
            {
                m_confirmationVisible = true;
            }

            if (finish)
            {
                if (m_completeFunc != NULL)
                {
                    m_completeFunc(m_completeFuncParam);
                }
                int closeMode = progressState == NULL ? 0 : progressState->closeMode;
                if (closeMode == 1)
                {
                    gConsoleUIController.UpdatePlayerBasePositions();
                    navigateBack();
                }
                else if (closeMode == 2)
                {
                    gConsoleUIController.CloseUIScenes(progressState->playerId, false);
                    gConsoleUIController.UpdatePlayerBasePositions();
                }
                else if (closeMode == 3)
                {
                    gConsoleUIController.CloseAllPlayersScenes(false);
                    gConsoleUIController.UpdatePlayerBasePositions();
                }
                else if (closeMode == 4)
                {
                    gConsoleUIController.NavigateToHomeMenu(false);
                    gConsoleUIController.UpdatePlayerBasePositions();
                }
                else if (closeMode == 5 || closeMode == 6)
                {
                    if (closeMode == 5)
                    {
                        gConsoleUIController.SetIgnoreAutosaveMenuDisplayed(m_playerId, false);
                    }
                    gConsoleUIController.UpdatePlayerBasePositions();
                    if (m_layer != NULL)
                    {
                        m_layer->NavigateBack(m_playerId, progressState->backScene);
                    }
                    gConsoleUIController.FixupMenuDisplayedAfterNavigateBack();
                }
            }
        }
        else if (threadState != 0x4c7)
        {
            CGameNetworkManager::sInstance.WiiU_ShowErrorViewer();
            if (gApp.GetXuiAction(m_playerId) != eXuiAction_ConnectionLostWait)
            {
                int parameter = static_cast<int>(IDS_CONFIRM_OK);
                int title = CGameNetworkManager::sInstance.CorrectErrorIDS(IDS_CONNECTION_FAILED);
                int message = CGameNetworkManager::sInstance.CorrectErrorIDS(IDS_CONNECTION_LOST_SERVER);
                gConsoleUIController.RequestErrorMessage(title, message, &parameter, 1, 0xff,
                                                         NULL, NULL, NULL, true);
                gConsoleUIController.NavigateToHomeMenu(false);
            }
            gConsoleUIController.UpdatePlayerBasePositions();
        }
    }

    if (!m_miniGame && m_movie != NULL)
    {
        EMiniGameId miniGameId = Minecraft::GetMiniGameId();
        if (miniGameId != eMiniGameId_Any && miniGameId != eMiniGameId_Lobby)
        {
            SetLoadingScreenType(1);
            setMiniGameLogoTexture();
            m_miniGame = true;
            m_miniGameId = miniGameId;
            gApp.SetFirstMiniGameType(miniGameId);
            handleTimerComplete(1);
            killTimer(1);
            addTimer(1, 7000);
        }
    }
}

std::wstring UIScene_FullscreenProgress::getMoviePath()
{
    return L"FullscreenProgress";
}

void UIScene_FullscreenProgress::handleInput(int playerId, int action, bool repeat, bool pressed,
                                             bool released, bool& handled)
{
    if (action == EControllerActions_MenuOk && pressed)
    {
        sendInputToMovie(action, repeat, pressed, released);
        return;
    }
    if ((action == EControllerActions_MenuB || action == EControllerActions_MenuCancel) && pressed && m_cancelFunc != NULL && !m_wasCancelled)
    {
        m_wasCancelled = true;
        m_cancelFunc(m_cancelFuncParam);
    }
}

void UIScene_FullscreenProgress::handlePress(int controlId, int childId)
{
    if (!m_confirmationVisible || controlId != 0) return;
    gConsoleUIController.AnimateKeyPress(m_playerId, 0, false, true, false);
    if (m_completeFunc != NULL) m_completeFunc(m_completeFuncParam);
    FullscreenProgressState* state = m_completionData;
    int mode = state == NULL ? 0 : state->closeMode;
    if (mode == 1)
    {
        gConsoleUIController.UpdatePlayerBasePositions();
        navigateBack();
    }
    else if (mode == 2)
    {
        gConsoleUIController.CloseUIScenes(state->playerId, false);
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
    else if (mode == 6)
    {
        gConsoleUIController.UpdatePlayerBasePositions();
        if (m_layer != NULL)
        {
            m_layer->NavigateBack(m_playerId, state->backScene);
        }
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
    return !m_waitForThreadToDelete || m_thread == NULL || m_thread->isRunning() == 0;
}

int UIScene_FullscreenProgress::getSceneType()
{
    return EUIScene_FullscreenProgress;
}

unsigned int UIScene_FullscreenProgress::getDefaultGtcButtons()
{
    return 0;
}
