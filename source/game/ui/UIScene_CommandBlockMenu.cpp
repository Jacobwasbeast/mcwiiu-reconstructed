#include "game/ui/UIScene_CommandBlockMenu.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"

extern CConsoleMinecraftApp gApp;

UIScene_CommandBlockMenu::UIScene_CommandBlockMenu(int type, void* data, UILayer* layer)
    : m_data(data), m_type(type), m_commandBlock(0), m_layer(layer)
{
}

UIScene_CommandBlockMenu::~UIScene_CommandBlockMenu()
{
}

std::wstring UIScene_CommandBlockMenu::getMoviePath()
{
        if (gApp.GetLocalPlayerCount() < 2)
        return std::wstring(L"CommandBlockMenu");
    return std::wstring(L"CommandBlockMenuSplit");
}

void UIScene_CommandBlockMenu::Initialise(CommandBlockEntity* commandBlock)
{
    m_commandBlock = commandBlock;
}

void UIScene_CommandBlockMenu::ConfirmButtonClicked()
{
}
