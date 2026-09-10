#include "game/ui/UIScene_AchievementsMenu.hpp"

#include <stdint.h>
#include <cwchar>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/FJ_AchievementsMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

struct AchievementInfo
{
    int id;
    const wchar_t* icon;
    int unused;
    int title;
    int description;
};

static const wchar_t* gAchievementIcons[31] = {
    L"MCTrophy_00", L"MCTrophy_01", L"MCTrophy_02", L"MCTrophy_03", L"MCTrophy_04",
    L"MCTrophy_05", L"MCTrophy_06", L"MCTrophy_07", L"MCTrophy_08", L"MCTrophy_09",
    L"MCTrophy_10", L"MCTrophy_11", L"MCTrophy_12", L"MCTrophy_13", L"MCTrophy_14",
    L"MCTrophy_15", L"MCTrophy_16", L"MCTrophy_17", L"MCTrophy_18", L"MCTrophy_19",
    L"", L"", L"MCTrophy_22", L"MCTrophy_23", L"MCTrophy_24", L"",
    L"MCTrophy_33", L"MCTrophy_34", L"MCTrophy_35", L"", L""
};

static AchievementInfo gAchievementInfo[31];
static bool gAchievementInfoInitialised = false;

static void InitialiseAchievementInfo()
{
    if (gAchievementInfoInitialised)
        return;
    for (int i = 0; i < 31; ++i) {
        gAchievementInfo[i].id = i;
        gAchievementInfo[i].icon = gAchievementIcons[i];
        gAchievementInfo[i].unused = i + 1;
        gAchievementInfo[i].title = 0;
        gAchievementInfo[i].description = 0;
    }
    gAchievementInfoInitialised = true;
}

static AchievementInfo* Achievement(int index)
{
    return &gAchievementInfo[index];
}

static void AddAchievement(UIControl_AchievementsList& list, int index)
{
    AchievementInfo* info = Achievement(index);
    list.AddNewItem(index, info->icon);
    list.EnableItem(index, true, true);
}

UIScene_AchievementsMenu::UIScene_AchievementsMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer), m_achievementIndex(0), m_showDescription(false), m_updateDetails(false),
      m_mainPanel(), m_achievementsList(), m_achievementName(), m_achievementDescription()
{
    InitialiseAchievementInfo();
    initialiseMovie();
    m_achievementsList.init(0);
    int selected = (int)(uintptr_t)data;
    for (int i = 0; i < 31; ++i) {
        registerTexture(i);
        AddAchievement(m_achievementsList, i);
    }
    m_achievementName.init(UIString(0x31dacd58));
    m_achievementDescription.init(UIString(L"CommandBlockMenuSplit"));
    m_achievementsList.setCurrentSelection(selected, true);
    updateTooltips();
    UpdateAchievementDetails();
}

UIScene_AchievementsMenu::~UIScene_AchievementsMenu()
{
}

int UIScene_AchievementsMenu::GetInfo(int achievement)
{
    InitialiseAchievementInfo();
    return (int)(uintptr_t)Achievement(achievement);
}

void UIScene_AchievementsMenu::registerTexture(int achievement)
{
    std::wstring name;
    wchar_t path[64];
    swprintf(path, 64, L"Graphics\\Achievements\\%ls.png", Achievement(achievement)->icon);
    name.assign(path);
    if (!g_pApp->hasArchiveFile(name, false))
        return;
    arrayWithLength<unsigned char> file = g_pApp->getArchiveFile(name, false);
    if (file.data != NULL && file.length != 0)
        registerSubstitutionTexture(name, file.data, file.length, true);
}

void UIScene_AchievementsMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(static_cast<unsigned int>(m_playerId), -1, 0xe21d8023, 0xae01f627,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_AchievementsMenu::SetAchievementDescription(std::vector<HtmlString>* lines)
{
    std::wstring description = HtmlString::Compose(lines);
    fuiRenderNode* root = m_movie == NULL ? NULL : m_movie->getRootNode();
    FJ_AchievementsMenu* document = root == NULL ? NULL : static_cast<FJ_AchievementsMenu*>(root->getFuiNode()->asFJ_Document());
    if (document != NULL)
        document->SetAchievementDescription(description);
}

void UIScene_AchievementsMenu::UpdateAchievementDetails()
{
    AchievementInfo* info = Achievement(m_achievementIndex);
    m_achievementName.setLabel(UIString(L10N::GetString(info->title)), false, false);
    std::vector<HtmlString> lines;
    if (m_showDescription)
        lines.push_back(HtmlString(L10N::GetString(info->description), static_cast<eMinecraftColour>(437), false, false, 0));
    SetAchievementDescription(&lines);
}

unsigned int UIScene_AchievementsMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie == NULL ? NULL : m_movie->getRootNode();
    m_achievementDescription.setupControl(this, root, "AcheivementsLabel");
    m_achievementName.setupControl(this, root, "AchievementName");
    m_mainPanel.setupControl(this, root, "AchievementsListContainer");
    m_achievementsList.setupControl(this, m_mainPanel.getFuiRenderNode(), "AchievementsList");
    return 1;
}

int UIScene_AchievementsMenu::getSceneType()
{
    return 0x51;
}

void UIScene_AchievementsMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x43), false);
}

std::wstring UIScene_AchievementsMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? L"AchievementsMenu" : L"AchievementsMenuSplit";
}

void UIScene_AchievementsMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                           bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (pressed) {
        if (action == 2) {
            m_updateDetails = true;
            m_showDescription = !m_showDescription;
        } else if (action > 3 && action < 10) {
            sendInputToMovie(action, repeat, released, pressed);
        } else if (action == 0x15) {
            navigateBack();
        }
    }
    handled = true;
}

void UIScene_AchievementsMenu::handleGainFocus(bool hadFocus)
{
    m_updateDetails = true;
}

void UIScene_AchievementsMenu::handleFocusChange(int focus, int childFocus)
{
    m_achievementIndex = childFocus;
    m_updateDetails = true;
}

UIControl* UIScene_AchievementsMenu::GetMainPanel()
{
    return &m_mainPanel;
}

void UIScene_AchievementsMenu::handleReload()
{
    m_achievementsList.init(0);
    for (int i = 0; i < 31; ++i) {
        registerTexture(i);
        AddAchievement(m_achievementsList, i);
    }
    m_achievementName.init(UIString(0x31dacd58));
    m_achievementDescription.init(UIString(L"CommandBlockMenuSplit"));
    UpdateAchievementDetails();
}

void UIScene_AchievementsMenu::tick()
{
    UIScene::tick();
    if (m_updateDetails) {
        UpdateAchievementDetails();
        m_updateDetails = false;
    }
}
