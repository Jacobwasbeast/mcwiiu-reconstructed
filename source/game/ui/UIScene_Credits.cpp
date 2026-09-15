#include "game/ui/UIScene_Credits.hpp"

#include <cwchar>

#include "game/core/ConsoleUIController.hpp"
#include "game/ui/FJ_CreditsMenu.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

extern ConsoleUIController gConsoleUIController;
extern unsigned int XGetLanguage();

UIScene_Credits::UIScene_Credits(int type, void*, UILayer* layer)
    : UIScene(type, layer), m_currentCredit(-1), m_creditCount(0), m_requestMoreData(false),
      m_dlcCredits(), m_seenCreditText(), m_unknownF0(0)
{
    initialiseMovie();
    m_creditCount = 0x220;
    m_currentCredit = -1;
    m_requestMoreData = false;
}

UIScene_Credits::~UIScene_Credits()
{
    std::vector<SCreditTextItemDef*>::iterator it = m_dlcCredits.begin();
    for (; it != m_dlcCredits.end(); ++it)
    {
        if (*it != NULL)
        {
            delete[] (*it)->text;
            delete *it;
        }
    }
    m_dlcCredits.clear();
}

bool UIScene_Credits::AlreadySeenCreditText(const std::wstring& text)
{
    std::vector<std::wstring>::const_iterator it = m_seenCreditText.begin();
    for (; it != m_seenCreditText.end(); ++it)
    {
        if (*it == text) return true;
    }
    m_seenCreditText.push_back(text);
    return false;
}

void UIScene_Credits::AddCreditText(const wchar_t* text)
{
    SCreditTextItemDef* item = new SCreditTextItemDef;
    item->text = new wchar_t[std::wcslen(text) + 1];
    std::wcscpy(item->text, text);
    item->primaryId = -1;
    item->secondaryId = -1;
    item->type = 3;
    m_dlcCredits.push_back(item);
}

void UIScene_Credits::addCreditLine(const std::wstring& text)
{
    if (AlreadySeenCreditText(text)) return;

    AddCreditText(L"");
    bool useShortLine = false;
    if (gConsoleUIController.IsReloadingSkin() == false &&
        gConsoleUIController.IsExpectingOrReloadingSkin() == false)
    {
        useShortLine = true;
    }
    unsigned int maximum = (XGetLanguage() == 2 ||
                            (XGetLanguage() > 6 && XGetLanguage() < 9) ||
                            !useShortLine) ? 55 : 45;
    std::wstring remaining = text;
    while (remaining.size() > maximum)
    {
        std::wstring::size_type split = remaining.find_last_of(L" ", maximum);
        if (split == std::wstring::npos) split = maximum;
        if (split != maximum) ++split;
        AddCreditText(remaining.substr(0, split).c_str());
        remaining = remaining.substr(split);
    }
    AddCreditText(remaining.c_str());
}

int UIScene_Credits::GetDLCCreditsCount()
{
    if (m_dlcCredits.empty()) return 0;
    return static_cast<int>(m_dlcCredits.size());
}

void UIScene_Credits::setNextLabel(const std::wstring& text, ECreditTextTypes type)
{
    if (m_movie == NULL || m_movie->getRootNode() == NULL) return;
    FJ_CreditsMenu* menu = static_cast<FJ_CreditsMenu*>(
        m_movie->getRootNode()->getFuiNode()->asFJ_Document());
    if (menu != NULL)
    {
        menu->SetNextLabel(text, static_cast<int>(type),
                           m_currentCredit == m_creditCount - 1);
    }
}

void UIScene_Credits::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, -1, 0x4f6bcf5e, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_Credits::updateComponents()
{
    if (m_layer != NULL) m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x43), true);
}

void UIScene_Credits::handleReload()
{
    navigateBack();
}

SCreditTextItemDef* UIScene_Credits::GetDLCCredits(int index)
{
    return m_dlcCredits.at(static_cast<std::size_t>(index));
}

void UIScene_Credits::addImage(ECreditIcons icon)
{
    if (m_movie == NULL || m_movie->getRootNode() == NULL) return;
    FJ_CreditsMenu* menu = static_cast<FJ_CreditsMenu*>(
        m_movie->getRootNode()->getFuiNode()->asFJ_Document());
    if (menu != NULL) menu->AddImage(static_cast<int>(icon), m_currentCredit == m_creditCount - 1);
}

void UIScene_Credits::tick()
{
    UIScene::tick();
    if (!m_requestMoreData) return;
    m_requestMoreData = false;
    ++m_currentCredit;
    if (m_currentCredit >= m_creditCount) m_currentCredit = 0;
    if (m_currentCredit < GetDLCCreditsCount())
    {
        SCreditTextItemDef* item = GetDLCCredits(m_currentCredit);
        if (item != NULL && item->text != NULL)
        {
            setNextLabel(std::wstring(item->text), static_cast<ECreditTextTypes>(item->type));
        }
    }
}

std::wstring UIScene_Credits::getMoviePath()
{
    return L"Credits";
}

void UIScene_Credits::handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                                  bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (action > 3 && (action < 6 || action == 20))
    {
        sendInputToMovie(action, repeat, released, pressed);
    }
    else if (action == 21 && pressed && !released)
    {
        navigateBack();
    }
}

void UIScene_Credits::handleRequestMoreData(int, bool)
{
    m_requestMoreData = true;
}

int UIScene_Credits::getSceneType()
{
    return 23;
}

void UIScene_Credits::setLogoLanguage(int language)
{
    if (m_movie == NULL || m_movie->getRootNode() == NULL) return;
    FJ_CreditsMenu* menu = static_cast<FJ_CreditsMenu*>(
        m_movie->getRootNode()->getFuiNode()->asFJ_Document());
    if (menu != NULL) menu->SetLogoLanguage(language);
}
