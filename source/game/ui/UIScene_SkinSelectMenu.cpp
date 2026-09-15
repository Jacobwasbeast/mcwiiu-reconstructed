#include "game/ui/UIScene_SkinSelectMenu.hpp"

#include <algorithm>
#include <cstdio>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/DLCSkinFile.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/cDLCInfo.hpp"
#include "game/ui/FJ_SkinSelect.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

extern CConsoleMinecraftApp gApp;
extern CMinecraftApp* g_pApp;
extern ConsoleUIController gConsoleUIController;

UIScene_SkinSelectMenu::UIScene_SkinSelectMenu(int player, void* data, UILayer* layer)
    : UIScene(player, layer), m_dlcScene(static_cast<eDLCContentType>(1)), m_activePackId(0),
      m_packTypeId(0), m_skinIndex(0), m_selectedSkinId(0), m_skinBoxes(0),
      m_characterFacing(false), m_skinChanging(false), m_favouriteChanged(false),
      m_focusChanged(false), m_packChanged(true), m_dlcChanged(false),
      m_hasSelectedPack(false), m_highlightedPackId(0), m_inputPending(false)
{
    m_skinIndex = static_cast<int>(GameSettings::GetPlayerSkinId(player));
    m_selectedSkinId = m_skinIndex;
    m_skinName = GameSettings::GetPlayerSkinName(player);
    m_defaultSkinName = L"default";
    m_touch.init(10);
    m_skinButtonList.init(0);
    for (int i = 0; i < 7; ++i)
        m_characterPreviews[i].SetFacing(UIControl_PlayerSkinPreview::eFacingFront, false);
    m_packChanged = true;
}

UIScene_SkinSelectMenu::~UIScene_SkinSelectMenu()
{
}

unsigned int UIScene_SkinSelectMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();

    m_touch.setupControl(this, root, std::string("TouchIggyCharacters"));
    m_controls.push_back(&m_touch);

    m_packName.setupControl(this, root, std::string("Pack_Name"));
    m_controls.push_back(&m_packName);

    m_packType.setupControl(this, root, std::string("Pack_Type"));
    m_controls.push_back(&m_packType);

    m_skinTitle1.setupControl(this, root, std::string("SkinTitle1"));
    m_controls.push_back(&m_skinTitle1);

    m_skinTitle2.setupControl(this, root, std::string("SkinTitle2"));
    m_controls.push_back(&m_skinTitle2);

    m_timer.setupControl(this, root, std::string("Timer"));
    m_controls.push_back(&m_timer);

    m_characterPanel.setupControl(this, root, std::string("IggyCharacters"));
    m_controls.push_back(&m_characterPanel);

    fuiRenderNode* panelNode = m_characterPanel.getFuiRenderNode();
    static const char* const characterNames[7] = {
        "iggy_Character0", "iggy_Character1", "iggy_Character2", "iggy_Character3",
        "iggy_Character4", "iggy_Character5", "iggy_Character6"
    };
    for (int i = 0; i < 7; ++i)
    {
        m_characterPreviews[i].setupControl(this, panelNode, std::string(characterNames[i]));
        m_controls.push_back(&m_characterPreviews[i]);
    }

    m_packIcon.setupControl(this, root, std::string("TexturePackIcon"));
    m_controls.push_back(&m_packIcon);

    m_skinButtonList.setupControl(this, root, std::string("SkinButtonList"));
    m_controls.push_back(&m_skinButtonList);

    return 1;
}

void UIScene_SkinSelectMenu::setCharacterLocked(bool locked)
{
    FJ_SkinSelect* skin = static_cast<FJ_SkinSelect*>(m_movie->getRootNode()->getFuiNode());
    skin->SetCharacterLocked(locked);
}

void UIScene_SkinSelectMenu::setCharacterSelected(bool selected)
{
    FJ_SkinSelect* skin = static_cast<FJ_SkinSelect*>(m_movie->getRootNode()->getFuiNode());
    skin->SetPlayerCharacterSelected(selected, false);
}

void UIScene_SkinSelectMenu::setCharacterFavourite(bool favourite)
{
    FJ_SkinSelect* skin = static_cast<FJ_SkinSelect*>(m_movie->getRootNode()->getFuiNode());
    skin->SetCharacterFavourite(favourite);
}

void UIScene_SkinSelectMenu::setCharacterBlocked(bool blocked)
{
    FJ_SkinSelect* skin = static_cast<FJ_SkinSelect*>(m_movie->getRootNode()->getFuiNode());
    skin->SetCharacterBlocked(blocked);
}

void UIScene_SkinSelectMenu::setActivePackIndex()
{
    m_activePackId = 0;
    m_hasSelectedPack = false;
    if (gApp.mDLCManager.getPackCount(DLCManager::eDLCType_Skin) != 0)
    {
        DLCPack* pack = gApp.mDLCManager.getPackContainingSkin(m_skinName);
        if (pack != 0)
            m_activePackId = static_cast<int>(pack->GetPackId());

        const unsigned int count = GameSettings::GetPlayerFavoriteSkinsCount(m_playerId);
        for (unsigned int i = 0; i < count; ++i)
        {
            const unsigned int skin = GameSettings::GetPlayerFavoriteSkin(m_playerId, i);
            bool isDefault = false;
            bool hasPurchased = false;
            DLCFile* file = gApp.mDLCManager.getSkinFile(static_cast<int>(skin), isDefault,
                                                         hasPurchased);
            if (file == 0)
                continue;
            if (file->m_path == m_skinName)
            {
                m_activePackId = 0;
                m_hasSelectedPack = true;
                break;
            }
        }
    }
}

unsigned int UIScene_SkinSelectMenu::registerTexture(std::wstring textureName)
{
    std::wstring path = L"Graphics_PackGraphics_" + textureName + L".png";
    if (!g_pApp->hasArchiveFile(path, false))
        return 0;
    arrayWithLength<unsigned char> file = g_pApp->getArchiveFile(path, false);
    if (file.data == 0 || file.length == 0)
        return 0;
    registerSubstitutionTexture(textureName, file.data, file.length, true);
    return 1;
}

unsigned char UIScene_SkinSelectMenu::getTextureId(int index)
{
    if (index > 0x12)
        return 0x25;
    static const unsigned char textureIds[19] = {
        0x25, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0xaa, 0xa9
    };
    return textureIds[index];
}

unsigned int UIScene_SkinSelectMenu::getNextSkinIndex(unsigned int index)
{
    unsigned int count;
    if (m_packTypeId == 1)
        count = GameSettings::GetPlayerFavoriteSkinsCount(m_playerId);
    else if (m_activePackId != 0)
    {
        DLCPack* pack = gApp.mDLCManager.getPackFromPackID(m_activePackId);
        count = pack == 0 ? 0 : static_cast<unsigned int>(pack->getSkinCount());
    }
    else
        count = 0x13;
    return index + 1 < count ? index + 1 : 0;
}

int UIScene_SkinSelectMenu::getPreviousSkinIndex(unsigned int index)
{
    if (index != 0)
        return static_cast<int>(index - 1);
    if (m_packTypeId == 1)
        return GameSettings::GetPlayerFavoriteSkinsCount(m_playerId) - 1;
    if (m_activePackId != 0)
    {
        DLCPack* pack = gApp.mDLCManager.getPackFromPackID(m_activePackId);
        return pack == 0 ? 0 : pack->getSkinCount() - 1;
    }
    return 0x12;
}

void UIScene_SkinSelectMenu::handleSkinIndexChanged()
{
    m_skinChanging = false;
    m_favouriteChanged = false;
    setCharacterSelected(false);
    m_skinBoxes = 0;
    m_skinTitle1.init(UIString(L""));
    m_skinTitle2.init(UIString(L""));

    if (m_packTypeId == 0)
    {
        m_selectedSkinId = m_skinIndex;
        m_defaultSkinName = m_skinIndex == 0 ? L"default" : L"";
        setCharacterLocked(false);
        setCharacterBlocked(false);
        m_characterPanel.setVisible(true);
        m_packIcon.setTextureName(std::wstring(), false);
        return;
    }

    if (m_packTypeId == 1)
    {
        const unsigned int count = GameSettings::GetPlayerFavoriteSkinsCount(m_playerId);
        if (count == 0)
        {
            m_characterPanel.setVisible(false);
            m_hasSelectedPack = true;
            return;
        }
        m_selectedSkinId = static_cast<int>(GameSettings::GetPlayerFavoriteSkin(
            m_playerId, static_cast<unsigned int>(m_skinIndex)));
        m_favouriteChanged = IsFavoriteSkin(m_playerId, m_selectedSkinId) != 0;
        m_characterPanel.setVisible(true);
        setCharacterSelected(true);
        setCharacterLocked(false);
        setCharacterBlocked(false);
        return;
    }

    DLCPack* pack = gApp.mDLCManager.getPackFromPackID(m_activePackId);
    if (pack == 0)
    {
        setCharacterLocked(false);
        setCharacterBlocked(false);
        return;
    }
    m_selectedSkinId = m_skinIndex;
    DLCSkinFile* file = static_cast<DLCSkinFile*>(pack->getSkinFile(
        static_cast<unsigned int>(m_skinIndex)));
    if (file == 0)
    {
        setCharacterSelected(false);
        setCharacterLocked(false);
        setCharacterBlocked(false);
        return;
    }
    m_skinName = file->m_path;
    m_skinBoxes = file->getAdditionalBoxes();
    const bool selected = m_skinName == GameSettings::GetPlayerSkinName(m_playerId);
    const bool blocked = !file->isFree() && !pack->hasPurchasedPack();
    setCharacterSelected(selected);
    setCharacterLocked(!selected && !blocked);
    setCharacterBlocked(blocked);
    m_characterPanel.setVisible(true);
}

unsigned int UIScene_SkinSelectMenu::IsTrialDiscontinuedPack(int packId)
{
    if (packId <= 1)
        return 0;
    DLCPack* pack = gApp.mDLCManager.getPackFromPackID(packId);
    cDLCInfo* dlcInfo = static_cast<cDLCInfo*>((void*)gApp.GetDLCInfoInstance());
    DLC_INFO* info = dlcInfo->GetDLCInfoForPackID(static_cast<unsigned int>(packId));
    if (info != 0 && info->PackType == 4 &&
        GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(2), -1) == 0)
        return pack != 0 && !pack->hasPurchasedPack() ? 1U : 0U;
    return 0;
}

unsigned int UIScene_SkinSelectMenu::GetIsNewPack(int packId)
{
    return std::find(m_newPackIds.begin(), m_newPackIds.end(), packId) != m_newPackIds.end();
}

bool UIScene_SkinSelectMenu::SORTNEWEST(int first, int second)
{
    static const int kNewestPackOrder[12] = { 0x22C, 0x224, 0x233, 0x222, 0x216, 0x225,
                                              0x234, 0x207, 0x227, 0x4F9, 0x23C, 0x40E };
    int firstPos = 0;
    int secondPos = 0;
    for (int i = 0; i < 12; ++i)
    {
        if (kNewestPackOrder[i] == first)
            firstPos = i;
        if (kNewestPackOrder[i] == second)
            secondPos = i;
    }
    return firstPos < secondPos;
}

bool UIScene_SkinSelectMenu::SORTALPHABETICALLY(int first, int second)
{
    DLCPack* a = gApp.mDLCManager.getPackFromPackID(first);
    DLCPack* b = gApp.mDLCManager.getPackFromPackID(second);
    if (a == 0 || b == 0)
        return first < second;
    return a->getLocalisedDisplayName() < b->getLocalisedDisplayName();
}

void UIScene_SkinSelectMenu::SetSkinPackButtonList()
{
    if (!m_dlcChanged)
        return;
    m_dlcChanged = false;
    m_packIds.clear();
    m_newPackIds.clear();
    m_selectedPackIds.clear();
    m_skinButtonList.clearList();
    m_skinButtonList.AddNewButton(L10N::GetString(0xa6517d6b), 0, true);
    m_skinButtonList.AddNewButton(L10N::GetString(0x70a431bd), 1, true);

    const unsigned int count = gApp.mDLCManager.getPackCount(DLCManager::eDLCType_Skin);
    for (unsigned int i = 0; i < count; ++i)
    {
        DLCPack* pack = gApp.mDLCManager.getPack(i, DLCManager::eDLCType_Skin);
        if (pack == 0 || IsTrialDiscontinuedPack(static_cast<int>(pack->GetPackId())) != 0)
            continue;
        const int id = static_cast<int>(pack->GetPackId());
        if (pack->getSkinCount() == 0)
            continue;
        m_packIds.push_back(id);
        if (GetIsNewPack(id) != 0)
            m_newPackIds.push_back(id);
    }
    std::sort(m_packIds.begin(), m_packIds.end(), SORTALPHABETICALLY);
    for (std::vector<int>::const_iterator it = m_packIds.begin(); it != m_packIds.end(); ++it)
    {
        DLCPack* pack = gApp.mDLCManager.getPackFromPackID(*it);
        if (pack != 0)
            m_skinButtonList.AddNewButton(pack->getLocalisedDisplayName(), *it, true);
    }
    m_skinButtonList.HighlightItem(static_cast<unsigned int>(m_packTypeId), false);
}

void UIScene_SkinSelectMenu::setPackLabel()
{
    if (m_packTypeId < 2)
    {
        if (m_packTypeId == 1)
            m_packName.setLabel(UIString(0x70a431bd), false, false);
        else
            m_packName.setLabel(UIString(0xa6517d6b), false, false);
    }
    else
    {
        DLCPack* pack = gApp.mDLCManager.getPackFromPackID(m_packTypeId);
        m_packName.setLabel(UIString(pack->getLocalisedDisplayName()), false, false);
    }

    if (m_packTypeId > 1)
    {
        if (static_cast<unsigned int>(m_packTypeId - 0x400) < 0x400)
            m_packType.setLabel(UIString(0x74a1ede8), false, false);
        else
            m_packType.setLabel(UIString(0x175adb3d), false, false);
    }
}

void UIScene_SkinSelectMenu::updatePackDisplay()
{
    setPackLabel();
}

void UIScene_SkinSelectMenu::handlePackIndexChanged()
{
    m_packTypeId = static_cast<int>(m_skinButtonList.getCurrentSelection());
    setActivePackIndex();
    handleSkinIndexChanged();
    SetSkinPackButtonList();
    updatePackDisplay();
}

void UIScene_SkinSelectMenu::tick()
{
    UIScene::tick();
    if (m_focusChanged)
    {
        m_focusChanged = false;
        handlePackIndexChanged();
    }
    m_dlcScene.tick();
}

unsigned int UIScene_SkinSelectMenu::IsFavoriteSkin(int player, int skin)
{
    unsigned int count = GameSettings::GetPlayerFavoriteSkinsCount(player);
    for (unsigned int i = 0; i < count; ++i)
        if (GameSettings::GetPlayerFavoriteSkin(m_playerId, i) == static_cast<unsigned int>(skin))
            return 1;
    return 0;
}

void UIScene_SkinSelectMenu::updateTooltips()
{
    int skinTooltip = -1;
    unsigned int player = static_cast<unsigned int>(m_playerId);

    if (m_packTypeId == 1)
    {
        if (GameSettings::GetPlayerFavoriteSkinsCount(m_playerId) != 0)
            skinTooltip = 0x3de1e040;
    }
    else
    {
        skinTooltip = 0x855b0987;
        if (m_packTypeId == 0)
        {
            if (IsFavoriteSkin(m_playerId, m_selectedSkinId) != 0)
                skinTooltip = 0x3de1e040;
        }
        else
        {
            DLCPack* pack = gApp.mDLCManager.getPackFromPackID(m_packTypeId);
            DLCSkinFile* file = pack == 0 ? 0 : static_cast<DLCSkinFile*>(pack->getSkinFile(static_cast<unsigned int>(m_selectedSkinId)));
            if (file != 0)
            {
                int skinId = CMinecraftApp::getSkinIdFromPath(file->m_path);
                if (IsFavoriteSkin(m_playerId, skinId) != 0)
                    skinTooltip = 0x3de1e040;
                if (!pack->hasPurchasedPack() && !file->isFree())
                    skinTooltip = -1;
            }
        }
    }

    const int primaryTooltip = m_characterFacing ? -1 : 0x4d09d95;
    gConsoleUIController.SetTooltips(player, primaryTooltip, 0xe21d8023, -1, skinTooltip,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, 0x2336ec22, false, false);
}

void UIScene_SkinSelectMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x43), false);
}

void UIScene_SkinSelectMenu::handleAnimationEnd()
{
    if (!m_skinChanging)
    {
        return;
    }
    m_skinChanging = false;
    m_favouriteChanged = true;
}

std::wstring UIScene_SkinSelectMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? L"SkinSelectMenu" : L"SkinSelectMenuSplit";
}

void UIScene_SkinSelectMenu::InputActionOK(unsigned int player)
{
    if (m_packTypeId == 1)
    {
        unsigned int skin = GameSettings::GetPlayerFavoriteSkin(static_cast<int>(player), m_skinIndex);
        GameSettings::SetPlayerSkin(static_cast<int>(player), skin, true);
        GameSettings::SetPlayerCape(static_cast<int>(player), 0, true);
    }
    else
        GameSettings::SetPlayerSkin(static_cast<int>(player), static_cast<unsigned int>(m_selectedSkinId), true);
    m_selectedSkinId = static_cast<int>(GameSettings::GetPlayerSkinId(static_cast<int>(player)));
    setCharacterSelected(true);
}

int UIScene_SkinSelectMenu::ManageFavoritesReturned(void* context, int, C4JStorage::EMessageResult result)
{
    UIScene* callbackScene = gConsoleUIController.GetSceneFromCallbackId(static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context)));
    UIScene_SkinSelectMenu* scene = dynamic_cast<UIScene_SkinSelectMenu*>(callbackScene);
    if (scene != NULL)
    {
        if (result == static_cast<C4JStorage::EMessageResult>(4))
        {
            scene->m_skinButtonList.HighlightItem(static_cast<unsigned int>(scene->m_packTypeId), false);
            scene->handlePackIndexChanged();
        }
        else
        {
            scene->m_inputPending = false;
        }
    }
    return 0;
}

void UIScene_SkinSelectMenu::SetFavoriteSkin(int player, int skin)
{
    unsigned int count = GameSettings::GetPlayerFavoriteSkinsCount(player);
    unsigned int slot = 0;
    bool found = false;
    if (count != 0)
    {
        int existing = static_cast<int>(GameSettings::GetPlayerFavoriteSkin(m_playerId, 0));
        while (existing != skin)
        {
            ++slot;
            if (slot >= count)
            {
                break;
            }
            existing = static_cast<int>(GameSettings::GetPlayerFavoriteSkin(m_playerId, slot));
        }
        found = slot < count;
    }
    if (found)
    {
        setCharacterFavourite(false);
        for (unsigned int i = slot; i < count - 1; ++i)
        {
            unsigned int next = GameSettings::GetPlayerFavoriteSkin(player, i + 1);
            GameSettings::SetPlayerFavoriteSkin(player, i, next);
        }
        GameSettings::SetPlayerFavoriteSkin(player, count - 1, static_cast<unsigned int>(-1));
        if (GameSettings::GetPlayerFavoriteSkinsPos(player) == slot)
        {
            unsigned int newCount = GameSettings::GetPlayerFavoriteSkinsCount(player);
            GameSettings::SetPlayerFavoriteSkinsPos(m_playerId, newCount);
        }
        return;
    }
    if (count == 10)
    {
        int messageParameters[2] = { static_cast<int>(0xb9a1bf4d), static_cast<int>(0x4fcbc7e7) };
        gConsoleUIController.RequestAlertMessage(0x397a0127, 0x935a46d2, messageParameters, 2, player,
                                                 ManageFavoritesReturned,
                                                 reinterpret_cast<void*>(GetCallbackUniqueId()), NULL, true);
        return;
    }
    setCharacterFavourite(true);
    GameSettings::SetPlayerFavoriteSkin(player, count, static_cast<unsigned int>(skin));
    if (m_selectedSkinId == skin)
    {
        GameSettings::SetPlayerFavoriteSkinsPos(m_playerId, count);
    }
}

void UIScene_SkinSelectMenu::InputActionFavorite(unsigned int player)
{
    SetFavoriteSkin(static_cast<int>(player), m_selectedSkinId);
}

void UIScene_SkinSelectMenu::handleInput(int player, int action, bool repeat, bool released,
                                         bool pressed, bool& handled)
{
    if (m_dlcScene.m_purchasePending)
    {
        return;
    }
    switch (action)
    {
    case 3:
        if (released)
        {
            gConsoleUIController.AnimateKeyPress(player, action, repeat, released, pressed);
            InputActionFavorite(static_cast<unsigned int>(player));
            handled = true;
            return;
        }
        break;
    case 4:
    case 5:
    case 8:
    case 9:
        if (released)
        {
            sendInputToMovie(action, repeat, released, pressed);
            handled = true;
            return;
        }
        break;
    case 6:
        if (released && !m_skinChanging)
        {
            gConsoleUIController.AnimateKeyPress(player, action, repeat, released, pressed);
            gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
            m_skinIndex = getNextSkinIndex(m_skinIndex);
            m_skinChanging = true;
            m_characterFacing = true;
            m_characterPreviews[0].SetFacing(UIControl_PlayerSkinPreview::eFacingLeft, true);
            m_characterPreviews[1].SetFacing(UIControl_PlayerSkinPreview::eFacingFront, true);
            sendInputToMovie(7, repeat, released, pressed);
            return;
        }
        break;
    case 7:
        if (released && !m_skinChanging)
        {
            gConsoleUIController.AnimateKeyPress(player, action, repeat, released, pressed);
            gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
            m_skinIndex = getPreviousSkinIndex(m_skinIndex);
            m_characterFacing = true;
            m_skinChanging = true;
            m_characterPreviews[0].SetFacing(UIControl_PlayerSkinPreview::eFacingRight, true);
            m_characterPreviews[4].SetFacing(UIControl_PlayerSkinPreview::eFacingFront, true);
            sendInputToMovie(6, repeat, released, pressed);
            return;
        }
        break;
    case 0xd:
        if (released)
        {
            gConsoleUIController.PlayUISFX(SoundEvents::UI_BACK);
            m_characterPreviews[0].mRotation = 0;
            m_characterPreviews[0].mPitch = 0;
            return;
        }
        break;
    case 0xe:
        if (released)
        {
            m_characterPreviews[0].CyclePreviousAnimation();
            return;
        }
        break;
    case 0xf:
        if (released)
        {
            m_characterPreviews[0].CycleNextAnimation();
        }
    case 0x10:
        if (released)
        {
            m_characterPreviews[0].mRotateForward = true;
            return;
        }
        if (pressed)
        {
            m_characterPreviews[0].mRotateForward = false;
            return;
        }
        break;
    case 0x11:
        if (released)
        {
            m_characterPreviews[0].mRotateBackward = true;
            return;
        }
        if (pressed)
        {
            m_characterPreviews[0].mRotateBackward = false;
            return;
        }
        break;
    case 0x14:
        if (released)
        {
            InputActionOK(static_cast<unsigned int>(player));
            return;
        }
        break;
    case 0x15:
        if (released)
        {
            gConsoleUIController.AnimateKeyPress(player, action, repeat, released, pressed);
            GameSettings::CheckGameSettingsChanged(true, player);
            navigateBack();
            return;
        }
        break;
    default:
        break;
    }
}

void UIScene_SkinSelectMenu::customDraw(const char* name, fuiRect* rect)
{
    if (m_characterFacing && m_movie != NULL)
    {
        FJ_FuiNode* characters = m_movie->getRootNode()->getFuiNode()->getChildByName("IggyCharacters");
        if (characters != NULL && characters->currentFrame() == 1)
        {
            m_characterPreviews[0].SetFacing(UIControl_PlayerSkinPreview::eFacingFront, false);
            m_characterPreviews[1].SetFacing(UIControl_PlayerSkinPreview::eFacingRight, false);
            m_characterPreviews[2].SetFacing(UIControl_PlayerSkinPreview::eFacingLeft, false);
            m_characterFacing = false;
            m_skinChanging = false;
            handleSkinIndexChanged();
        }
    }
    int character = -1;
    if (name != NULL)
        std::sscanf(name, "Character%d", &character);
    if (character >= 0 && character < 7)
        m_characterPreviews[character].render(rect);
}

void UIScene_SkinSelectMenu::HandleDLCMountingComplete()
{
    m_characterPanel.setVisible(false);
    m_timer.setVisible(true);
    GameSettings::ValidateFavoriteSkins(m_playerId);
    setActivePackIndex();
    m_dlcChanged = true;
    handlePackIndexChanged();
    m_dlcScene.m_purchasePending = false;
    gApp.mDLCManager.checkForCorruptDLCAndAlert(true);
}

void UIScene_SkinSelectMenu::HandleDLCInstalled()
{
    if (g_pApp->StartInstallDLCProcess(m_playerId) == 0)
        return;
    m_dlcChanged = true;
    m_timer.setVisible(true);
    m_characterPanel.setVisible(false);
}

void UIScene_SkinSelectMenu::HandleDLCLicenseChange()
{
    handleSkinIndexChanged();
}

void UIScene_SkinSelectMenu::handleTouchInput(unsigned int playerId, int touchX, int touchY,
                                              int touchBox, bool repeat, bool released,
                                              bool pressed)
{
    if (!repeat)
    {
        if (!released)
        {
            if (pressed && touchBox == 10 && !m_inputPending)
            {
                InputActionOK(playerId);
            }
        }
        else if (touchBox == 10)
        {
            if (touchX < m_packButtonX - 50)
            {
                if (!m_skinChanging && !m_inputPending)
                {
                    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
                    m_skinIndex = getNextSkinIndex(m_skinIndex);
                    m_skinChanging = true;
                    m_characterFacing = true;
                    m_characterPreviews[0].SetFacing(UIControl_PlayerSkinPreview::eFacingLeft, true);
                    m_characterPreviews[1].SetFacing(UIControl_PlayerSkinPreview::eFacingFront, true);
                    sendInputToMovie(7, false, true, false);
                    m_inputPending = true;
                    return;
                }
            }
            else if (touchX > m_packButtonX + 50)
            {
                if (!m_skinChanging && !m_inputPending)
                {
                    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
                    m_skinIndex = getPreviousSkinIndex(m_skinIndex);
                    m_skinChanging = true;
                    m_characterFacing = true;
                    m_characterPreviews[0].SetFacing(UIControl_PlayerSkinPreview::eFacingRight, true);
                    m_characterPreviews[4].SetFacing(UIControl_PlayerSkinPreview::eFacingFront, true);
                    sendInputToMovie(6, false, true, false);
                    m_inputPending = true;
                    return;
                }
            }
        }
    }
    else if (touchBox == 10 && (m_packTypeId != 1 || GameSettings::GetPlayerFavoriteSkinsCount(m_playerId) != 0))
    {
        m_packButtonX = touchX;
        m_inputPending = false;
        return;
    }
}

void UIScene_SkinSelectMenu::handleReload()
{
    m_dlcScene.m_purchasePending = false;
    m_skinTitle1.init(UIString(L""));
    m_skinTitle2.init(UIString(L""));
    m_characterFacing = false;
    m_dlcChanged = true;
    m_highlightedPackId = 0;
    int previousPackTypeId = m_packTypeId;
    m_focusChanged = true;
    m_hasSelectedPack = false;
    if (gApp.mDLCManager.getPackCount(DLCManager::eDLCType_Skin) != 0)
    {
        DLCPack* pack = gApp.mDLCManager.getPackContainingSkin(m_skinName);
        m_dlcScene.m_pack = pack;
        m_activePackId = 0;
        if (pack != 0)
            m_activePackId = static_cast<int>(pack->GetPackId());
    }
    GameSettings::ValidateFavoriteSkins(m_playerId);
    if (GameSettings::GetPlayerFavoriteSkinsCount(m_playerId) != 0)
        m_packTypeId = 1;
    handlePackIndexChanged();
    m_skinButtonList.updateChildFocus(previousPackTypeId);
    handlePackIndexChanged();
    m_skinButtonList.HighlightItem(static_cast<unsigned int>(previousPackTypeId), false);
}

void UIScene_SkinSelectMenu::handleFocusChange(int focus, int childFocus)
{
    if (focus == 0)
    {
        m_skinButtonList.updateChildFocus(childFocus);
        m_focusChanged = true;
    }
}

void UIScene_SkinSelectMenu::handlePress(int, int)
{
}

int UIScene_SkinSelectMenu::getSceneType()
{
    return 0x26;
}
