#include "game/ui/UIScene_InGameHostOptionsMenu.hpp"

#include <cwchar>
#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/command/GameDifficultyCommand.hpp"
#include "game/command/TimeCommand.hpp"
#include "game/command/WeatherCommand.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameType.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/network/ServerSettingsChangedPacket.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_InGameHostOptions.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

static void addHostCheckbox(UIControl_MultiList* list, int stringId, int id, bool checked)
{
    list->AddNewCheckbox(std::wstring(L10N::GetString(stringId)), id, checked);
}

static void addHostButton(UIControl_MultiList* list, int stringId, int id)
{
    list->AddNewButton(std::wstring(L10N::GetString(stringId)), id, true);
}

UIScene_InGameHostOptionsMenu::UIScene_InGameHostOptionsMenu(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer), m_hostOptions()
{
    initialiseMovie();
    Init();
}

UIScene_InGameHostOptionsMenu::~UIScene_InGameHostOptionsMenu()
{
}

unsigned int UIScene_InGameHostOptionsMenu::getWeatherId(int weatherState)
{
    if (weatherState == 1)
        return 0xb05e725f;
    if (weatherState == 2)
        return 0xa8f9a6d7;
    return 0xd885f542;
}

void UIScene_InGameHostOptionsMenu::Init()
{
    addHostCheckbox(&m_hostOptions, 0x1de3701, 0, g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0xe)) != 0);
    addHostCheckbox(&m_hostOptions, 0xef311201, 1, g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0xd)) != 0);
    if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0xf)) != 0)
    {
        addHostCheckbox(&m_hostOptions, 0x5e940e19, 2, g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x1e)) != 0);
        addHostCheckbox(&m_hostOptions, 0x7a2d3b2, 3, g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x19)) != 0);
        addHostCheckbox(&m_hostOptions, 0x3decac90, 4, g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x1a)) != 0);
        addHostCheckbox(&m_hostOptions, 0x2a7c56c4, 5, g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x18)) != 0);
        addHostButton(&m_hostOptions, 0xd554d957, 0xc);
        addHostButton(&m_hostOptions, 0xc0d12a27, 0xd);

        const Difficulty* difficulty = Difficulty::byId(g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0)));
        wchar_t difficultyLabel[256];
        swprintf(difficultyLabel, 256, L"%ls: %ls", L10N::GetString(0xd324428d),
                 L10N::GetString(difficulty->getKey()));
        m_hostOptions.AddNewSlider(std::wstring(difficultyLabel), 0xf, 0, 3,
                                   g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0)), false, 7, 3);

        const GameType* gameType = GameType::byId(g_pApp->GetGameHostOption(static_cast<eGameHostOption>(6)));
        m_hostOptions.AddNewSlider(std::wstring(L10N::GetString(gameType->getSliderLabel())), 0x10, 0,
                                   GameType::ADVENTURE->getId(), g_pApp->GetGameHostOption(static_cast<eGameHostOption>(6)),
                                   false, 7, 3);
        addHostButton(&m_hostOptions, 0x4c920637, 0xe);

        Level* level = Minecraft::GetInstance()->getLevel(0);
        if (level != 0)
        {
            m_hostOptions.AddNewLabel(std::wstring(L" "), 0x12);
            m_hostOptions.AddNewLabel(std::wstring(L10N::GetString(0x38c1c044)), 0x12);
            m_hostOptions.AddNewSlider(std::wstring(L10N::GetString(getWeatherId(level->getWeatherState()))),
                                       0x11, 0, 2, level->getWeatherState(), false, 7, 3);
        }
        addHostCheckbox(&m_hostOptions, 0xfaaaeb73, 9, g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x15)) != 0);
    }
    addHostCheckbox(&m_hostOptions, 0x4aac0bc3, 6, g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x1b)) != 0);
    addHostCheckbox(&m_hostOptions, 0x78e0a40d, 7, g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x1c)) != 0);
    addHostCheckbox(&m_hostOptions, 0x6f7d8591, 8, g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x1d)) != 0);

    Level* level = Minecraft::GetInstance()->getLevel(0);
    bool hasMultiplePlayers = false;
    if (level != 0)
    {
        std::vector<boost::shared_ptr<Player> >* players = level->getPlayers(0, NULL);
        hasMultiplePlayers = players->size() > 1;
        delete players;
    }
    if (level != 0 && g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0xf)) != 0 &&
        Player::getPlayerGamePrivilege(g_pApp->GetPlayerPrivileges(static_cast<unsigned char>(m_sceneType)),
                                       static_cast<Player::EPlayerGamePrivileges>(0x10)) != 0 &&
        hasMultiplePlayers)
    {
        addHostButton(&m_hostOptions, 0xdd235a1d, 0xa);
        addHostButton(&m_hostOptions, 0x4296e646, 0xb);
    }
    if (m_movie != 0)
    {
        fuiRenderNode* root = m_movie->getRootNode();
        if (root != 0 && root->getFuiNode() != 0)
            static_cast<FJ_InGameHostOptions*>(root->getFuiNode())->InitMenu();
    }
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_InGameHostOptionsMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_sceneType, 0x8d6323ed, 0x4f6bcf5e, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_InGameHostOptionsMenu::handleReload()
{
    Init();
    gConsoleUIController.TouchBoxRebuild(this);
}

std::wstring UIScene_InGameHostOptionsMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"InGameHostOptions") :
                                               std::wstring(L"InGameHostOptionsSplit");
}

void UIScene_InGameHostOptionsMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                                bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_sceneType, action, repeat, released, pressed);
    if (action <= 3)
        return;
    if (action < 10 || action == 0x14)
    {
        sendInputToMovie(action, repeat, released, pressed);
        return;
    }
    if (action != 0x15 || !pressed)
        return;

    unsigned int options = g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x20));
    g_pApp->SetGameHostOption(options, static_cast<eGameHostOption>(0xe), m_hostOptions.GetCheckboxValue(0));
    g_pApp->SetGameHostOption(options, static_cast<eGameHostOption>(0xd), m_hostOptions.GetCheckboxValue(1));
    bool settingsChanged = false;
    if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0xf)) != 0)
    {
        g_pApp->SetGameHostOption(options, static_cast<eGameHostOption>(0x1e), m_hostOptions.GetCheckboxValue(2));
        g_pApp->SetGameHostOption(options, static_cast<eGameHostOption>(0x19), m_hostOptions.GetCheckboxValue(3));
        g_pApp->SetGameHostOption(options, static_cast<eGameHostOption>(0x1a), m_hostOptions.GetCheckboxValue(4));
        g_pApp->SetGameHostOption(options, static_cast<eGameHostOption>(0x18), m_hostOptions.GetCheckboxValue(5));
        g_pApp->SetGameHostOption(options, static_cast<eGameHostOption>(0x15), m_hostOptions.GetCheckboxValue(9));
        int difficulty = static_cast<int>(m_hostOptions.GetSliderValue(0xf));
        if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0)) != difficulty)
        {
            ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_sceneType);
            if (connection != 0)
            {
                connection->send(GameDifficultyCommand::preparePacket(difficulty));
                settingsChanged = true;
            }
        }
        unsigned int gameType = static_cast<unsigned int>(m_hostOptions.GetSliderValue(0x10));
        if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(6)) != gameType)
        {
            ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_sceneType);
            if (connection != 0)
            {
                arrayWithLength<unsigned char> data(1, false);
                data.data[0] = static_cast<unsigned char>(gameType);
                connection->send(boost::shared_ptr<Packet>(
                    new GameCommandPacket(eGameCommand_DefaultGameMode, data)));
                settingsChanged = true;
            }
        }
        Level* level = Minecraft::GetInstance()->getLevel(0);
        if (level != 0)
        {
            unsigned int weather = m_hostOptions.GetSliderValue(0x11);
            if (level->getWeatherState() != weather)
            {
                ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_sceneType);
                if (connection != 0)
                    connection->send(WeatherCommand::preparePacket(static_cast<unsigned char>(weather)));
            }
        }
    }
    g_pApp->SetGameHostOption(options, static_cast<eGameHostOption>(0x1b), m_hostOptions.GetCheckboxValue(6));
    g_pApp->SetGameHostOption(options, static_cast<eGameHostOption>(0x1c), m_hostOptions.GetCheckboxValue(7));
    g_pApp->SetGameHostOption(options, static_cast<eGameHostOption>(0x1d), m_hostOptions.GetCheckboxValue(8));
    if (settingsChanged || options != g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x20)))
    {
        ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_sceneType);
        if (connection != 0)
            connection->send(boost::shared_ptr<Packet>(new ServerSettingsChangedPacket(1, options)));
    }
    navigateBack();
    handled = true;
    (void)playerId;
}

void UIScene_InGameHostOptionsMenu::handlePress(int, int second)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (second < 10 || second > 0xe)
        return;
    if (second < 0xc)
    {
        struct SceneData { int player; bool value; char padding[3]; };
        SceneData* data = new SceneData;
        data->player = m_sceneType;
        data->value = second == 10;
        gConsoleUIController.NavigateToScene(m_sceneType, static_cast<EUIScene>(0x34), data,
                                             static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
        return;
    }
    ClientPacketListener* connection = Minecraft::GetInstance()->getConnection(m_sceneType);
    if (connection == 0)
        return;
    if (second == 0xc)
        connection->send(TimeCommand::preparePacket(1000));
    else if (second == 0xd)
        connection->send(TimeCommand::preparePacket(14000));
    else
        connection->send(boost::shared_ptr<Packet>(new GameCommandPacket(
            eGameCommand_SetDefaultSpawnpoint, arrayWithLength<unsigned char>())));
}

void UIScene_InGameHostOptionsMenu::handleCheckboxElementToggled(int, int, bool)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
}

void UIScene_InGameHostOptionsMenu::handleSliderElementMove(int, int element, int value)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    if (element == 0xf)
    {
        const Difficulty* difficulty = Difficulty::byId(value);
        wchar_t label[256];
        swprintf(label, 256, L"%ls: %ls", L10N::GetString(0xd324428d), L10N::GetString(difficulty->getKey()));
        m_hostOptions.SetItemLabel(0xf, UIString(label), false);
    }
    else if (element < 0x11 && element > 0xf)
    {
        const GameType* gameType = GameType::byId(value);
        m_hostOptions.SetItemLabel(element, UIString(L10N::GetString(gameType->getSliderLabel())), false);
    }
    else if (element == 0x11)
    {
        m_hostOptions.SetItemLabel(0x11, UIString(L10N::GetString(getWeatherId(value))), false);
    }
}

unsigned int UIScene_InGameHostOptionsMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    m_hostOptions.setupControl(this, root, std::string("HostOptionsList"));
    m_controls.push_back(&m_hostOptions);
    return 1;
}

int UIScene_InGameHostOptionsMenu::getSceneType()
{
    return 0x28;
}
