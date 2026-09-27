#include "game/ui/UIScene_CreateWorldMenu.hpp"
#include "game/core/StringIds.hpp"

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
    m_worldName.assign(L10N::GetString(IDS_DEFAULT_WORLD_NAME));
    m_playerId = sceneType;
    m_worldNameLabel.init(UIString(L10N::GetString(IDS_WORLD_NAME)));
    m_worldNameInput.init(UIString(m_worldName), 0);
    m_worldNameInput.SetCharLimit(0x19);
    m_gameModeSlider.init(UIString(L10N::GetString(IDS_GAMEMODE_SURVIVAL)), 1, 0,
                          GameType::CREATIVE->getId(), GameType::SURVIVAL->getId(), false, 10, 1);
    m_moreOptionsButton.init(UIString(L10N::GetString(IDS_MORE_OPTIONS)), 3);
    m_playButton.init(UIString(L10N::GetString(IDS_CREATE_NEW_WORLD)), 6);
    m_texturePackList.init(std::wstring(L10N::GetString(0xf6194eb1)), 4);
    m_texturePackName.init(UIString(L""));
    m_texturePackDescription.init(UIString(L""));
    m_launchData.mNewWorld = true;
    m_difficulty = GameSettings::GetGameSettings(m_playerId, eGameSetting_Difficulty);
    wchar_t difficultyLabel[256];
    std::swprintf(difficultyLabel, 256, L"%ls: %ls", L10N::GetString(IDS_SLIDER_DIFFICULTY),
                  L10N::GetString(Difficulty::byId(m_difficulty)->getKey()));
    m_difficultySlider.init(UIString(difficultyLabel), 2, 0, 3, m_difficulty, false, 10, 1);
    bool initialChecked = m_launchData.mOnlineGame;
    m_onlineGameCheckBox.SetEnable(true);
    if (!gProfile->IsSignedInLive(m_playerId))
        m_onlineGameCheckBox.SetEnable(false);
    if (m_launchData.mOnlineSettingChangedBySystem)
    {
        m_onlineGameCheckBox.SetEnable(false);
        initialChecked = false;
    }
    m_onlineGameCheckBox.init(UIString(L10N::GetString(IDS_ONLINE_GAME)), 5,
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
    gConsoleUIController.SetTooltips(0, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_CreateWorldMenu::updateComponents()
{
    if (m_layer != NULL)
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
        m_layer->showComponent(m_playerId, EUIScene_Logo, false);
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
    switch (action)
    {
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuOtherStickUp:
    case EControllerActions_MenuOtherStickDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, released, pressed);
        handled = true;
        break;
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            navigateBack();
            handled = true;
        }
        break;
    }
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

void UIScene_CreateWorldMenu::handlePress(int controlId, int childId)
{
    if (m_keyboardActive)
        return;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (controlId == 0)
    {
        m_keyboardActive = true;
        if (g_pInput != NULL && g_pInput->RequestKeyboard(L10N::GetString(IDS_CREATE_NEW_WORLD),
                                                            m_worldName.c_str(), m_playerId, 0x19,
                                                            KeyboardCompleteWorldNameCallback, this,
                                                            C_4JInput::EKeyboardMode_0) != 0)
            m_keyboardActive = false;
    }
    else if (controlId == 3)
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_LaunchMoreOptionsMenu,
                                             &m_launchData, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
    else if (controlId == 4)
        UpdateCurrentTexturePack(childId);
    else if (controlId == 6)
        StartSharedLaunchFlow();
}

void UIScene_CreateWorldMenu::handleCheckboxToggled(int, bool)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
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
    return EUIScene_CreateWorldMenu;
}
