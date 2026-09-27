#include "game/ui/UIScene_LanguageSelector.hpp"
#include "game/core/StringIds.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"

UIScene_LanguageSelector::UIScene_LanguageSelector(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer)
{
    initialiseMovie();
    m_languageList.init(0);
    m_languageList.addItem(UIString(0x4A55D76A), 0);
    m_languageList.addItem(UIString(0x85CABB8D), 1);
    m_languageList.addItem(UIString(0xA7BED7AD), 2);
    m_languageList.addItem(UIString(0x927530AB), 3);
    m_languageList.addItem(UIString(0xB524803B), 4);
    m_languageList.addItem(UIString(0x1C9D5CE9), 5);
    m_languageList.addItem(UIString(0xAF947AA3), 6);
    m_languageList.addItem(UIString(0xFD7B9B78), 7);
    m_languageList.addItem(UIString(0x8DC52CD3), 8);
    m_languageList.addItem(UIString(0xBEACA054), 9);
    m_languageList.addItem(UIString(0x4F5B792B), 10);
    m_languageList.addItem(UIString(0xD1E5AF59), 11);
    m_languageList.addItem(UIString(0x21828946), 12);
}

UIScene_LanguageSelector::~UIScene_LanguageSelector()
{
}

void UIScene_LanguageSelector::updateTooltips()
{
    gConsoleUIController.SetTooltips(static_cast<unsigned int>(m_playerId), IDS_TOOLTIPS_SELECT,
                                     IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, false, false);
}

void UIScene_LanguageSelector::updateComponents()
{
    bool showSplitScreen = false;
    if (Minecraft::GetInstance()->mLevel == NULL)
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
        showSplitScreen = true;
    }
    else
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, false);
        if (g_pApp->GetLocalPlayerCount() == 1)
            showSplitScreen = true;
    }
    m_layer->showComponent(m_playerId, EUIScene_Logo, showSplitScreen);
}

void UIScene_LanguageSelector::handleReload()
{
    m_languageList.addItem(UIString(0x4A55D76A), 0);
    m_languageList.addItem(UIString(0x85CABB8D), 1);
    m_languageList.addItem(UIString(0xA7BED7AD), 2);
    m_languageList.addItem(UIString(0x927530AB), 3);
    m_languageList.addItem(UIString(0xB524803B), 4);
    m_languageList.addItem(UIString(0x1C9D5CE9), 5);
    m_languageList.addItem(UIString(0xAF947AA3), 6);
    m_languageList.addItem(UIString(0xFD7B9B78), 7);
    m_languageList.addItem(UIString(0x8DC52CD3), 8);
    m_languageList.addItem(UIString(0xBEACA054), 9);
    m_languageList.addItem(UIString(0x4F5B792B), 10);
    m_languageList.addItem(UIString(0xD1E5AF59), 11);
    m_languageList.addItem(UIString(0x21828946), 12);
}

std::wstring UIScene_LanguageSelector::getMoviePath()
{
    if (g_pApp->GetLocalPlayerCount() < 2)
        return std::wstring(L"LanguagesMenu");
    return std::wstring(L"LanguagesMenuSplit");
}

void UIScene_LanguageSelector::handleInput(int, int action, bool repeat, bool released, bool pressed,
                                           bool&)
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

void UIScene_LanguageSelector::handlePress(int controlId, int childId)
{
    static const unsigned char languages[] = {0, 1, 3, 5, 4, 6, 9, 2, 7, 8, 17, 16, 12};
    static const unsigned int locales[] = {0, 0, 0, 31, 0, 0, 0, 27, 0, 0, 0, 0, 0};

    if (controlId == 0)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        GameSettings::SetMinecraftLanguage(m_playerId, languages[childId]);
        GameSettings::SetMinecraftLocale(m_playerId, static_cast<unsigned char>(locales[childId]));
        GameSettings::CheckGameSettingsChanged(true, m_playerId);
    }
}

unsigned int UIScene_LanguageSelector::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    std::string name("HowToList");
    m_languageList.setupControl(this, root, name);
    m_controls.push_back(&m_languageList);
    return 1;
}

int UIScene_LanguageSelector::getSceneType()
{
    return EUIScene_LanguageSelector;
}
