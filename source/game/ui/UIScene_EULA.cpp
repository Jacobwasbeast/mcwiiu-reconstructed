#include "game/ui/UIScene_EULA.hpp"

#include <string>
#include <vector>

#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/UIString.hpp"

UIScene_EULA::UIScene_EULA(int sceneType, void* callback, UILayer* layer)
    : UIScene(sceneType, layer)
{
    initialiseMovie();
    layer->addComponent(sceneType, (EUIScene)66, 0);
    layer->addComponent(sceneType, (EUIScene)67, 0);
    m_acceptButton.init(UIString(L10N::GetString(0xd8a6b805)), 0);

    std::wstring text(L10N::GetString(0x8ff423f1));
    text.append(L"\r\n");
    std::vector<std::wstring> lines;
    std::wstring::size_type start = 0;
    std::wstring::size_type separator = text.find(L"\r\n", start);
    while (separator != std::wstring::npos)
    {
        lines.push_back(text.substr(start, separator - start) + L" ");
        start = separator + 2;
        separator = text.find(L"\r\n", start);
    }
    lines.push_back(text.substr(start));

    unsigned int i;
    for (i = 0; i < lines.size(); ++i)
        m_eulaText.addText(lines[i], i == lines.size() - 1);

    m_hasFocus = false;
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_EULA::~UIScene_EULA()
{
    m_layer->removeComponent((EUIScene)66);
    m_layer->removeComponent((EUIScene)67);
}

unsigned int UIScene_EULA::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    m_acceptButton.setupControl(this, root, "AcceptButton");
    m_controls.push_back(&m_acceptButton);
    m_eulaText.setupControl(this, root, "EULAtext");
    m_controls.push_back(&m_eulaText);
    return 1;
}

void UIScene_EULA::updateTooltips()
{
    gConsoleUIController.SetTooltips(0, 0x8d6323ed, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, false, false);
}

std::wstring UIScene_EULA::getMoviePath()
{
    return std::wstring(L"EULA");
}

void UIScene_EULA::handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                               bool& handled)
{
    if (!m_hasFocus)
    {
        gConsoleUIController.AnimateKeyPress(playerId, m_controlFocus, repeat, released, pressed);
        if (action > 3 && (action < 6 || (action > 7 && (action < 10 ||
            (action > 13 && (action < 16 || action == 20))))))
            sendInputToMovie(action, repeat, released, pressed);
    }
}

void UIScene_EULA::handlePress(int first, int second)
{
    if (second == 0)
    {
        gConsoleUIController.PlayUISFX(0);
        GameSettings::SetGameSettings(0, (eGameSetting)30, 1);
        gConsoleUIController.NavigateToScene(0, (EUIScene)2, 0, (EUILayer)6, (EUIGroup)7);
    }
}

int UIScene_EULA::getSceneType()
{
    return 77;
}

bool UIScene_EULA::hasFocus(int controller)
{
    return m_hasFocus;
}

unsigned int UIScene_EULA::getDefaultGtcButtons()
{
    return 0;
}
