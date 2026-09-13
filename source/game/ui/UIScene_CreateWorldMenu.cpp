#include "game/ui/UIScene_CreateWorldMenu.hpp"

#include <cwchar>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GameType.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"

extern ConsoleUIController gConsoleUIController;

UIScene_CreateWorldMenu::UIScene_CreateWorldMenu(int sceneType, void* data, UILayer* layer)
    : IUIScene_StartGame(sceneType, data, layer, false), m_worldNameLabel(),
      m_moreOptionsButton(), m_playButton(), m_worldNameInput(), m_worldIcon(),
      m_keyboardActive(false)
{
    initialiseMovie();
    m_worldName.assign(L10N::GetString(0x860494ea));
    m_playerId = sceneType;
    m_worldNameLabel.init(UIString(L10N::GetString(0x9db1d436)));
    m_worldNameInput.init(UIString(m_worldName), 0);
    m_worldNameInput.SetCharLimit(0x19);
    m_gameModeSlider.init(UIString(L10N::GetString(0x9ba782a6)), 1, 0,
                          GameType::CREATIVE->getId(), GameType::SURVIVAL->getId(), false, 10, 1);
    m_moreOptionsButton.init(UIString(L10N::GetString(0xbe1d5b9e)), 3);
    m_playButton.init(UIString(L10N::GetString(0xb0b877e6)), 6);
    m_texturePackList.init(std::wstring(L10N::GetString(0xf6194eb1)), 4);
    m_texturePackName.init(UIString(L""));
    m_texturePackDescription.init(UIString(L""));
    m_launchData.mFlag3 = true;
    m_difficulty = GameSettings::GetGameSettings(m_playerId, static_cast<eGameSetting>(3));
    wchar_t difficultyLabel[256];
    std::swprintf(difficultyLabel, 256, L"%ls: %ls", L10N::GetString(0xd324428d),
                  L10N::GetString(Difficulty::byId(m_difficulty)->getKey()));
    m_difficultySlider.init(UIString(difficultyLabel), 2, 0, 3, m_difficulty, false, 10, 1);
    bool initialChecked = m_launchData.mFlag0;
    m_onlineGameCheckBox.SetEnable(true);
    if (!gProfile->IsSignedIn(m_playerId))
        m_onlineGameCheckBox.SetEnable(false);
    if (m_launchData.mFlag17)
    {
        m_onlineGameCheckBox.SetEnable(false);
        initialChecked = false;
    }
    m_onlineGameCheckBox.init(UIString(L10N::GetString(0x2c692b72)), 5,
                              initialChecked);
    _init();
    if (data != NULL)
        ::operator delete(data);
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_CreateWorldMenu::~UIScene_CreateWorldMenu()
{
}

unsigned int UIScene_CreateWorldMenu::mapElementsAndNames()
{
    IUIScene_StartGame::mapElementsAndNames();
    if (m_movie == NULL)
        return 0;
    fuiRenderNode* root = m_mainPanel.getFuiRenderNode();
    m_worldNameLabel.setupControl(this, root, std::string("WorldName"));
    m_controls.push_back(&m_worldNameLabel);
    m_worldNameInput.setupControl(this, root, std::string("EditWorldName"));
    m_controls.push_back(&m_worldNameInput);
    m_texturePackList.setupControl(this, root, std::string("TexturePackSelector"));
    m_controls.push_back(&m_texturePackList);
    m_gameModeSlider.setupControl(this, root, std::string("GameModeToggle"));
    m_controls.push_back(&m_gameModeSlider);
    m_onlineGameCheckBox.setupControl(this, root, std::string("CheckboxOnline"));
    m_controls.push_back(&m_onlineGameCheckBox);
    m_moreOptionsButton.setupControl(this, root, std::string("MoreOptions"));
    m_controls.push_back(&m_moreOptionsButton);
    m_playButton.setupControl(this, root, std::string("NewWorld"));
    m_controls.push_back(&m_playButton);
    m_difficultySlider.setupControl(this, root, std::string("Difficulty"));
    m_controls.push_back(&m_difficultySlider);
    return 1;
}

void UIScene_CreateWorldMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(0, 0x8d6323ed, 0x4f6bcf5e, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_CreateWorldMenu::updateComponents()
{
    if (m_layer != NULL)
    {
        m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x42), true);
        m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x43), false);
    }
}

void UIScene_CreateWorldMenu::handleDestroy()
{
    if (g_pInput != NULL)
        g_pInput->DestroyKeyboard();
}

void UIScene_CreateWorldMenu::tick()
{
    IUIScene_StartGame::tick();
}

UIControl* UIScene_CreateWorldMenu::GetMainPanel()
{
    return &m_mainPanel;
}

std::wstring UIScene_CreateWorldMenu::getMoviePath()
{
    return std::wstring(L"CreateWorldMenu");
}

void UIScene_CreateWorldMenu::handleGainFocus(bool hadFocus)
{
    if (hadFocus)
        m_onlineGameCheckBox.setChecked(false);
}

void UIScene_CreateWorldMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                          bool pressed, bool& handled)
{
    if (m_keyboardActive)
        return;
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, released, pressed);
    if (action < 4)
        return;
    if (action > 7 && action < 0xe)
        return;
    if (action > 0xf && action != 0x14)
    {
        if (action != 0x15 || !pressed)
            return;
        navigateBack();
        handled = true;
        return;
    }
    sendInputToMovie(action, repeat, released, pressed);
    handled = true;
}

int UIScene_CreateWorldMenu::KeyboardCompleteWorldNameCallback(void* data, bool accepted)
{
    UIScene_CreateWorldMenu* scene = static_cast<UIScene_CreateWorldMenu*>(data);
    if (scene == NULL)
        return 0;
    scene->m_keyboardActive = false;
    if (accepted && g_pInput != NULL)
    {
        wchar_t text[128];
        g_pInput->GetText(text, 0x7f);
        if (text[0] != L'\0')
        {
            scene->m_worldNameInput.setLabel(UIString(text), false, false);
            scene->m_worldName.assign(text);
        }
        scene->SetEnablePlayGameButton(true);
    }
    return 0;
}

void UIScene_CreateWorldMenu::handlePress(int first, int second)
{
    if (m_keyboardActive)
        return;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
    if (first == 0)
    {
        m_keyboardActive = true;
        if (g_pInput != NULL && g_pInput->RequestKeyboard(L10N::GetString(0xb0b877e6),
                                                            m_worldName.c_str(), m_playerId, 0x19,
                                                            KeyboardCompleteWorldNameCallback, this,
                                                            C_4JInput::EKeyboardMode_0) != 0)
            m_keyboardActive = false;
    }
    else if (first == 3)
        gConsoleUIController.NavigateToScene(m_playerId, static_cast<EUIScene>(0x2b),
                                             &m_launchData, static_cast<EUILayer>(6),
                                             static_cast<EUIGroup>(7));
    else if (first == 4)
        UpdateCurrentTexturePack(second);
    else if (first == 6)
        StartSharedLaunchFlow();
}

void UIScene_CreateWorldMenu::handleCheckboxToggled(int, bool)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
}

void UIScene_CreateWorldMenu::checkStateAndStartGame()
{
    LaunchGame();
}

void UIScene_CreateWorldMenu::SetEnablePlayGameButton(bool enable)
{
    m_playButton.setEnable(enable, true);
}

int UIScene_CreateWorldMenu::getSceneType()
{
    return 0x1a;
}
