#include "game/ui/UIScene_DLCMainMenu.hpp"

#include <stdint.h>
#include <string>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/cCommerce.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_DynamicButtonList.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"

extern ConsoleUIController gConsoleUIController;
extern CConsoleMinecraftApp gApp;
extern CMinecraftApp* g_pApp;

UIScene_DLCMainMenu::UIScene_DLCMainMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer), m_offerList(), m_titleLabel(), m_timer(), m_initialized(false)
{
    initialiseMovie();
    g_pApp->SetLiveLinkRequired(true);
    m_controlFocus = static_cast<int>((uintptr_t)data);
    m_titleLabel.init(UIString(0x65154db0));
    m_offerList.init(0);
    gApp.LoadLocalDLCImages();
    m_timer.setVisible(true);
    cCommerce* commerce = static_cast<cCommerce*>((void*)gApp.GetCommerceInstance());
    commerce->AttemptLogIn();
    addTimer(0, 100);
}

UIScene_DLCMainMenu::~UIScene_DLCMainMenu()
{
    g_pApp->SetLiveLinkRequired(false);
    gApp.FreeLocalDLCImages();
    cCommerce* commerce = static_cast<cCommerce*>((void*)gApp.GetCommerceInstance());
    commerce->ClearProductInfo();
}

unsigned int UIScene_DLCMainMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    m_offerList.setupControl(this, root, std::string("OffersList"));
    m_controls.push_back(&m_offerList);
    m_titleLabel.setupControl(this, root, std::string("OffersList_Title"));
    m_controls.push_back(&m_titleLabel);
    m_timer.setupControl(this, root, std::string("Timer"));
    m_controls.push_back(&m_timer);
    return 1;
}

int UIScene_DLCMainMenu::ExitDLCMainMenu(void* context, int,
                                          C4JStorage::EMessageResult)
{
    cCommerce* commerce = static_cast<cCommerce*>((void*)gApp.GetCommerceInstance());
    commerce->ClearProductInfo();
    static_cast<UIScene*>(context)->navigateBack();
    return 0;
}

void UIScene_DLCMainMenu::handleTimerComplete(int timerId)
{
    if (timerId == 0 && g_pApp->DLCInstallPending() == 0)
    {
        int parameter = static_cast<int>(0xb9a1bf4d);
        gConsoleUIController.RequestErrorMessage(0xf1c0f3d9, 0x16cb58e9,
                                                  &parameter, 1, m_controlFocus,
                                                  ExitDLCMainMenu, this, NULL, true);
    }
}

void UIScene_DLCMainMenu::handleGainFocus(bool hadFocus)
{
    updateTooltips();
    if (hadFocus)
        addTimer(0, 100);
}

void UIScene_DLCMainMenu::tick()
{
    UIScene::tick();
    if (!m_initialized)
    {
        m_initialized = true;
        m_timer.setVisible(false);
        m_offerList.addItem(UIString(0xb512343d), 0);
        m_offerList.addItem(UIString(0xe6ce8669), 1);
        m_offerList.addItem(UIString(0xf6194eb1), 2);
        m_offerList.addItem(UIString(0x73639b6c), 3);
        m_offerList.addItem(UIString(0x500dc372), 4);
    }
}

void UIScene_DLCMainMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(static_cast<unsigned int>(m_controlFocus),
                                      0x8d6323ed, 0x4f6bcf5e, -1, -1, -1, -1, -1, -1,
                                      -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

std::wstring UIScene_DLCMainMenu::getMoviePath()
{
    return std::wstring(L"DLCMainMenu");
}

void UIScene_DLCMainMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                      bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_controlFocus, action, repeat, released, pressed);
    if (action <= 3)
        return;
    if (action == 0x15)
    {
        if (released)
        {
            cCommerce* commerce = static_cast<cCommerce*>((void*)gApp.GetCommerceInstance());
            commerce->ClearProductInfo();
            navigateBack();
        }
        return;
    }
    if (action > 9 && action != 0x14)
        return;
    if (action == 0x14 && released)
        gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
    sendInputToMovie(action, repeat, released, pressed);
}

void UIScene_DLCMainMenu::handlePress(int first, int second)
{
    if (first == 0)
    {
        killTimer(0);
        int* data = new int[3]();
        data[0] = m_controlFocus;
        data[2] = second;
        gConsoleUIController.NavigateToScene(m_controlFocus, static_cast<EUIScene>(0x20),
                                             data, static_cast<EUILayer>(6),
                                             static_cast<EUIGroup>(7));
    }
}

int UIScene_DLCMainMenu::getSceneType()
{
    return 0x2c;
}
