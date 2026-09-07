#include "game/ui/UIScene_DeathMenu.hpp"

#include <string>
#include <vector>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UIScene_PauseMenu.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"

UIScene_DeathMenu::UIScene_DeathMenu(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer), m_deathMenuActive(false), m_unknownStateA(false),
      m_unknownStateB(false)
{
    initialiseMovie();

    if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x1f)) == 0)
    {
        m_deathMenuActive = false;
        m_exitButton.init(UIString(L10N::GetString(0x2686c8e5)), 1);
        m_respawnButton.init(UIString(L10N::GetString(0x52841e60)), 0);
        m_exitButton.setEnable(false, true);
        m_respawnButton.setEnable(false, true);
        addTimer(1, 1000);
    }
    else
    {
        m_deathMenuActive = true;
        m_exitButton.setVisible(false);
        m_respawnButton.setVisible(false);
        addTimer(0, Minecraft::InMiniGame(static_cast<EMiniGameId>(3), true) ? 1000 : 2000);
    }

    if (!Minecraft::InMiniGame(static_cast<EMiniGameId>(3), true))
        m_titleLabel.setLabel(UIString(L10N::GetString(0x69c7eda4)), false, false);
    else
        m_titleLabel.setLabel(UIString(L10N::GetString(0x7b5878dc)), false, false);

    if (!Minecraft::InMiniGame(static_cast<EMiniGameId>(2), true) &&
        !Minecraft::InMiniGame(static_cast<EMiniGameId>(3), true))
    {
        m_deathMessageLabel.setLabel(UIString(L""), false, false);
        m_unknownStateA = false;
        m_unknownStateB = false;
    }
    else
    {
        m_deathMessageLabel.setLabel(UIString(L""), false, false);
        m_unknownStateB = false;
        m_unknownStateA = false;
    }

    m_extraMessageLabel.setLabel(UIString(L""), false, false);

    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != NULL && minecraft->mGameModes[sceneType] != NULL)
    {
        Tutorial* tutorial = minecraft->mGameModes[sceneType]->getTutorial();
        if (tutorial != NULL)
            tutorial->showTutorialPopup(false);
    }
}

UIScene_DeathMenu::~UIScene_DeathMenu()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    ClientMasterGameMode* gameMode = minecraft != NULL ? minecraft->GetClientMasterGameMode() : NULL;
    if (gameMode != NULL && gameMode->IsRoundRestarting() && hasTimer(0))
        handleTimerComplete(0);

    if (minecraft != NULL && minecraft->mGameModes[m_resolution] != NULL)
    {
        Tutorial* tutorial = minecraft->mGameModes[m_resolution]->getTutorial();
        if (tutorial != NULL)
            tutorial->showTutorialPopup(true);
    }
}

unsigned int UIScene_DeathMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    m_respawnButton.setupControl(this, root, std::string("Respawn"));
    m_controls.push_back(&m_respawnButton);
    m_exitButton.setupControl(this, root, std::string("ExitGame"));
    m_controls.push_back(&m_exitButton);
    m_titleLabel.setupControl(this, root, std::string("Title"));
    m_controls.push_back(&m_titleLabel);
    m_deathMessageLabel.setupControl(this, root, std::string("DeathMessage"));
    m_controls.push_back(&m_deathMessageLabel);
    m_extraMessageLabel.setupControl(this, root, std::string("ExtraMessage"));
    m_controls.push_back(&m_extraMessageLabel);
    return 1;
}

void UIScene_DeathMenu::handleTimerComplete(int timerId)
{
    if (timerId == 0)
    {
        g_pApp->SetAction(m_resolution, static_cast<eXuiAction>(6), NULL);
        killTimer(0);
    }
    else if (timerId != 1)
        return;

    m_exitButton.setEnable(true, true);
    m_respawnButton.setEnable(true, true);
}

void UIScene_DeathMenu::updateTooltips()
{
    int tooltip = 0x8d6323ed;
    if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x1f)) != 0)
        tooltip = -1;
    gConsoleUIController.SetTooltips(m_resolution, tooltip, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

std::wstring UIScene_DeathMenu::getMoviePath()
{
    if (g_pApp->GetLocalPlayerCount() < 2)
        return std::wstring(L"DeathMenu");
    return std::wstring(L"DeathMenuSplit");
}

void UIScene_DeathMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                    bool pressed, bool& handled)
{
    if (!m_deathMenuActive)
    {
        gConsoleUIController.AnimateKeyPress(playerId, action, repeat, released, pressed);
        if (action > 3)
        {
            if (action < 6 || action == 0x14)
                sendInputToMovie(action, repeat, released, pressed);
            else if (action != 0x15)
                return;
            handled = true;
        }
    }
}

void UIScene_DeathMenu::tick()
{
    UIScene::tick();
}

void UIScene_DeathMenu::handlePress(int first, int)
{
    if (first == 0)
    {
        m_deathMenuActive = true;
        g_pApp->SetAction(m_resolution, static_cast<eXuiAction>(6), NULL);
    }
    else if (first == 1)
    {
        unsigned int callbackId = GetCallbackUniqueId();
        static_cast<UIScene_PauseMenu*>(m_layer)->OpenExitGameDialog(
            callbackId, (void*)0x02f78ca8, (void*)0x02f79218, 0);
    }
}

int UIScene_DeathMenu::getSceneType()
{
    return 0x18;
}
