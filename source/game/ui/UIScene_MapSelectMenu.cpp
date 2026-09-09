#include "game/ui/UIScene_MapSelectMenu.hpp"

#include <algorithm>
#include <stdio.h>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DLCFile.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/cDLCInfo.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"

extern ConsoleUIController gConsoleUIController;

UIScene_MapSelectMenu::UIScene_MapSelectMenu(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer), IUIScene_DlcSelectMenu((eDLCContentType)0),
      m_levelRuleGroups(), m_generationOptions(NULL), m_texturePackSelection(0),
      m_mapInfoPending(0), m_miniGameType(0), m_mapSelection(0), m_mapList(),
      m_mapListTitle(), m_mapTitle(), m_mapDescription(), m_mapIcon()
{
    initialiseMovie();
    m_miniGameType = 0;
    if (gApp.getLevelRules() != NULL)
        gApp.getLevelRules()->getRulesByType((LevelRules::ELevelRuleType)0,
                                               &m_levelRuleGroups, true);
    std::sort(m_levelRuleGroups.begin(), m_levelRuleGroups.end(),
              LevelRules::LevelRulesetGroup::SortMapRules);
    if (m_miniGameType != 1 && m_miniGameType != 3)
        m_mapSelection = 0;
    _init();
}

UIScene_MapSelectMenu::~UIScene_MapSelectMenu()
{
}

unsigned int UIScene_MapSelectMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    m_mapList.setupControl(this, root, std::string("MapList"));
    m_controls.push_back(&m_mapList);
    m_mapListTitle.setupControl(this, root, std::string("MapList_Title"));
    m_controls.push_back(&m_mapListTitle);
    m_mapTitle.setupControl(this, root, std::string("MapTitle"));
    m_controls.push_back(&m_mapTitle);
    m_mapDescription.setupControl(this, root, std::string("HTMLMapText"));
    m_controls.push_back(&m_mapDescription);
    m_mapIcon.setupControl(this, root, std::string("MapPic"));
    m_controls.push_back(&m_mapIcon);
    return 1;
}

void UIScene_MapSelectMenu::_init()
{
    m_mapList.init(0);
    m_mapListTitle.init(UIString(0x88c46bf1));
    m_mapTitle.init(UIString(L" "));
    m_mapDescription.init(UIString(L" "));
    bool anyPurchased = false;
    unsigned int i;
    for (i = 0; i < m_levelRuleGroups.size(); ++i)
    {
        LevelRules::LevelRulesetGroup* group = m_levelRuleGroups[i];
        LevelRuleset* ruleset = group->rulesets.at(0);
        LevelGenerationOptions* options = ruleset->getGenerationOptions();
        bool purchased = checkHasPurchased((int)i, false) != 0;
        anyPurchased = anyPurchased || purchased;
        std::wstring worldName(options->getWorldName());
        std::wstring textureName(L" ");
        m_mapList.AddNewItem(worldName, (int)i, purchased, textureName);
        if (m_miniGameType != 0)
        {
            m_mapList.EnableItem((int)i, false, true);
            m_mapList.SetCheckbox((int)i, purchased);
        }
    }
    if (!anyPurchased && m_mapList.getItemCount() > 3)
    {
        unsigned int count = m_mapList.getItemCount();
        m_mapList.SetCheckbox((int)count - 4, true);
        m_mapList.SetCheckbox((int)count - 3, true);
        m_mapList.SetCheckbox((int)count - 2, true);
        m_mapList.SetCheckbox((int)count - 1, true);
    }
    gConsoleUIController.TouchBoxRebuild(this);
    m_mapInfoPending = 1;
}

std::wstring UIScene_MapSelectMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() < 2 ? L"MapSelectMenu" : L"MapSelectMenuSplit";
}

void UIScene_MapSelectMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(0, 0x8d6323ed, 0x4f6bcf5e, 0xb30cbafb, 0xd96891f0,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     false, false);
    return;
}

void UIScene_MapSelectMenu::updateComponents()
{
    m_layer->showComponent(m_sceneType, (EUIScene)0x43, false);
}

void UIScene_MapSelectMenu::UpdateMapInfo()
{
    unsigned int index = m_mapList.getCurrentSelection();
    LevelRules::LevelRulesetGroup* group = m_levelRuleGroups.at(index);
    m_generationOptions = group->rulesets.at(0)->getGenerationOptions();
    m_mapTitle.setLabel(UIString(m_generationOptions->getWorldName()), false, false);
    m_mapDescription.setLabel(UIString(m_generationOptions->getWorldDescription()), false, false);

    const wchar_t* previewImage = m_generationOptions->getWorldPreviewImage();
    DLCPack* pack = m_generationOptions->getPack();
    DLCFile* file = pack->getFile(DLCManager::eDLCType_Texture, std::wstring(previewImage), true, true);

    if (file == NULL)
    {
        m_mapIcon.setTextureName(std::wstring(L"MapSelect"), true);
        return;
    }

    unsigned int imageSize = 0;
    unsigned char* imageData = file->getData(imageSize);
    if (imageData == NULL || imageSize == 0)
    {
        m_mapIcon.setTextureName(std::wstring(L"MapSelect"), true);
        return;
    }

    wchar_t textureName[64];
    std::swprintf(textureName, 64, L"mapSelect%08x", m_generationOptions->getUniqueId());
    std::wstring name(textureName);
    registerSubstitutionTexture(name, imageData, imageSize, false);
    m_mapIcon.setTextureName(name, true);
}

void UIScene_MapSelectMenu::tick()
{
    UIScene::tick();
    if (m_mapInfoPending != 0)
    {
        m_mapInfoPending = 0;
        UpdateMapInfo();
    }
    IUIScene_DlcSelectMenu::tick();
}

void UIScene_MapSelectMenu::handleReload()
{
    _init();
}

int UIScene_MapSelectMenu::checkHasPurchased(int index, bool tryPurchase)
{
    LevelRules::LevelRulesetGroup* group = m_levelRuleGroups.at(index);
    LevelRuleset* ruleset = group->rulesets.at(0);
    LevelGenerationOptions* options = ruleset->getGenerationOptions();
    DLCPack* pack = options->getPack();
    bool purchased = pack->hasPurchasedPack();
    if (!purchased && gApp.mDLCManager.IsMiniGameTemporarilyFree((int)ruleset->getRuleType()))
        purchased = true;
    if (!purchased && tryPurchase)
    {
        bool handled = false;
        cDLCInfo* dlcInfo = static_cast<cDLCInfo*>((void*)gApp.GetDLCInfoInstance());
        DLC_INFO* info = dlcInfo->GetDLCInfoForPackID(pack->GetPackId());
        int contentType = info == NULL ? 0 : info->ContentType;
        tryPurchaseDlc(m_sceneType, handled, (eDLCContentType)contentType);
    }
    return purchased ? 1 : 0;
}

void UIScene_MapSelectMenu::handleInput(int, int action, bool repeat, bool released, bool pressed,
                                        bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_sceneType, action, repeat, released, pressed);
    if (action == 2 || action == 3)
    {
        if (released || !pressed)
            return;
        unsigned int i;
        for (i = 0; i < m_mapList.getItemCount(); ++i)
            if (checkHasPurchased((int)i, false) != 0)
                m_mapList.SetCheckbox((int)i, action == 2);
        gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
        return;
    }
    if (action == 20 && pressed && !released && checkHasPurchased((int)m_mapList.getCurrentSelection(), true) != 0)
    {
        sendInputToMovie(action, repeat, released, pressed);
        handled = true;
        return;
    }
    if (action == 21 && pressed)
    {
        unsigned int selection = 0;
        unsigned int i;
        for (i = 0; i < m_mapList.getItemCount(); ++i)
            if (m_mapList.GetCheckbox((int)i) != 0)
                selection |= 1U << (m_levelRuleGroups.at(i)->rulesets.at(0)->getGenerationOptions()->getThemeId() & 0x1f);
        m_mapSelection = selection;
        if (m_miniGameType == 1)
            GameSettings::SetBattleMiniGameMapSelection(m_sceneType, selection);
        else if (m_miniGameType == 3)
            GameSettings::SetGlideMiniGameMapSelection(m_sceneType, selection);
        GameSettings::CheckGameSettingsChanged(true, m_sceneType);
        navigateBack();
        handled = true;
        return;
    }
    if (action > 5)
        sendInputToMovie(action, repeat, released, pressed);
}

void UIScene_MapSelectMenu::handleFocusChange(int, int childFocus)
{
    m_mapList.updateChildFocus(childFocus);
    m_mapInfoPending = 1;
    unsigned int selection = m_mapList.getCurrentSelection();
    m_generationOptions = m_levelRuleGroups.at(selection)->rulesets.at(0)->getGenerationOptions();
    m_texturePackSelection = m_generationOptions->getUniqueId();
}

void UIScene_MapSelectMenu::handlePress(int, int)
{
}

int UIScene_MapSelectMenu::getSceneType()
{
    return 0x3f;
}
