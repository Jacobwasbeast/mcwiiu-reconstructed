#include "game/ui/UIScene_LaunchMoreOptionsMenu.hpp"

#include <cwchar>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/FJ_LaunchMoreOptionsMenu.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/SuperflatMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/CInput.hpp"

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

UIScene_LaunchMoreOptionsMenu::UIScene_LaunchMoreOptionsMenu(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer), m_worldSize(), m_worldType(), m_gameMode(), m_lootSet(),
      m_foodBalance(), m_mapSize(), m_roundLength(), m_worldTab(), m_gameTab(), m_worldOptions(),
      m_gameOptions(), m_worldBackground(), m_gameBackground(), m_worldOptionsList(),
      m_gameOptionsList(), m_worldLabel(), m_gameLabel(), m_resizeLabel(), m_worldDescription(),
      m_gameDescription(), m_worldTabLabel(), m_gameTabLabel(), m_textPanelLabel(), m_optionsLabel(),
      m_initData(static_cast<LaunchMoreOptionsMenuInitData*>(data)), m_online(false),
      m_keyboardActive(false), m_menuType(0), m_updateCheats(false), m_updateOnline(false), m_seed()
{
    initialiseMovie();
    m_menuType = static_cast<int>(m_initData != NULL);
    _init();
    addTimer(0, 100);
}

UIScene_LaunchMoreOptionsMenu::~UIScene_LaunchMoreOptionsMenu()
{
}

void UIScene_LaunchMoreOptionsMenu::EnableCustomOptions(bool enabled)
{
    m_gameOptionsList.EnableItem(0x1e, enabled, true);
    m_gameOptionsList.EnableItem(0x22, enabled, true);
    m_gameOptionsList.EnableItem(0x23, enabled, true);
    m_gameOptionsList.EnableItem(0x24, enabled, true);
    m_gameOptionsList.EnableItem(0x25, enabled, true);
    m_gameOptionsList.EnableItem(0x27, enabled, true);
    m_gameOptionsList.EnableItem(0x28, enabled, true);
    m_gameOptionsList.EnableItem(0x29, enabled, true);
    m_gameOptionsList.EnableItem(0x2a, enabled, true);
}

void UIScene_LaunchMoreOptionsMenu::UpdateCheats()
{
    m_updateCheats = false;
    m_gameOptionsList.EnableItem(0xc, true, true);
    m_gameOptionsList.EnableItem(0xd, true, true);
    m_gameOptionsList.EnableItem(0xf, true, true);
    m_gameOptionsList.EnableItem(0x12, true, true);
    m_gameOptionsList.EnableItem(0x13, true, true);
}

void UIScene_LaunchMoreOptionsMenu::UpdateDescriptionText()
{
    m_worldDescription.setVisible(false);
    m_gameDescription.setVisible(false);
    if (m_menuType == 1)
        m_worldDescription.setVisible(true);
    else
        m_gameDescription.setVisible(true);
}

void UIScene_LaunchMoreOptionsMenu::_init()
{
    mapElementsAndNames();
    UpdateDescriptionText();
    updateTooltips();
    updateComponents();
}

unsigned int UIScene_LaunchMoreOptionsMenu::mapElementsAndNames()
{
    if (m_movie == NULL)
        return 0;
    fuiRenderNode* root = m_movie->getRootNode();
    m_worldLabel.setupControl(this, root, std::string("LabelWorld"));
    m_gameLabel.setupControl(this, root, std::string("LabelGame"));
    m_worldOptions.setupControl(this, root, std::string("WorldOptions"));
    m_worldOptionsList.setupControl(this, m_worldOptions.getFuiRenderNode(), std::string("WorldOptionsList"));
    m_worldTab.setupControl(this, m_worldOptions.getFuiRenderNode(), std::string("TouchTabWorld"));
    m_worldDescription.setupControl(this, m_worldOptions.getFuiRenderNode(), std::string("Description_WorldOptions"));
    m_gameOptions.setupControl(this, root, std::string("GameOptions"));
    m_gameOptionsList.setupControl(this, m_gameOptions.getFuiRenderNode(), std::string("GameOptionsList"));
    m_gameTab.setupControl(this, m_gameOptions.getFuiRenderNode(), std::string("TouchTabGame"));
    m_gameDescription.setupControl(this, m_gameOptions.getFuiRenderNode(), std::string("Description_GameOptions"));
    m_controls.push_back(&m_worldLabel);
    m_controls.push_back(&m_gameLabel);
    m_controls.push_back(&m_worldOptions);
    m_controls.push_back(&m_worldOptionsList);
    m_controls.push_back(&m_worldTab);
    m_controls.push_back(&m_worldDescription);
    m_controls.push_back(&m_gameOptions);
    m_controls.push_back(&m_gameOptionsList);
    m_controls.push_back(&m_gameTab);
    m_controls.push_back(&m_gameDescription);
    return 1;
}

void UIScene_LaunchMoreOptionsMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_sceneType, 0x8d6323ed, 0x4f6bcf5, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_LaunchMoreOptionsMenu::updateComponents()
{
    if (m_layer != NULL)
    {
        m_layer->showComponent(m_sceneType, static_cast<EUIScene>(0x42), true);
        m_layer->showComponent(m_sceneType, static_cast<EUIScene>(0x43), false);
    }
}

std::wstring UIScene_LaunchMoreOptionsMenu::getMoviePath()
{
    if (g_pApp != NULL && g_pApp->GetLocalPlayerCount() > 1)
        return std::wstring(L"LaunchMoreOptionsMenuSplit");
    return std::wstring(L"LaunchMoreOptionsMenu");
}

int UIScene_LaunchMoreOptionsMenu::KeyboardCompleteSeedCallback(void* data, bool accepted)
{
    UIScene_LaunchMoreOptionsMenu* scene = static_cast<UIScene_LaunchMoreOptionsMenu*>(data);
    if (scene != NULL)
    {
        scene->m_keyboardActive = false;
        if (accepted && g_pInput != NULL)
        {
            wchar_t text[128];
            g_pInput->GetText(text, 127);
            scene->m_seed.assign(text);
            scene->UpdateDescriptionText();
        }
    }
    return 0;
}

int UIScene_LaunchMoreOptionsMenu::HandleRequestKeyboard()
{
    m_keyboardActive = true;
    if (g_pInput == NULL)
        return 0;
    const wchar_t* title = L10N::GetString(0xa60b975e);
    if (g_pInput->RequestKeyboard(title, m_seed.c_str(), m_sceneType, 0x3c,
                                  KeyboardCompleteSeedCallback, this,
                                  C_4JInput::EKeyboardMode_0) != 0)
        m_keyboardActive = false;
    return 0;
}

void UIScene_LaunchMoreOptionsMenu::UpdateOnline()
{
    m_gameOptionsList.EnableItem(2, m_online, true);
    m_updateOnline = false;
}

void UIScene_LaunchMoreOptionsMenu::tick()
{
    UIScene::tick();
    if (m_updateCheats)
        UpdateCheats();
    if (m_updateOnline)
        UpdateOnline();
}

void UIScene_LaunchMoreOptionsMenu::handleDestroy()
{
    if (g_pInput != NULL)
        g_pInput->DestroyKeyboard();
}

void UIScene_LaunchMoreOptionsMenu::handleReload()
{
    _init();
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_LaunchMoreOptionsMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                                bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, released, pressed);
    if (action > 3 && (action < 10 || action == 0x14))
        sendInputToMovie(action, repeat, released, pressed);
    if (action == 0x15 && pressed)
    {
        navigateBack();
        handled = true;
    }
}

void UIScene_LaunchMoreOptionsMenu::handleFocusChange(int focus, int childFocus)
{
    m_controlFocus = focus;
    m_controlChildFocus = childFocus;
    UpdateDescriptionText();
}

void UIScene_LaunchMoreOptionsMenu::handleTimerComplete(int)
{
}

void UIScene_LaunchMoreOptionsMenu::handlePress(int first, int second)
{
    gConsoleUIController.PlayUISFX(NULL);
    if (first == 0x16)
    {
        m_keyboardActive = false;
        HandleRequestKeyboard();
    }
    else if (second == 0x15 || second == 0x19 || second == 0x1b)
        navigateBack();
    else if (second == 0x2d)
        UpdateDescriptionText();
}

void UIScene_LaunchMoreOptionsMenu::handleSliderElementMove(int, int second, int value)
{
    if (second == 0x2e)
        m_worldSize.Update(m_gameOptionsList, value);
    else if (second == 0x2f)
        m_roundLength.Update(m_gameOptionsList, value);
    UpdateDescriptionText();
}

void UIScene_LaunchMoreOptionsMenu::handleGainFocus(bool)
{
    UpdateDescriptionText();
}

void UIScene_LaunchMoreOptionsMenu::handleCheckboxElementToggled(int, int second, bool checked)
{
    gConsoleUIController.PlayUISFX(NULL);
    if (second == 0)
        m_online = checked;
    if (second == 7)
        m_updateCheats = true;
    if (second == 0x1a)
        m_updateOnline = true;
    UpdateDescriptionText();
}

void UIScene_LaunchMoreOptionsMenu::handleTouchInput(unsigned int, int action, int, int, bool repeat,
                                                     bool, bool pressed)
{
    if (pressed && action > 0x33 && action < 0x36)
    {
        gConsoleUIController.PlayUISFX(NULL);
        m_menuType = action == 0x34;
        UpdateDescriptionText();
        gConsoleUIController.TouchBoxRebuild(this);
    }
    (void)repeat;
}

UIControl* UIScene_LaunchMoreOptionsMenu::GetMainPanel()
{
    return m_menuType == 1 ? &m_worldBackground : &m_gameBackground;
}

int UIScene_LaunchMoreOptionsMenu::getSceneType()
{
    return 0x2b;
}
