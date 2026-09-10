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
#include "game/ui/FJ_SkinSelect.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

extern CConsoleMinecraftApp gApp;
extern CMinecraftApp* g_pApp;
extern ConsoleUIController gConsoleUIController;

UIScene_SkinSelectMenu::UIScene_SkinSelectMenu(int player, void* data, UILayer* layer)
    : UIScene(0x26, layer), m_dlcScene(static_cast<eDLCContentType>(1)), m_activePackId(0),
      m_packTypeId(0), m_skinIndex(0), m_selectedSkinId(0), m_skinBoxes(0),
      m_characterFacing(false), m_skinChanging(false), m_favouriteChanged(false),
      m_focusChanged(false), m_packChanged(true), m_dlcChanged(false),
      m_hasSelectedPack(false), m_packButtonX(0), m_packButtonY(0), m_inputPending(false),
      m_player(player)
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
    if (m_packTypeId > 1)
    {
        DLCPack* pack = gApp.mDLCManager.getPackFromPackID(m_packTypeId);
        if (pack != 0)
            m_activePackId = static_cast<int>(pack->GetPackId());
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
        count = GameSettings::GetPlayerFavoriteSkinsCount(m_player);
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
        return GameSettings::GetPlayerFavoriteSkinsCount(m_player) - 1;
    if (m_activePackId != 0)
    {
        DLCPack* pack = gApp.mDLCManager.getPackFromPackID(m_activePackId);
        return pack == 0 ? 0 : pack->getSkinCount() - 1;
    }
    return 0x12;
}

void UIScene_SkinSelectMenu::handleSkinIndexChanged()
{
    if (m_packTypeId == 1)
        m_selectedSkinId = static_cast<int>(GameSettings::GetPlayerFavoriteSkin(m_player, m_skinIndex));
    else if (m_activePackId != 0)
        m_selectedSkinId = m_skinIndex;
    m_skinChanging = false;
}

unsigned int UIScene_SkinSelectMenu::IsTrialDiscontinuedPack(int packId)
{
    DLCPack* pack = gApp.mDLCManager.getPackFromPackID(packId);
    return pack != 0 && !pack->hasPurchasedPack() ? 1U : 0U;
}

unsigned int UIScene_SkinSelectMenu::GetIsNewPack(int packId)
{
    return std::find(m_newPackIds.begin(), m_newPackIds.end(), packId) != m_newPackIds.end();
}

bool UIScene_SkinSelectMenu::SORTNEWEST(int first, int second)
{
    return first < second;
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
    m_skinButtonList.clearList();
    m_skinButtonList.AddNewButton(L"Default", 0, true);
    m_skinButtonList.AddNewButton(L"Favorites", 1, true);
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
    std::wstring name = L"Default";
    if (m_packTypeId == 1)
        name = L"Favorites";
    else if (m_packTypeId > 1)
    {
        DLCPack* pack = gApp.mDLCManager.getPackFromPackID(m_packTypeId);
        if (pack != 0)
            name = pack->getLocalisedDisplayName();
    }
    m_packName.setLabel(UIString(name), false, false);
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
        if (GameSettings::GetPlayerFavoriteSkin(player, i) == static_cast<unsigned int>(skin))
            return 1;
    return 0;
}

void UIScene_SkinSelectMenu::updateTooltips()
{
    UIScene::updateTooltips();
}

void UIScene_SkinSelectMenu::updateComponents()
{
    UIScene::updateComponents();
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

unsigned int UIScene_SkinSelectMenu::ManageFavoritesReturned(void* context, int, C4JStorage::EMessageResult result)
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
    GameSettings::SetPlayerFavoriteSkin(player, m_skinIndex, static_cast<unsigned int>(skin));
    m_favouriteChanged = true;
}

void UIScene_SkinSelectMenu::InputActionFavorite(unsigned int player)
{
    SetFavoriteSkin(static_cast<int>(player), m_selectedSkinId);
}

void UIScene_SkinSelectMenu::handleInput(int player, int action, bool, bool, bool pressed, bool& handled)
{
    if (pressed && action == 10 && !m_skinChanging)
    {
        InputActionOK(static_cast<unsigned int>(player));
        handled = true;
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
    m_dlcChanged = true;
    GameSettings::ValidateFavoriteSkins(m_player);
    setActivePackIndex();
    handlePackIndexChanged();
}

void UIScene_SkinSelectMenu::HandleDLCInstalled()
{
    if (g_pApp->StartInstallDLCProcess(m_player) == 0)
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
    handlePackIndexChanged();
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
