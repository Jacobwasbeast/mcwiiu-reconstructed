#include "game/ui/UIScene_ConnectingProgress.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/System.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FullScreenProgress.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

namespace
{
struct ConnectingProgressData
{
    int title;
    bool miniGame;
    bool timerEnabled;
    int timerDuration;
    void (*callback)(int);
    int callbackData;
    int miniGameId;
};
}

UIScene_ConnectingProgress::UIScene_ConnectingProgress(int type, void* data, UILayer* layer)
    : UIScene(type, layer), m_timerEnabled(false), m_timerDuration(0), m_miniGame(false),
      m_removePlayer(false), m_confirmationVisible(false), m_cancelled(false),
      m_removePlayerCallback(NULL), m_removePlayerCallbackData(0), m_miniGameId(0),
      m_miniGameLoaded(false)
{
    initialiseMovie();
    m_confirmationLabel.init(UIString(L""));
    m_confirmationLabel.setVisible(false);

    ConnectingProgressData* progressData = static_cast<ConnectingProgressData*>(data);
    if (progressData != NULL)
    {
        m_miniGame = progressData->miniGame;
        m_timerEnabled = progressData->timerEnabled;
        m_timerDuration = progressData->timerDuration;
        m_removePlayerCallback = progressData->callback;
        m_removePlayerCallbackData = progressData->callbackData;
        m_miniGameId = progressData->miniGameId;
        m_playerId = progressData->miniGameId;

        const wchar_t* title = progressData->title == -1
                                   ? L""
                                   : L10N::GetString(progressData->title);
        m_titleLabel.init(UIString(title));
        delete progressData;
    }
    else
    {
        m_titleLabel.init(UIString(L""));
    }

    SetLoadingScreenType(m_miniGame ? 1 : 0);
    if (m_miniGame)
    {
        setMiniGameLogoTexture();
        m_miniGameLoaded = true;
    }

    layer->addComponent(type, static_cast<EUIScene>(0x42), NULL);
    layer->addComponent(type, static_cast<EUIScene>(0x43), NULL);
    m_progressControl.init(UIString(L""), 0, 0, 100, 0);
    m_progressControl.showBar(false);
    m_progressControl.setVisible(false);
    m_confirmationButton.init(UIString(L10N::GetString(0xb9a1bf4d)), 0);
    ShowConfirmationButton(false);
}

UIScene_ConnectingProgress::~UIScene_ConnectingProgress()
{
    if (m_layer != NULL)
    {
        m_layer->removeComponent(static_cast<EUIScene>(0x42));
        m_layer->removeComponent(static_cast<EUIScene>(0x43));
    }

    if (m_confirmationVisible)
    {
        tryRemovePlayer();
        if (m_removePlayer)
        {
            removeLocalPlayer();
        }
    }
}

void UIScene_ConnectingProgress::SetLoadingScreenType(int type)
{
    if (m_movie == NULL || m_movie->getRootNode() == NULL)
    {
        return;
    }

    FJ_FuiNode* node = m_movie->getRootNode()->getFuiNode();
    if (node == NULL)
    {
        return;
    }

    FJ_FullScreenProgress* document =
        static_cast<FJ_FullScreenProgress*>(node->asFJ_Document());
    if (document != NULL)
    {
        document->SetLoadingScreenType(type);
    }
}

void UIScene_ConnectingProgress::setMiniGameLogoTexture()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    const MiniGameDef* miniGame = minecraft->GetMiniGame();
    if (miniGame == NULL || miniGame->GetId() == 15 || miniGame->GetId() == 0)
    {
        miniGame = minecraft->GetFirstMiniGame();
    }

    std::wstring icon;
    if (miniGame != NULL)
    {
        icon = L"Graphics/" + miniGame->getTextureName() + L".png";
    }

    if (icon.empty() || !g_pApp->hasArchiveFile(icon, false))
    {
        icon = L"";
    }
    else
    {
        arrayWithLength<unsigned char> file = g_pApp->getArchiveFile(icon, false);
        if (file.data != NULL && file.length != 0)
        {
            registerSubstitutionTexture(icon, file.data, file.length, true);
        }
    }

    m_logoControl.setTextureName(icon, false);
}

void UIScene_ConnectingProgress::ShowConfirmationButton(bool show)
{
    if (m_movie == NULL || m_movie->getRootNode() == NULL)
    {
        return;
    }

    FJ_FuiNode* node = m_movie->getRootNode()->getFuiNode();
    if (node == NULL)
    {
        return;
    }

    FJ_FullScreenProgress* document =
        static_cast<FJ_FullScreenProgress*>(node->asFJ_Document());
    if (document != NULL)
    {
        document->ShowConfirmationButton(show);
    }
}

unsigned int UIScene_ConnectingProgress::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie == NULL ? NULL : m_movie->getRootNode();

    m_progressControl.setupControl(this, root, "ProgressBar");
    m_controls.push_back(&m_progressControl);
    m_titleLabel.setupControl(this, root, "Title");
    m_controls.push_back(&m_titleLabel);
    m_confirmationLabel.setupControl(this, root, "Confirm");
    m_controls.push_back(&m_confirmationLabel);
    m_timerTitleControl.setupControl(this, root, "GMIcon");
    m_controls.push_back(&m_timerTitleControl);
    m_logoControl.setupControl(this, m_timerTitleControl.getFuiRenderNode(), "MG_Logo");
    m_controls.push_back(&m_logoControl);
    m_confirmationButton.setupControl(this, root, "Confirm");
    m_controls.push_back(&m_confirmationButton);
    m_timerControl.setupControl(this, root, "Timer");
    m_controls.push_back(&m_timerControl);

    return 1;
}

void UIScene_ConnectingProgress::tryRemovePlayer()
{
    m_confirmationVisible = false;
    if (m_removePlayerCallback != NULL)
    {
        m_removePlayer = true;
        return;
    }

    gConsoleUIController.NavigateToHomeMenu(false);
}

void UIScene_ConnectingProgress::removeLocalPlayer()
{
    m_removePlayer = false;
    Minecraft::GetInstance()->removeLocalPlayerIdx(m_playerId);
}

void UIScene_ConnectingProgress::tick()
{
    UIScene::tick();

    if (m_removePlayer)
    {
        removeLocalPlayer();
    }

    Minecraft* minecraft = Minecraft::GetInstance();
    const MiniGameDef* miniGame = minecraft->GetMiniGame();
    if (!m_miniGameLoaded && m_movie != NULL && miniGame != NULL && miniGame->GetId() != 15)
    {
        SetLoadingScreenType(1);
        setMiniGameLogoTexture();
        m_miniGameLoaded = true;
        m_miniGameId = miniGame->GetId();
        g_pApp->SetFirstMiniGameType(static_cast<EMiniGameId>(m_miniGameId));
    }
}

void UIScene_ConnectingProgress::handleReload()
{
    m_miniGame =
        g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x1f)) != 0;
    m_miniGameLoaded = m_miniGame;
    SetLoadingScreenType(m_miniGame ? 1 : 0);
    if (m_miniGame)
    {
        setMiniGameLogoTexture();
    }
}

void UIScene_ConnectingProgress::updateTooltips()
{
    int tooltip = m_miniGame ? static_cast<int>(0x23ec13a6) : -1;
    gConsoleUIController.SetTooltips(m_playerId, -1, tooltip, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_ConnectingProgress::handleGainFocus(bool hadFocus)
{
    if (!hadFocus && m_timerEnabled)
    {
        addTimer(0, m_timerDuration);
    }
}

void UIScene_ConnectingProgress::handleLoseFocus()
{
    const int timerId = 0;
    m_timers[timerId];
    System::processTimeInMilliSecs();
    m_timers[timerId];
    killTimer(timerId);
}

void UIScene_ConnectingProgress::handleTimerComplete(int timerId)
{
    if (timerId != 0)
    {
        return;
    }

    ShowConfirmationButton(true);
    m_confirmationVisible = true;
    m_confirmationLabel.setLabel(UIString(L10N::GetString(0x4bb9d6ae)), false, false);
    m_progressControl.setLabel(UIString(L10N::GetString(0x665e892b)), false, false);
    m_progressControl.setVisible(true);
    m_timerControl.setVisible(false);
}

std::wstring UIScene_ConnectingProgress::getMoviePath()
{
    if (g_pApp->GetLocalPlayerCount() < 2 ||
        (m_layer != NULL && m_layer->IsFullscreenGroup()))
    {
        return L"FullscreenProgress";
    }

    return L"FullscreenProgressSplit";
}

void UIScene_ConnectingProgress::handleInput(int playerId, int action, bool repeat,
                                             bool released, bool pressed, bool& handled)
{
    if (!m_miniGame)
    {
        if (!m_confirmationVisible || action != 20)
        {
            return;
        }
    }
    else
    {
        gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
        if (action < 4)
        {
            return;
        }

        if (action > 5 && action != 20)
        {
            if (action != 21)
            {
                return;
            }

            if (m_removePlayerCallback != NULL)
            {
                m_removePlayerCallback(m_removePlayerCallbackData);
            }
            handled = true;
            m_cancelled = true;
            return;
        }
    }

    if (released)
    {
        sendInputToMovie(action, repeat, released, pressed);
    }
}

void UIScene_ConnectingProgress::handlePress(int first, int second)
{
    if (first != 0 || !m_confirmationVisible)
    {
        return;
    }

    m_confirmationVisible = false;
    if (m_removePlayerCallback != NULL)
    {
        m_removePlayerCallback(m_removePlayerCallbackData);
        m_removePlayer = true;
        return;
    }

    gConsoleUIController.NavigateToHomeMenu(false);
}

int UIScene_ConnectingProgress::getSceneType()
{
    return 31;
}
