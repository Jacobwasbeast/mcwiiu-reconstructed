#include "game/ui/UIScene_EndPoem.hpp"

#include <cwchar>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/FJ_EndPoem.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/util/Random.hpp"

extern ConsoleUIController gConsoleUIController;

std::wstring escapeXML(const std::wstring& text);
void replaceAll(std::wstring& result, const std::wstring& source,
                const std::wstring& search, const std::wstring& replacement);

UIScene_EndPoem::UIScene_EndPoem(int type, void*, UILayer* layer)
    : UIScene(type, layer), m_text(), m_noiseLengths(), m_finished(false), m_currentLabel(0), m_labels()
{
    initialiseMovie();

    m_text += L10N::GetString(0x47630c46);
    m_text += L10N::GetString(0x69414293);
    m_text += L10N::GetString(0x433ec82a);
    m_text += L" ";
    gApp.FormatHTMLString(m_resolution, m_text, static_cast<int>(0xff000000));

    Minecraft* minecraft = Minecraft::GetInstance();
    std::wstring playerName;
    unsigned int userIndex = gConsoleUIController.GetWinUserIndex();
    if (minecraft != NULL && userIndex < 4 && minecraft->mLocalPlayers[userIndex])
    {
        playerName = escapeXML(minecraft->mLocalPlayers[userIndex]->getName());
    }
    replaceAll(m_text, m_text, L"{*PLAYER*}", playerName);

    Random random(0x7bf7d3LL);
    std::wstring::size_type position = m_text.find(L"{*NOISE*}");
    while (position != std::wstring::npos)
    {
        m_noiseLengths.push_back(random.nextInt(4) + 3);
        position = m_text.find(L"{*NOISE*}", position + 1);
    }

    updateNoise();

    std::wstring::size_type start = 0;
    position = m_text.find(L"<br /><br />", start);
    while (position != std::wstring::npos)
    {
        m_labels.push_back(m_text.substr(start, position - start));
        start = position + 12;
        position = m_text.find(L"<br /><br />", start);
    }
    m_labels.push_back(m_text.substr(start));
    m_currentLabel = 0;
}

UIScene_EndPoem::~UIScene_EndPoem()
{
}

void UIScene_EndPoem::updateNoise()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    Random random(0x7bf7d3LL);
    std::wstring noise = L"{*NOISE*}";
    std::wstring::size_type position = m_text.find(noise);
    std::vector<int>::const_iterator count = m_noiseLengths.begin();
    const wchar_t* characters = L"!\"#$%&'()*+,-./0123456789:;<=>?@";

    while (position != std::wstring::npos && count != m_noiseLengths.end())
    {
        std::wstring replacement;
        int length = *count++;
        int i;
        for (i = 0; i < length; ++i)
        {
            wchar_t character = characters[random.nextInt(static_cast<int>(std::wcslen(characters)))];
            if (character == L'<') replacement += L"&lt;";
            else if (character == L'>') replacement += L"&gt;";
            else if (character == L'\"') replacement += L"&quot;";
            else if (character == L'&') replacement += L"&amp;";
            else if (character == L'\\') replacement += L"\\\\";
            else if (character == L'{') replacement += L"}";
            else replacement += character;

            int colour = 0x199;
            int colourChoice = random.nextInt(2);
            if (colourChoice == 1) colour = 0x19a;
            else if (colourChoice == 2) colour = 0x19b;
            wchar_t colourText[64];
            std::swprintf(colourText, 64, L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
                          gApp.GetHTMLColour(static_cast<eMinecraftColour>(colour)),
                          gApp.GetHTMLColour(static_cast<eMinecraftColour>(colour)));
            replacement += colourText;
        }
        m_text.replace(position, noise.size(), replacement);
        position = m_text.find(noise, position + replacement.size());
    }
}

void UIScene_EndPoem::updateTooltips()
{
    int tooltip = m_finished ? -1 : static_cast<int>(0x8ad40344);
    gConsoleUIController.SetTooltips(m_resolution, -1, tooltip, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, true);
}

std::wstring UIScene_EndPoem::getMoviePath()
{
    return L"EndPoem";
}

void UIScene_EndPoem::tick()
{
    UIScene::tick();
    if (m_currentLabel >= 0 && m_currentLabel < static_cast<int>(m_labels.size()))
    {
        std::wstring label = m_labels[m_currentLabel];
        if (m_movie != NULL && m_movie->getRootNode() != NULL)
        {
            FJ_EndPoem* poem = static_cast<FJ_EndPoem*>(
                m_movie->getRootNode()->getFuiNode()->asFJ_Document());
            if (poem != NULL)
            {
                poem->SetNextLabel(label, m_currentLabel,
                                   m_currentLabel == static_cast<int>(m_labels.size()) - 1);
                m_currentLabel = -1;
            }
        }
    }
}

void UIScene_EndPoem::handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                                  bool& handled)
{
    if (m_finished) return;
    if (pressed)
    {
        gConsoleUIController.AnimateKeyPress(playerId, action, repeat, released, pressed);
    }
    if (action > 3)
    {
        if (action < 14) return;
        if (action > 15)
        {
            if (action != 0x15 || !pressed) return;
            m_finished = true;
            Minecraft* minecraft = Minecraft::GetInstance();
            if (minecraft != NULL)
            {
                int i;
                for (i = 0; i < 4; ++i)
                {
                    if (minecraft->mLocalPlayers[i])
                    {
                        gApp.SetAction(i, static_cast<eXuiAction>(6), NULL);
                    }
                }
                if (minecraft->mGameMode != NULL && minecraft->mGameMode->getTutorial() != NULL)
                {
                    minecraft->mGameMode->getTutorial()->showTutorialPopup(true);
                }
            }
            updateTooltips();
            navigateBack();
            handled = true;
            return;
        }
        sendInputToMovie(action, repeat, released, pressed);
    }
}

void UIScene_EndPoem::handleDestroy()
{
}

void UIScene_EndPoem::handleRequestMoreData(int id, bool)
{
    m_currentLabel = id;
}

int UIScene_EndPoem::getSceneType()
{
    return 0x30;
}
