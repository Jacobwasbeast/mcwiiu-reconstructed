#include "game/ui/UIScene_NewUpdateMessage.hpp"

#include <string>
#include <vector>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"

UIScene_NewUpdateMessage::UIScene_NewUpdateMessage(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer)
{
    initialiseMovie();
    layer->addComponent(sceneType, static_cast<EUIScene>(0x42), NULL);
    layer->addComponent(sceneType, static_cast<EUIScene>(0x43), NULL);
    m_acceptButton.init(UIString(L10N::GetString(static_cast<int>(0xd8a6b805))), 0);

    std::wstring text(L10N::GetString(static_cast<int>(0x31bf39b5)));
    text += L"\r\n";
    text = gApp.FormatHTMLString(m_playerId, text, -1);

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
    for (unsigned int i = 0; i < lines.size(); ++i)
        m_eulaText.addText(lines[i], i == lines.size() - 1);

    m_ignoreInput = false;
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_NewUpdateMessage::~UIScene_NewUpdateMessage()
{
    if (m_layer != NULL)
    {
        m_layer->removeComponent(static_cast<EUIScene>(0x42));
        m_layer->removeComponent(static_cast<EUIScene>(0x43));
    }
}

unsigned int UIScene_NewUpdateMessage::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    m_acceptButton.setupControl(this, root, std::string("AcceptButton"));
    m_controls.push_back(&m_acceptButton);
    m_eulaText.setupControl(this, root, std::string("EULAtext"));
    m_controls.push_back(&m_eulaText);
    return 1;
}

void UIScene_NewUpdateMessage::updateTooltips()
{
    gConsoleUIController.SetTooltips(0, 0x8d6323ed, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

std::wstring UIScene_NewUpdateMessage::getMoviePath()
{
    return L"EULA";
}

void UIScene_NewUpdateMessage::handleInput(int, int action, bool repeat, bool released,
                                           bool pressed, bool&)
{
    if (!m_ignoreInput)
    {
        gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
        if (action < 8)
        {
            if (action == 1)
            {
                unsigned int value = GameSettings::GetGameSettings(
                    m_playerId, static_cast<eGameSetting>(0x16));
                if (value > 0)
                    --value;
                GameSettings::SetGameSettings(m_playerId, static_cast<eGameSetting>(0x16),
                                              static_cast<unsigned char>(value));
                GameSettings::SetGameSettings(m_playerId, static_cast<eGameSetting>(0x25), 1);
                GameSettings::CheckGameSettingsChanged(true, m_playerId);
                gConsoleUIController.NavigateBack(m_playerId, false, static_cast<EUIScene>(0),
                                                   static_cast<EUILayer>(0x53));
                return;
            }
            if (action < 4 || action > 5)
                return;
        }
        else if (action > 9 && action < 0xe)
            return;
        else if (action > 0xf && action != 0x14)
            return;
        sendInputToMovie(action, repeat, released, pressed);
    }
}

void UIScene_NewUpdateMessage::handlePress(int, int second)
{
    if (second == 0)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
        unsigned int value = GameSettings::GetGameSettings(
            m_playerId, static_cast<eGameSetting>(0x16));
        if (value > 0)
            --value;
        GameSettings::SetGameSettings(m_playerId, static_cast<eGameSetting>(0x16),
                                      static_cast<unsigned char>(value));
        GameSettings::CheckGameSettingsChanged(true, m_playerId);
        gConsoleUIController.NavigateBack(m_playerId, false, static_cast<EUIScene>(0),
                                           static_cast<EUILayer>(0x53));
    }
}

int UIScene_NewUpdateMessage::getSceneType()
{
    return 77;
}

bool UIScene_NewUpdateMessage::hasFocus(int)
{
    return m_hasFocus;
}

unsigned int UIScene_NewUpdateMessage::getDefaultGtcButtons()
{
    return 0;
}
