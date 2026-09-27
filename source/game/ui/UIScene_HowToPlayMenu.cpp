#include "game/ui/UIScene_HowToPlayMenu.hpp"
#include "game/core/StringIds.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/core/GameSettings.hpp"

#include <stdint.h>

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

UIScene_HowToPlayMenu::UIScene_HowToPlayMenu(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer)
{
    static const unsigned int labels[32] = {
        0x0ad78575, 0x4e09b239, 0xa1bfe39a, 0xfe7ebcc1,
        0x665939c3, 0xf451ca88, 0x1a5444f1, 0x5d603ceb,
        0xa89bcc3a, 0x9bfe8c42, 0xf8fe2a8a, 0xae9ec2e0,
        0xdc9bf801, 0x95ea9872, 0x6c8abc81, 0x55175d0c,
        0xc5e8da45, 0x68b19531, 0xccd04093, 0x0b54d580,
        0x74c4b1e7, 0x0b9c0e92, 0xc48a1dcf, 0x97d17f3b,
        0xd554b135, 0x07ff49bc, 0x90785456, 0x670457b6,
        0x3fe78b47, 0x6533d815, 0x7ee2ae4a, 0x6ef61516
    };
    initialiseMovie();
    m_buttonList.init(0);
    for (int i = 0; i < 16; ++i)
    {
        m_buttonList.addItem(std::wstring(L10N::GetString(static_cast<int>(labels[i * 2]))), i * 2);
        m_buttonList.addItem(std::wstring(L10N::GetString(static_cast<int>(labels[i * 2 + 1]))), i * 2 + 1);
    }
    m_buttonList.addItem(std::wstring(L10N::GetString(static_cast<int>(0xf3d9c148))), 32);
    m_buttonList.setCurrentSelection(0, false, true);
    doHorizontalResizeCheck();
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_HowToPlayMenu::~UIScene_HowToPlayMenu()
{
}

void UIScene_HowToPlayMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK,
                                     -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_HowToPlayMenu::updateComponents()
{
    bool showSecond = true;
    if (Minecraft::GetInstance()->mPendingLevelChange == 0)
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
    }
    else
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, false);
        showSecond = g_pApp->GetLocalPlayerCount() == 1;
    }
    m_layer->showComponent(m_playerId, EUIScene_Logo, showSecond);
}

void UIScene_HowToPlayMenu::handleReload()
{
    static const unsigned int labels[32] = {
        0x0ad78575, 0x4e09b239, 0xa1bfe39a, 0xfe7ebcc1,
        0x665939c3, 0xf451ca88, 0x1a5444f1, 0x5d603ceb,
        0xa89bcc3a, 0x9bfe8c42, 0xf8fe2a8a, 0xae9ec2e0,
        0xdc9bf801, 0x95ea9872, 0x6c8abc81, 0x55175d0c,
        0xc5e8da45, 0x68b19531, 0xccd04093, 0x0b54d580,
        0x74c4b1e7, 0x0b9c0e92, 0xc48a1dcf, 0x97d17f3b,
        0xd554b135, 0x07ff49bc, 0x90785456, 0x670457b6,
        0x3fe78b47, 0x6533d815, 0x7ee2ae4a, 0x6ef61516
    };
    for (int i = 0; i < 16; ++i)
    {
        m_buttonList.addItem(std::wstring(L10N::GetString(static_cast<int>(labels[i * 2]))), i * 2);
        m_buttonList.addItem(std::wstring(L10N::GetString(static_cast<int>(labels[i * 2 + 1]))), i * 2 + 1);
    }
    m_buttonList.addItem(std::wstring(L10N::GetString(static_cast<int>(0xf3d9c148))), 32);
    m_buttonList.setCurrentSelection(m_buttonList.getCurrentSelection(), false, true);
    doHorizontalResizeCheck();
    gConsoleUIController.TouchBoxRebuild(this);
}

std::wstring UIScene_HowToPlayMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"HowToPlayMenu")
                                             : std::wstring(L"HowToPlayMenuSplit");
}

void UIScene_HowToPlayMenu::handleInput(int, int action, bool repeat, bool released, bool pressed,
                                        bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    switch (action)
    {
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, released, pressed);
        break;
    case EControllerActions_MenuCancel:
        if (pressed)
            navigateBack();
        break;
    }
}

void UIScene_HowToPlayMenu::handlePress(int controlId, int childId)
{
    if (controlId != 0)
        return;
    m_buttonList.setCurrentSelection(childId, false, true);
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    const uint32_t sceneData = 0x80000000u |
        (static_cast<uint32_t>(childId) << 16) |
        static_cast<uint32_t>(static_cast<uint16_t>(m_playerId));
    gConsoleUIController.NavigateToScene(m_playerId, EUIScene_HowToPlay,
                                         reinterpret_cast<void*>(static_cast<uintptr_t>(sceneData)),
                                         EUILayer_Scene, EUIGroup_PlayerDefault);
}

unsigned int UIScene_HowToPlayMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    m_buttonList.setupControl(this, root, std::string("HowToList"));
    m_controls.push_back(&m_buttonList);
    return 1;
}

int UIScene_HowToPlayMenu::getSceneType()
{
    return EUIScene_HowToPlayMenu;
}
