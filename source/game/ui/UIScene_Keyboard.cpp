#include "game/ui/UIScene_Keyboard.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/UIString.hpp"

UIScene_Keyboard::UIScene_Keyboard(int sceneType, void* sceneData, UILayer* layer)
    : UIScene(sceneType, layer), m_keyboardDone(false)
{
    initialiseMovie();

    m_enterTextLabel.init(UIString(L"Enter Sign Text"));
    m_keyboardTextInput.init(UIString(L"Enter Sign Text"), -1);
    m_keyboardTextInput.SetCharLimit(15);
    m_spaceButton.init(UIString(L"Space"), -1);
    m_cursorLeftButton.init(UIString(L"Cursor Left"), -1);
    m_cursorRightButton.init(UIString(L"Cursor Right"), -1);
    m_capsButton.init(UIString(L"Caps"), -1);
    m_doneButton.init(UIString(L"Done"), 0);
    m_symbolsButton.init(UIString(L"Symbols"), -1);
    m_backspaceButton.init(UIString(L"Backspace"), -1);

    layer->addComponent(sceneType, static_cast<EUIScene>(0x48), sceneData);
}

UIScene_Keyboard::~UIScene_Keyboard()
{
    m_layer->removeComponent(static_cast<EUIScene>(0x48));
}

unsigned int UIScene_Keyboard::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();

    m_enterTextLabel.setupControl(this, root, std::string("EnterTextLabel"));
    m_controls.push_back(&m_enterTextLabel);
    m_keyboardTextInput.setupControl(this, root, std::string("KeyboardTextInput"));
    m_controls.push_back(&m_keyboardTextInput);
    m_spaceButton.setupControl(this, root, std::string("Button_space"));
    m_controls.push_back(&m_spaceButton);
    m_cursorLeftButton.setupControl(this, root, std::string("Button_CursorLeft"));
    m_controls.push_back(&m_cursorLeftButton);
    m_cursorRightButton.setupControl(this, root, std::string("Button_CursorRight"));
    m_controls.push_back(&m_cursorRightButton);
    m_capsButton.setupControl(this, root, std::string("Button_Caps"));
    m_controls.push_back(&m_capsButton);
    m_doneButton.setupControl(this, root, std::string("Button_Done"));
    m_controls.push_back(&m_doneButton);
    m_symbolsButton.setupControl(this, root, std::string("Button_symbols"));
    m_controls.push_back(&m_symbolsButton);
    m_backspaceButton.setupControl(this, root, std::string("Button_bspace"));
    m_controls.push_back(&m_backspaceButton);

    return 1;
}

void UIScene_Keyboard::updateTooltips()
{
    gConsoleUIController.SetTooltips(0, 0x8d6323ed, 0x4f6bcf5e, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

bool UIScene_Keyboard::allowRepeat(int action)
{
    return action > 1 && action != 0x12 && (action < 0x14 || action > 0x15);
}

void UIScene_Keyboard::handleInput(int, int, bool, bool, bool, bool&)
{
}

void UIScene_Keyboard::KeyboardDonePressed()
{
    navigateBack();
}

void UIScene_Keyboard::handleTimerComplete(int timerId)
{
    if (timerId == 0)
    {
        killTimer(0);
        KeyboardDonePressed();
    }
}

void UIScene_Keyboard::handlePress(int first, int)
{
    if (first == 0 && !m_keyboardDone)
    {
        addTimer(0, 100);
        m_keyboardDone = true;
    }
}

std::wstring UIScene_Keyboard::getMoviePath()
{
    if (g_pApp->GetLocalPlayerCount() < 2 || m_layer->IsFullscreenGroup())
        return std::wstring(L"Keyboard");
    return std::wstring(L"KeyboardSplit");
}

int UIScene_Keyboard::getSceneType()
{
    return 0x49;
}

bool UIScene_Keyboard::hasFocus(int)
{
    return m_hasFocus;
}

bool UIScene_Keyboard::hidesLowerScenes()
{
    return false;
}
