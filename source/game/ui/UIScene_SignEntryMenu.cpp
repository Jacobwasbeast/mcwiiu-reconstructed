#include "game/ui/UIScene_SignEntryMenu.hpp"

#include <string>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/UIControl_Base.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/world/block/SignBlockEntity.hpp"
#include "platform/wiiu/CInput.hpp"

UIScene_SignEntryMenu::UIScene_SignEntryMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer), m_sign(), m_selectedLine(0), m_cancelRequested(false),
      m_keyboardActive(false)
{
    if (data != NULL)
        m_sign = *static_cast<boost::shared_ptr<SignBlockEntity>*>(data);

    initialiseMovie();
    m_confirmButton.init(UIString(L10N::GetString(0x6d2a309c)), 4);
    m_messageLabel.init(UIString(L10N::GetString(0x56c538)));

    for (int i = 0; i < 4; ++i)
        m_lineInputs[i].init(UIString(L""), i);

    if (layer != NULL)
        layer->addComponent(type, static_cast<EUIScene>(0x48), NULL);
}

UIScene_SignEntryMenu::~UIScene_SignEntryMenu()
{
    if (m_layer != NULL)
        m_layer->removeComponent(static_cast<EUIScene>(0x48));
}

unsigned int UIScene_SignEntryMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    m_confirmButton.setupControl(this, root, std::string("Confirm"));
    m_messageLabel.setupControl(this, root, std::string("Message"));
    m_lineInputs[0].setupControl(this, root, std::string("Line1"));
    m_lineInputs[1].setupControl(this, root, std::string("Line2"));
    m_lineInputs[2].setupControl(this, root, std::string("Line3"));
    m_lineInputs[3].setupControl(this, root, std::string("Line4"));
    m_controls.push_back(&m_confirmButton);
    m_controls.push_back(&m_messageLabel);
    for (int i = 0; i < 4; ++i)
        m_controls.push_back(&m_lineInputs[i]);
    return 1;
}

void UIScene_SignEntryMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_sceneType, 0x8d6323ed, 0x4f6bcf5e,
                                     -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_SignEntryMenu::tick()
{
    UIScene::tick();

    if (m_cancelRequested)
    {
        m_cancelRequested = false;
        if (m_sign != 0)
        {
            std::wstring empty;
            for (int i = 0; i < 4; ++i)
                m_sign->SetMessage(i, empty);
            m_sign->setChanged();
        }
        gConsoleUIController.CloseUIScenes(m_sceneType, false);
        return;
    }

    if (m_sign == 0)
        return;

}

std::wstring UIScene_SignEntryMenu::getMoviePath()
{
    if (g_pApp->GetLocalPlayerCount() < 2)
        return std::wstring(L"SignEntryMenu");
    return std::wstring(L"SignEntryMenuSplit");
}

void UIScene_SignEntryMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                        bool pressed, bool& handled)
{
    if (m_cancelRequested || m_keyboardActive)
        return;

    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, released, pressed);
    if (action > 3 && (action < 6 || action == 0x14))
    {
        sendInputToMovie(action, repeat, released, pressed);
        handled = true;
    }
    else if (action == 0x15 && released)
    {
        if (m_sign != 0)
        {
            std::wstring empty;
            for (int i = 0; i < 4; ++i)
                m_sign->SetMessage(i, empty);
            m_sign->setChanged();
        }
        navigateBack();
        handled = true;
    }
}

int UIScene_SignEntryMenu::KeyboardCompleteCallback(void* data, bool accepted)
{
    UIScene_SignEntryMenu* scene = static_cast<UIScene_SignEntryMenu*>(data);
    if (scene != NULL)
    {
        scene->m_keyboardActive = false;
        if (accepted && g_pInput != NULL)
        {
            wchar_t text[128];
            g_pInput->GetText(text, 127);
            scene->m_lineInputs[scene->m_selectedLine].setLabel(UIString(text), false, false);
        }
    }
    return 0;
}

void UIScene_SignEntryMenu::handlePress(int first, int)
{
    if (first < 4)
    {
        m_selectedLine = first;
        m_keyboardActive = true;
        if (g_pInput != NULL)
        {
            const wchar_t* title = L10N::GetString(0x3121eb00);
            const wchar_t* initial = m_lineInputs[m_selectedLine].getLabel();
            if (g_pInput->RequestKeyboard(title, initial, m_sceneType, 0x7f,
                                           KeyboardCompleteCallback, this,
                                           C_4JInput::EKeyboardMode_0) != 0)
                m_keyboardActive = false;
        }
    }
    else if (first == 4)
    {
        m_cancelRequested = true;
    }
}

void UIScene_SignEntryMenu::handleDestroy()
{
    if (g_pInput != NULL)
        g_pInput->DestroyKeyboard();
}

int UIScene_SignEntryMenu::getSceneType()
{
    return 0x1d;
}
