#include "game/ui/UIScene_MapSelectMenu.hpp"
#include "game/core/StringIds.hpp"
#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"

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
    : UIScene(sceneType, layer), IUIScene_DlcSelectMenu(eDLCContentType_Unknown),
      m_levelRuleGroups(), m_texturePackSelection(0),
      m_initData(static_cast<LaunchMoreOptionsMenuInitData*>(data)), m_generationOptions(NULL),
      m_mapInfoPending(0), m_mapSelection(0), m_miniGameType(eMiniGameId_Lobby), m_mapList(), m_mapListTitle(),
      m_mapTitle(), m_mapDescription(), m_mapIcon()
{
    initialiseMovie();
    m_playerId = m_initData->mPad;
    m_miniGameType = m_initData->mMiniGameId;
    LevelRules::ELevelRuleType ruleType = LevelRules::GetRuleTypeForMiniGame(m_miniGameType);
    if (m_initData->mMatchmaking && !m_initData->mTimeTrial &&
        !Minecraft::GetInstance()->GetMiniGame()->HasFeatureSeasonalMaps())
        gApp.getLevelRules()->getRulesByType(ruleType, &m_initData->mLevelRulesetGroups, true);
    gApp.getLevelRules()->getRulesByType(ruleType, &m_levelRuleGroups, false);
    std::sort(m_levelRuleGroups.begin(), m_levelRuleGroups.end(),
              LevelRules::LevelRulesetGroup::SortMapRules);
    if (m_miniGameType == eMiniGameId_Battle)
        m_mapSelection = GameSettings::GetBattleMiniGameMapSelection(m_playerId);
    else if (m_miniGameType == eMiniGameId_Glide)
        m_mapSelection = GameSettings::GetGlideMiniGameMapSelection(m_playerId);
    else
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
    m_mapListTitle.init(UIString(IDS_MG_SELECT_MAPS));
    m_mapTitle.init(UIString(L" "));
    m_mapDescription.init(UIString(L" "));
    bool anySelected = false;
    const bool useContextSelection = m_initData->mMatchmaking && !m_initData->mTimeTrial;
    unsigned int i;
    for (i = 0; i < m_levelRuleGroups.size(); ++i)
    {
        LevelRules::LevelRulesetGroup* group = m_levelRuleGroups[i];
        LevelRuleset* ruleset = group->rulesets.at(0);
        LevelGenerationOptions* options = ruleset->getGenerationOptions();

        const unsigned int texturePackId = options->getTexturePackId();
        TexturePack* texturePack =
            Minecraft::GetInstance()->mTexturePackRepository->getTexturePackById(texturePackId);
        std::wstring textureName(L"default");
        if (texturePack != NULL)
        {
            unsigned int iconSize = 0;
            const void* iconData = texturePack->getPackIcon(iconSize);
            if (iconData != NULL && iconSize != 0)
            {
                texturePack->loadUI();
                wchar_t iconName[64];
                std::swprintf(iconName, 64, L"tpack%08x", texturePackId);
                textureName.assign(iconName);
                if (!hasRegisteredSubstitutionTexture(textureName))
                    registerSubstitutionTexture(
                        textureName,
                        reinterpret_cast<unsigned char*>(const_cast<void*>(iconData)), iconSize,
                        false);
            }
        }

        const unsigned int themeId = options->getThemeId();
        const unsigned int themeBit =
            (themeId & 0x20) != 0 ? 0U : (1U << (themeId & 0x1f));
        bool selected = (m_mapSelection & themeBit) != 0;
        if (selected && !checkHasPurchased((int)i, false))
            selected = false;
        if (useContextSelection &&
            std::find(m_initData->mLevelRulesetGroups.begin(),
                      m_initData->mLevelRulesetGroups.end(), group) !=
                m_initData->mLevelRulesetGroups.end())
        {
            selected = true;
            anySelected = true;
        }
        else if (selected)
            anySelected = true;

        std::wstring worldName(options->getWorldName());
        m_mapList.AddNewItem(worldName, (int)i, selected, textureName);
        if (useContextSelection)
        {
            m_mapList.EnableItem((int)i, false, true);
            m_mapList.SetCheckbox((int)i, selected);
        }
    }
    if (!anySelected && m_mapList.getItemCount() > 3)
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
    gConsoleUIController.SetTooltips(0, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, IDS_TOOLTIPS_SELECT_ALL, IDS_TOOLTIPS_UNSELECT_ALL,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     false, false);
    return;
}

void UIScene_MapSelectMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
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
        return;

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
        tryPurchaseDlc(m_playerId, handled, (eDLCContentType)contentType);
    }
    return purchased ? 1 : 0;
}

void UIScene_MapSelectMenu::handleInput(int, int action, bool repeat, bool released, bool pressed,
                                        bool& handled)
{
    if (m_purchasePending)
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    switch (action)
    {
    case EControllerActions_MenuX:
        if (m_initData->mMatchmaking && !m_initData->mTimeTrial)
            return;
        if (!released || repeat)
            return;
        for (unsigned int i = 0; i < m_mapList.getItemCount(); ++i)
            if (checkHasPurchased((int)i, false) != 0)
                m_mapList.SetCheckbox((int)i, true);
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        break;
    case EControllerActions_MenuY:
        if (!released || repeat)
            return;
        if (m_initData->mMatchmaking && !m_initData->mTimeTrial)
            return;
        for (unsigned int i = 0; i < m_mapList.getItemCount(); ++i)
            if (checkHasPurchased((int)i, false) != 0)
                m_mapList.SetCheckbox((int)i, false);
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuOtherStickUp:
    case EControllerActions_MenuOtherStickDown:
        sendInputToMovie(action, repeat, released, pressed);
        break;
    case EControllerActions_MenuOk:
        if (!released || repeat)
            return;
        if (checkHasPurchased((int)m_mapList.getCurrentSelection(), true) == 0)
            return;
        sendInputToMovie(EControllerActions_MenuOk, false, released, pressed);
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        break;
    case EControllerActions_MenuCancel:
    {
        if (!released)
            return;
        unsigned int selection = 0;
        m_initData->mLevelRulesetGroups.clear();
        for (unsigned int i = 0; i < m_mapList.getItemCount(); ++i)
            if (m_mapList.GetCheckbox((int)i) != 0)
            {
                const unsigned int themeId = m_levelRuleGroups.at(i)
                                                 ->rulesets.at(0)
                                                 ->getGenerationOptions()
                                                 ->getThemeId();
                selection |= 1U << themeId;
                m_initData->mLevelRulesetGroups.push_back(m_levelRuleGroups.at(i));
            }
        if (!m_initData->mLevelRulesetGroups.empty())
        {
            if (selection != m_mapSelection)
            {
                if (m_miniGameType == eMiniGameId_Battle)
                    GameSettings::SetBattleMiniGameMapSelection(m_playerId, selection);
                else if (m_miniGameType == eMiniGameId_Glide)
                    GameSettings::SetGlideMiniGameMapSelection(m_playerId, selection);
                m_mapSelection = selection;
            }
            GameSettings::CheckGameSettingsChanged(true, m_playerId);
            navigateBack();
            handled = true;
            return;
        }

        int messageParameter = (int)IDS_CONFIRM_OK;
        m_mapSelection = 0;
        gConsoleUIController.RequestAlertMessage(IDS_MG_NO_MAPS_SELECTED_TITLE, IDS_MG_NO_MAPS_SELECTED_TEXT, &messageParameter, 1,
                                                 m_playerId, NULL, NULL, NULL, true);
        handled = true;
        break;
    }
    }
}

void UIScene_MapSelectMenu::handleFocusChange(int, int childFocus)
{
    m_mapList.updateChildFocus(childFocus);
    m_mapInfoPending = 1;
    unsigned int selection = m_mapList.getCurrentSelection();
    m_generationOptions = m_levelRuleGroups.at(selection)->rulesets.at(0)->getGenerationOptions();
    m_pack = m_generationOptions->getPack();
    m_texturePackSelection = m_generationOptions->getUniqueId();
}

void UIScene_MapSelectMenu::handlePress(int, int)
{
}

int UIScene_MapSelectMenu::getSceneType()
{
    return EUIScene_MapSelectMenu;
}
