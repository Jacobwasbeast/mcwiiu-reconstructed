#include "game/ui/UIScene_HowToPlayMenu.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"

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
    gConsoleUIController.SetTooltips(m_playerId, 0x8d6323ed, 0x4f6bcf5,
                                     -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_HowToPlayMenu::updateComponents()
{
    bool showSecond = true;
    if (Minecraft::GetInstance()->mPendingLevelChange == 0)
    {
        m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x42), true);
    }
    else
    {
        m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x42), false);
        showSecond = g_pApp->GetLocalPlayerCount() == 1;
    }
    m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x43), showSecond);
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
    if (action < 4)
        return;
    if (action > 5)
    {
        if (action < 8)
            return;
        if (action > 9 && action != 0x14)
        {
            if (action != 0x15 || !pressed)
                return;
            navigateBack();
            return;
        }
    }
    sendInputToMovie(action, repeat, released, pressed);
}

void UIScene_HowToPlayMenu::handlePress(int first, int second)
{
    if (first != 0)
        return;
    m_buttonList.setCurrentSelection(second, false, true);
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    static const EUIScene scenes[33] = {
        static_cast<EUIScene>(0), static_cast<EUIScene>(1), static_cast<EUIScene>(2),
        static_cast<EUIScene>(3), static_cast<EUIScene>(4), static_cast<EUIScene>(5),
        static_cast<EUIScene>(6), static_cast<EUIScene>(7), static_cast<EUIScene>(8),
        static_cast<EUIScene>(9), static_cast<EUIScene>(10), static_cast<EUIScene>(11),
        static_cast<EUIScene>(12), static_cast<EUIScene>(13), static_cast<EUIScene>(14),
        static_cast<EUIScene>(15), static_cast<EUIScene>(16), static_cast<EUIScene>(17),
        static_cast<EUIScene>(18), static_cast<EUIScene>(19), static_cast<EUIScene>(20),
        static_cast<EUIScene>(21), static_cast<EUIScene>(22), static_cast<EUIScene>(23),
        static_cast<EUIScene>(24), static_cast<EUIScene>(25), static_cast<EUIScene>(26),
        static_cast<EUIScene>(27), static_cast<EUIScene>(28), static_cast<EUIScene>(29),
        static_cast<EUIScene>(30), static_cast<EUIScene>(31), static_cast<EUIScene>(32)
    };
    gConsoleUIController.NavigateToScene(m_playerId, scenes[second], 0,
                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
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
    return 0x11;
}
