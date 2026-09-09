#include "game/core/ConsoleUIController.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <cwchar>

#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferedImage.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/PerformanceTimer.hpp"
#include "game/core/System.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/FJ_PressStartToPlay.hpp"
#include "game/ui/Tooltips.hpp"
#include "game/ui/UIComponent_PressStartToPlay.hpp"
#include "game/ui/UIComponent_Tooltips.hpp"
#include "game/ui/UIComponent_TutorialPopup.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_DynamicLabel.hpp"
#include "game/ui/UIControl_HTMLLabel.hpp"
#include "game/ui/UIControl_Slider.hpp"
#include "game/ui/UIControl_TexturePackList.hpp"
#include "game/ui/UIControl_CheckBox.hpp"
#include "game/ui/UIControl_LeaderboardList.hpp"
#include "game/ui/UIControl_List.hpp"
#include "game/ui/UIControl_GridList.hpp"
#include "game/ui/UIGroup.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/UIScene_HUD.hpp"
#include "game/ui/UITTFFont.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiBitmapFont.hpp"
#include "game/ui/fuiFile.hpp"
#include "platform/wiiu/C4JThread.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/UIComponent_WiiUDRC.hpp"
#include "vendor/_TutorialPopupInfo.hpp"

unsigned int XGetLanguage();
unsigned int GetTickCount();
void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

OSFastMutex g_uiCriticalSection;
ConsoleUIController gConsoleUIController;

static bool s_uiCriticalSectionInitialized = false;

void PopupToast(void* context, int playerId, int achievementId, int imageId, int value)
{
    (void)context;
    gConsoleUIController.SetAchievementUnlocked(playerId, achievementId, imageId, value);
}

ConsoleUIController::ConsoleUIController()
    : m_debugConsole(0), m_unknown08(0), m_screenWidth(0.0f), m_screenHeight(0.0f),
      m_initialized(false), m_renderOffsetX(0), m_renderOffsetY(0), m_currentFont(0),
      m_pendingFont(EFont_Bitmap), m_cleanupOnReload(false), m_autoCleanupOnReload(true),
      m_fontChangePending(false), m_unknown43B(0), m_font(0), m_oldFont(0),
      m_bitmapFont(0), m_bitmapFontAlt(0), m_touchHeld(false), m_touchX(0), m_touchY(0),
      m_rebuildingTouchBoxes(false), m_resolution(0), m_requestedResolution(0),
      m_resolutionDisableFlags(0), m_lastInputTime(0), m_trialTimerLimit(0),
      m_trialTimerValue(0), m_trialTimerTick(0), m_autosaveTimerTick(0),
      m_selectedPlayer(0), m_selectedX(0), m_selectedY(0), m_selectedItemVisible(false),
      m_winUserIndex(0), m_skinThreadExpected(false), m_skinThread(0), m_nextCallbackId(0),
      m_touchOffsetX(0), m_touchOffsetY(0), m_touchElement(0), m_lastTouchElement(0),
      m_pressStartPlaying(0), m_pressStartBlocked(0), m_fullscreenMenuDisplayed(false),
      m_customDrawLeft(0), m_customDrawTop(0), m_customDrawRight(0), m_customDrawBottom(0)
{
    std::memset(m_unknown90, 0, sizeof(m_unknown90));
    std::memset(m_menuDisplayed, 0, sizeof(m_menuDisplayed));
    std::memset(m_menuDisplayPending, 0, sizeof(m_menuDisplayPending));
    std::memset(m_menuDisplayDelay, 0, sizeof(m_menuDisplayDelay));
    std::memset(m_playerFlags, 0, sizeof(m_playerFlags));
    std::memset(m_groups, 0, sizeof(m_groups));
    InitializeCriticalSection(&m_renderMutex);
    InitializeCriticalSection(&m_touchMutex);
    InitializeCriticalSection(&m_skinMutex);
    InitializeCriticalSection(&m_fontMutex);
    InitializeCriticalSection(&m_skinThreadMutex);
    InitializeCriticalSection(&m_callbackMutex);
    if (!s_uiCriticalSectionInitialized) {
        InitializeCriticalSection(&g_uiCriticalSection);
        s_uiCriticalSectionInitialized = true;
    }
}

ConsoleUIController::~ConsoleUIController()
{
}

int ConsoleUIController::GetGroup(int playerId)
{
    if (playerId >= 0 && playerId != 0xff)
        return playerId + 2;
    return 1;
}

UIGroup* ConsoleUIController::group(EUIGroup groupId)
{
    return m_groups[(int)groupId];
}

UIGroup* ConsoleUIController::groupForPlayer(int playerId, bool forceGlobal)
{
    if (forceGlobal || !gApp.GetGameStarted())
        return m_groups[1];
    return m_groups[GetGroup(playerId)];
}

UIScene* ConsoleUIController::playerComponent(unsigned int playerId, EUIScene scene)
{
    UIGroup* playerGroup = groupForPlayer((int)playerId, false);
    return playerGroup == 0 ? 0 : playerGroup->FindScene(scene);
}

void ConsoleUIController::preInit(int width, int height)
{
    m_initialized = true;
    m_screenWidth = (float)width;
    m_screenHeight = (float)height;
}

void ConsoleUIController::postInit()
{
    gFui->setCustomTextureFn(TextureSubstitutionCreateCallbackFui,
                             TextureSubstitutionDestroyCallbackFui, this);
    SetupFont();
    loadSkins();
    m_groups[0] = new UIGroup((EUIGroup)0, -1);
    m_groups[1] = new UIGroup((EUIGroup)1, -1);
    m_groups[2] = new UIGroup((EUIGroup)2, 0);
    m_groups[3] = new UIGroup((EUIGroup)3, 1);
    m_groups[4] = new UIGroup((EUIGroup)4, 2);
    m_groups[5] = new UIGroup((EUIGroup)5, 3);
    NavigateToScene(0, (EUIScene)1, 0, (EUILayer)6, (EUIGroup)7);
}

void ConsoleUIController::init(int width, int height)
{
    gFui->setScreenSize((float)width, (float)height);
    preInit(width, height);
    postInit();
}

int ConsoleUIController::TextureSubstitutionCreateCallbackFui(void* context,
                                                               const std::wstring& name,
                                                               unsigned int* width,
                                                               unsigned int* height,
                                                               bool* hasAlpha)
{
    ConsoleUIController* controller = static_cast<ConsoleUIController*>(context);
    boost::unordered_map<std::wstring, _TextureDetail>::iterator found =
        controller->m_substitutionTextures.find(name);
    if (found == controller->m_substitutionTextures.end())
        return -1;
    _TextureDetail& detail = found->second;
    *width = 64;
    *height = 64;
    if (detail.textureLoaded)
        return detail.texture;
    if (detail.data == 0) {
        *hasAlpha = true;
        return (int)detail.size;
    }
    BufferedImage image(detail.data, detail.size);
    if (image.getData() == 0)
        return -1;
    image.preMultiplyAlpha();
    int texture = Minecraft::GetInstance()->getTextureManager()->getTexture(
        &image, (C4JRender::eTextureFormat)0, false);
    GlStateManager::bindTexture(texture);
    GlStateManager::texParameteri(0, 1, 1);
    return texture;
}

void ConsoleUIController::TextureSubstitutionDestroyCallbackFui(int texture)
{
    Minecraft::GetInstance()->getTextureManager()->releaseTexture(texture);
}

ConsoleUIController::EFont ConsoleUIController::getFontForLanguage(int language)
{
    if (language == 2)
        return EFont_Japanese;
    if (language == 7)
        return EFont_Korean;
    if (language == 8)
        return EFont_ChineseTraditional;
    if (language == 0x11)
        return EFont_ChineseSimplified;
    return EFont_Bitmap;
}

UITTFFont* ConsoleUIController::createFont(EFont font)
{
    if (font == EFont_Japanese)
        return new UITTFFont("Mojangles_TTF_jaJP", "Common/Media/font/JPN_DFGMaruGothic.ttf",
                             "Common/Media/font/KOR_BOKMSD.ttf",
                             "Common/Media/font/CHT_DFHeiMedium.ttf", 0x3f);
    if (font == EFont_ChineseSimplified)
        return new UITTFFont("Mojangled_TTF_cnCN", "Common/Media/font/CHS_MSYH.ttf", 0x3f);
    if (font == EFont_ChineseTraditional)
        return new UITTFFont("Mojangles_TTF_cnTD", "Common/Media/font/CHT_DFHeiMedium.ttf",
                             "Common/Media/font/KOR_BOKMSD.ttf",
                             "Common/Media/font/JPN_DFGMaruGothic.ttf", 0x3f);
    if (font == EFont_Korean)
        return new UITTFFont("Mojangles_TTF_koKR", "Common/Media/font/KOR_BOKMSD.ttf",
                             "Common/Media/font/JPN_DFGMaruGothic.ttf",
                             "Common/Media/font/CHT_DFHeiMedium.ttf", 0x3f);
    return 0;
}

void ConsoleUIController::updateCurrentFont()
{
    m_currentFont = m_pendingFont;
    if (m_oldFont != 0) {
        delete m_oldFont;
        m_oldFont = 0;
    }
}

void ConsoleUIController::SetupFont()
{
    EFont wanted = getFontForLanguage((int)XGetLanguage());
    if (wanted == m_currentFont && m_font != 0)
        return;
    EnterCriticalSection(&m_fontMutex);
    m_pendingFont = wanted;
    if (wanted == EFont_Bitmap) {
        if (m_bitmapFont != 0)
            m_bitmapFont->registerFont();
    } else {
        UITTFFont* replacement = createFont(wanted);
        if (replacement != 0) {
            m_oldFont = m_font;
            m_font = replacement;
        }
    }
    updateCurrentFont();
    LeaveCriticalSection(&m_fontMutex);
}

int ConsoleUIController::loadSkin(const std::wstring& name, const std::wstring& path, int texture)
{
    int loaded = Minecraft::GetInstance()->getTextureManager()->loadMemTexture(name, path);
    if (loaded == -1)
        loaded = texture;
    registerSubstitutionTexture(name, loaded);
    return loaded;
}

void ConsoleUIController::loadSkins()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    minecraft->getTextureManager()->reloadAll();
    m_skinThreadExpected = false;
}

void ConsoleUIController::renderScenes()
{
    PIXBeginNamedEvent(0.0f, "Rendering FUI scenes");
    if (gApp.GetGameStarted() && !m_groups[1]->hidesLowerScenes()) {
        m_groups[2]->render();
        m_groups[3]->render();
        m_groups[4]->render();
        m_groups[5]->render();
    }
    m_groups[1]->render();
    gFui->setScreenSize(854.0f, 480.0f);
    m_groups[0]->render();
    gFui->setScreenSize(1280.0f, 720.0f);
    PIXEndNamedEvent();
}

void ConsoleUIController::render()
{
    renderScenes();
}

bool ConsoleUIController::getCleanupOnReload()
{
    return m_cleanupOnReload;
}

void ConsoleUIController::setCleanupOnReload(bool cleanupOnReload)
{
    m_autoCleanupOnReload = cleanupOnReload;
    m_cleanupOnReload = true;
}

bool ConsoleUIController::PendingFontChange()
{
    return getFontForLanguage((int)XGetLanguage()) != m_currentFont;
}

bool ConsoleUIController::UsingBitmapFont()
{
    return m_currentFont == EFont_Bitmap;
}

void ConsoleUIController::registerSubstitutionTexture(const std::wstring& name, int texture)
{
    _TextureDetail& detail = m_substitutionTextures[name];
    ++detail.referenceCount;
    if (detail.referenceCount == 1) {
        detail.texture = texture;
        detail.textureLoaded = true;
    }
}

void ConsoleUIController::registerSubstitutionTexture(const std::wstring& name, unsigned char* data,
                                                       unsigned int size, bool deleteData)
{
    boost::unordered_map<std::wstring, _TextureDetail>::iterator found =
        m_substitutionTextures.find(name);
    if (found == m_substitutionTextures.end()) {
        _TextureDetail& detail = m_substitutionTextures[name];
        detail.referenceCount = 1;
        detail.data = data;
        detail.size = size;
        detail.textureLoaded = false;
    } else {
        ++found->second.referenceCount;
        if (deleteData)
            delete[] data;
    }
}

void ConsoleUIController::unregisterSubstitutionTexture(const std::wstring& name, bool deleteData)
{
    boost::unordered_map<std::wstring, _TextureDetail>::iterator found =
        m_substitutionTextures.find(name);
    if (found == m_substitutionTextures.end())
        return;
    if (--found->second.referenceCount < 1) {
        if (deleteData)
            delete[] found->second.data;
        m_substitutionTextures.erase(found);
    }
}

ConsoleUIController::_CachedMovieData& ConsoleUIController::getMovieData(const std::wstring& path)
{
    unsigned long long expiry = System::processTimeInMilliSecs() + 60000;
    boost::unordered_map<std::wstring, _CachedMovieData>::iterator found = m_movieCache.find(path);
    if (found == m_movieCache.end()) {
        arrayWithLength<unsigned char> data = gApp.getArchiveFile(path, false);
        m_movieCache[path] = _CachedMovieData(data, expiry);
    } else {
        found->second.expiryTime = expiry;
    }
    return m_movieCache[path];
}

void ConsoleUIController::getRenderDimensions(C4JRender::eViewportType viewport, int& width,
                                               int& height)
{
    if (viewport == C4JRender::eViewportType_FullScreen) {
        width = (int)m_screenWidth;
        height = (int)m_screenHeight;
    } else if (viewport == C4JRender::eViewportType_TopHalf ||
               viewport == C4JRender::eViewportType_BottomHalf) {
        width = (int)m_screenWidth;
        height = (int)m_screenHeight / 2;
    } else if (viewport == C4JRender::eViewportType_LeftHalf ||
               viewport == C4JRender::eViewportType_RightHalf) {
        width = (int)m_screenWidth / 2;
        height = (int)m_screenHeight;
    } else {
        width = (int)m_screenWidth / 2;
        height = (int)m_screenHeight / 2;
    }
}

void ConsoleUIController::setupRenderPosition(int x, int y)
{
    m_renderOffsetX = x;
    m_renderOffsetY = y;
}

void ConsoleUIController::updateViewportTouchOffset(int x, int y)
{
    m_touchOffsetX = x;
    m_touchOffsetY = y;
}

void ConsoleUIController::SetTutorialVisible(int playerId, bool visible)
{
    UIComponent_TutorialPopup* popup = static_cast<UIComponent_TutorialPopup*>(
        groupForPlayer(playerId, false)->m_componentSecondary);
    if (popup != 0)
        popup->SetVisible(visible);
}

void ConsoleUIController::SetTutorial(int playerId, Tutorial* tutorial)
{
    UIComponent_TutorialPopup* popup = static_cast<UIComponent_TutorialPopup*>(
        groupForPlayer(playerId, false)->m_componentSecondary);
    if (popup != 0)
        popup->m_tutorial = tutorial;
}

bool ConsoleUIController::IsPauseMenuDisplayed(int playerId)
{
    return groupForPlayer(playerId, false)->m_pauseMenuDisplayed;
}

bool ConsoleUIController::GetMenuDisplayed(int playerId)
{
    return m_menuDisplayed[playerId];
}

void ConsoleUIController::SetTutorialDescription(int playerId, _TutorialPopupInfo* popupInfo)
{
    UIGroup* playerGroup = groupForPlayer(playerId, false);
    UIComponent_TutorialPopup* popup = static_cast<UIComponent_TutorialPopup*>(
        playerGroup->m_componentSecondary);
    if (popup != 0) {
        popup->m_containerMenuDisplayed = playerGroup->m_containerMenuDisplayed;
        popup->SetTutorialDescription(popupInfo);
    }
}

void ConsoleUIController::RemoveInteractSceneReference(int playerId, UIScene* scene)
{
    UIComponent_TutorialPopup* popup = static_cast<UIComponent_TutorialPopup*>(
        groupForPlayer(playerId, false)->m_componentSecondary);
    if (popup != 0)
        popup->RemoveInteractSceneReference(scene);
}

void ConsoleUIController::updateViewportTouchOffset(C4JRender::eViewportType viewport)
{
    int x = 0;
    int y = 0;
    switch ((int)viewport) {
    case 1:
        x = (int)(m_screenWidth * 0.25f);
        break;
    case 2:
        x = (int)(m_screenWidth * 0.25f);
        y = (int)(m_screenHeight * 0.5f);
        break;
    case 3:
        y = (int)(m_screenHeight * 0.25f);
        break;
    case 4:
        x = (int)(m_screenWidth * 0.5f);
        y = (int)(m_screenHeight * 0.25f);
        break;
    case 6:
        x = (int)(m_screenWidth * 0.5f);
        break;
    case 7:
        y = (int)(m_screenHeight * 0.5f);
        break;
    case 8:
        x = (int)(m_screenWidth * 0.5f);
        y = (int)(m_screenHeight * 0.5f);
        break;
    }
    updateViewportTouchOffset(x, y);
}

void ConsoleUIController::setupRenderPosition(C4JRender::eViewportType viewport)
{
    updateViewportTouchOffset(viewport);
    setupRenderPosition(m_touchOffsetX, m_touchOffsetY);
    gFui->setOrigin((float)m_renderOffsetX, (float)m_renderOffsetY);
}

void ConsoleUIController::setupCustomDrawGameState()
{
    m_customDrawLeft = 0x7fffffff;
    m_customDrawTop = 0x7fffffff;
    m_customDrawRight = -0x7fffffff;
    m_customDrawBottom = -0x7fffffff;
    PIXBeginNamedEvent(0.0f, "Final setup");
    GlStateManager::matrixMode(1);
    GlStateManager::loadIdentity();
    GlStateManager::ortho(0.0, m_screenWidth, m_screenHeight, 0.0, 1000.0, 3000.0);
    GlStateManager::matrixMode(0);
    GlStateManager::enableAlphaTest();
    GlStateManager::alphaFunc(4, 0.1f);
    GlStateManager::enableDepthTest();
    GlStateManager::depthFunc(3);
    GlStateManager::depthMask(true);
    PIXEndNamedEvent();
}

void ConsoleUIController::endCustomDrawGameState()
{
    GlStateManager::depthMask(false);
    GlStateManager::disableAlphaTest();
}

void ConsoleUIController::SetMenuDisplayed(int playerId, bool displayed)
{
    if (playerId == 0xff) {
        for (int i = 0; i < 4; ++i) {
            m_menuDisplayPending[i] = displayed;
            m_menuDisplayDelay[i] = 2;
        }
    } else if (playerId >= 0 && playerId < 4) {
        m_menuDisplayPending[playerId] = displayed;
        m_menuDisplayDelay[playerId] = 2;
    }
}

void ConsoleUIController::setFullscreenMenuDisplayed(bool displayed)
{
    m_fullscreenMenuDisplayed = displayed;
    if (m_groups[1] != 0)
        m_groups[1]->showComponent((int)m_winUserIndex, (EUIScene)0x46, (EUILayer)0, displayed);
    for (int i = 2; i < 6; ++i)
        if (m_groups[i] != 0)
            m_groups[i]->showComponent(i - 2, (EUIScene)0x46, (EUILayer)0, !displayed);
}

bool ConsoleUIController::NavigateToScene(int playerId, EUIScene scene, void* sceneData,
                                          EUILayer layer, EUIGroup groupId)
{
    if (IsReloadingSkin())
        return false;
    if ((int)scene == 4) {
        groupId = (EUIGroup)1;
        layer = (EUILayer)4;
        SetMenuDisplayed(0xff, true);
    } else if ((int)scene == 0x1f) {
        layer = (EUILayer)4;
    } else if ((int)scene == 0x30) {
        groupId = (EUIGroup)1;
        layer = (EUILayer)6;
    }
    if ((int)groupId == 7) {
        if (gApp.GetGameStarted() && playerId >= 0 && playerId != 0xff)
            groupId = (EUIGroup)GetGroup(playerId);
        else {
            groupId = (EUIGroup)1;
            if (!gApp.GetGameStarted())
                layer = (EUILayer)4;
        }
    }
    EnterCriticalSection(&m_renderMutex);
    bool navigated = m_groups[(int)groupId]->NavigateToScene(playerId, scene, sceneData, layer);
    if (navigated && (int)groupId == 1)
        setFullscreenMenuDisplayed(true);
    LeaveCriticalSection(&m_renderMutex);
    return navigated;
}

bool ConsoleUIController::NavigateBack(int playerId, bool closeCurrent, EUIScene scene,
                                       EUILayer layer)
{
    bool navigated = m_groups[1]->NavigateBack(playerId, scene, layer);
    if (!gApp.GetGameStarted() && !m_groups[1]->GetMenuDisplayed())
        SetMenuDisplayed(0xff, false);
    if (!navigated && playerId >= 0 && playerId != 0xff) {
        UIGroup* playerGroup = m_groups[GetGroup(playerId)];
        navigated = playerGroup->NavigateBack(playerId, scene, layer);
        if (!playerGroup->GetMenuDisplayed()) {
            Tooltips::ForceTooltipReload(playerId);
            SetMenuDisplayed(playerId, false);
        }
    } else if (!m_groups[1]->GetMenuDisplayed()) {
        setFullscreenMenuDisplayed(false);
        FixupMenuDisplayedAfterNavigateBack();
    }
    return navigated;
}

void ConsoleUIController::FixupMenuDisplayedAfterNavigateBack()
{
    setFullscreenMenuDisplayed(false);
    for (int player = 0; player < 4; ++player)
        SetMenuDisplayed(player, m_groups[player + 2]->GetMenuDisplayed());
}

UIScene* ConsoleUIController::GetTopScene(int playerId, EUILayer layer, EUIGroup groupId)
{
    if ((int)groupId == 7)
        groupId = gApp.GetGameStarted() && playerId >= 0 && playerId != 0xff
                      ? (EUIGroup)GetGroup(playerId) : (EUIGroup)1;
    return m_groups[(int)groupId]->GetTopScene(layer);
}

unsigned int ConsoleUIController::RegisterForCallbackId(UIScene* scene)
{
    unsigned int callbackId = (GetTickCount() & 0x00ffffff) |
                              ((unsigned int)scene->getSceneType() << 24);
    EnterCriticalSection(&m_callbackMutex);
    m_callbackScenes[callbackId] = scene;
    LeaveCriticalSection(&m_callbackMutex);
    return callbackId;
}

void ConsoleUIController::UnregisterCallbackId(unsigned int callbackId)
{
    EnterCriticalSection(&m_callbackMutex);
    m_callbackScenes.erase(callbackId);
    LeaveCriticalSection(&m_callbackMutex);
}

UIScene* ConsoleUIController::GetSceneFromCallbackId(unsigned int callbackId)
{
    boost::unordered_map<unsigned int, UIScene*>::iterator found = m_callbackScenes.find(callbackId);
    return found == m_callbackScenes.end() ? 0 : found->second;
}

void ConsoleUIController::EnterCallbackIdCriticalSection()
{
    EnterCriticalSection(&m_callbackMutex);
}

void ConsoleUIController::LeaveCallbackIdCriticalSection()
{
    LeaveCriticalSection(&m_callbackMutex);
}

void ConsoleUIController::CloseUIScenes(int playerId, bool immediate)
{
    UIGroup* playerGroup = !gApp.GetGameStarted() && !immediate
                               ? m_groups[1] : m_groups[GetGroup(playerId)];
    EnterCriticalSection(&g_uiCriticalSection);
    playerGroup->closeAllScenes();
    playerGroup->ClearTooltips();
    Tooltips::ForceTooltipReload(playerId);
    _TutorialPopupInfo popupInfo;
    SetTutorialDescription(playerId, &popupInfo);
    SetMenuDisplayed(playerId, false);
    if (playerGroup == m_groups[1])
        setFullscreenMenuDisplayed(false);
    LeaveCriticalSection(&g_uiCriticalSection);
}

void ConsoleUIController::CloseAllPlayersScenes(bool immediate)
{
    EnterCriticalSection(&g_uiCriticalSection);
    for (int i = 0; i < 6; ++i) {
        m_groups[i]->closeAllScenes();
        m_groups[i]->ClearTooltips();
    }
    for (int player = 0; player < 4; ++player) {
        Tooltips::ForceTooltipReload(player);
        SetMenuDisplayed(player, false);
    }
    setFullscreenMenuDisplayed(false);
    LeaveCriticalSection(&g_uiCriticalSection);
}

bool ConsoleUIController::IsSceneInStack(int playerId, EUIScene scene)
{
    return groupForPlayer(playerId, false)->IsSceneInStack(scene);
}

bool ConsoleUIController::IsContainerMenuDisplayed(int playerId)
{
    return groupForPlayer(playerId, false)->m_containerMenuDisplayed;
}

bool ConsoleUIController::IsIgnorePlayerJoinMenuDisplayed(int playerId)
{
    return groupForPlayer(playerId, false)->m_ignorePlayerJoinMenuDisplayed;
}

bool ConsoleUIController::IsIgnoreAutosaveMenuDisplayed(int playerId)
{
    UIGroup* playerGroup = groupForPlayer(playerId, false);
    return m_groups[1]->m_ignoreAutosaveMenuDisplayed ||
           (playerGroup != m_groups[1] && playerGroup->m_ignoreAutosaveMenuDisplayed);
}

void ConsoleUIController::SetIgnoreAutosaveMenuDisplayed(int playerId, bool ignore)
{
}

void ConsoleUIController::CheckMenuDisplayed()
{
    for (int player = 0; player < 4; ++player) {
        if (!m_menuDisplayPending[player])
            continue;
        if (m_menuDisplayDelay[player] != 0) {
            --m_menuDisplayDelay[player];
            continue;
        }
        m_menuDisplayPending[player] = false;
        m_menuDisplayed[player] = false;
        g_pInput->SetMenuDisplayed(player, false);
    }
}

static UIComponent_Tooltips* GetTooltips(UIGroup* group)
{
    return static_cast<UIComponent_Tooltips*>(group->FindScene((EUIScene)0x46));
}

void ConsoleUIController::AnimateKeyPress(int playerId, int action, bool repeat, bool released,
                                          bool pressed)
{
    if (!released)
        return;
    UIComponent_Tooltips* tooltips = GetTooltips(groupForPlayer(playerId, false));
    if (tooltips != 0) {
        bool handled = false;
        tooltips->handleInput(playerId, action, repeat, released, pressed, handled);
    }
}

void ConsoleUIController::OverrideSFX(int playerId, int action, bool enabled)
{
    UIComponent_Tooltips* tooltips = GetTooltips(groupForPlayer(playerId, false));
    if (tooltips != 0)
        tooltips->overrideSFX(playerId, action, enabled);
}

void ConsoleUIController::SetTooltipText(unsigned int playerId, unsigned int tooltipIndex,
                                         int textId)
{
    UIComponent_Tooltips* tooltips = GetTooltips(groupForPlayer((int)playerId, false));
    if (tooltips != 0)
        tooltips->SetTooltipText(tooltipIndex, textId);
}

void ConsoleUIController::SetEnableTooltips(unsigned int playerId, bool enable)
{
    UIComponent_Tooltips* tooltips = GetTooltips(groupForPlayer((int)playerId, false));
    if (tooltips != 0)
        tooltips->SetEnableTooltips(enable);
}

void ConsoleUIController::ShowTooltip(unsigned int playerId, unsigned int tooltipIndex, bool show)
{
    UIComponent_Tooltips* tooltips = GetTooltips(groupForPlayer((int)playerId, false));
    if (tooltips != 0)
        tooltips->ShowTooltip(tooltipIndex, show);
}

void ConsoleUIController::SetTooltips(unsigned int playerId, int* tooltips, bool show,
                                      bool useGlobalGroup)
{
    if (IsReloadingSkin())
        return;
    UIComponent_Tooltips* component = GetTooltips(groupForPlayer((int)playerId, useGlobalGroup));
    if (component != 0)
        component->SetTooltips((int)playerId, tooltips, show);
}

void ConsoleUIController::SetTooltips(unsigned int playerId, int tooltip0, int tooltip1,
                                      int tooltip2, int tooltip3, int tooltip4, int tooltip5,
                                      int tooltip6, int tooltip7, int tooltip8, int tooltip9,
                                      int tooltip10, int tooltip11, int tooltip12, int tooltip13,
                                      int tooltip14, int tooltip15, bool show, bool useGlobalGroup)
{
    if (IsReloadingSkin())
        return;
    if (tooltip2 == 0x21c0a77d || tooltip2 == (int)0x9eda23db)
        tooltip2 = -1;
    if (tooltip3 == (int)0xaec93217 || tooltip3 == (int)0x85953770)
        tooltip3 = -1;
    UIComponent_Tooltips* component = GetTooltips(groupForPlayer((int)playerId, useGlobalGroup));
    if (component != 0)
        component->SetTooltips((int)playerId, tooltip0, tooltip1, tooltip2, tooltip3, tooltip4,
                               tooltip5, tooltip6, tooltip7, tooltip8, tooltip9, tooltip10,
                               tooltip11, tooltip12, tooltip13, tooltip14, tooltip15, show);
}

void ConsoleUIController::EnableTooltip(unsigned int playerId, unsigned int tooltipIndex,
                                        bool enable)
{
    UIComponent_Tooltips* tooltips = GetTooltips(groupForPlayer((int)playerId, false));
    if (tooltips != 0)
        tooltips->EnableTooltip(tooltipIndex, enable);
}

bool ConsoleUIController::IsToolTipDynamic(unsigned int playerId, unsigned int tooltipIndex,
                                           EControllerActions& action)
{
    UIComponent_Tooltips* tooltips = GetTooltips(groupForPlayer((int)playerId, false));
    return tooltips != 0 && tooltips->IsToolTipDynamic(tooltipIndex, action);
}

void ConsoleUIController::RefreshTooltips(unsigned int playerId)
{
}

void ConsoleUIController::DisplayGamertag(unsigned int playerId, bool show)
{
    UIScene_HUD* hud = static_cast<UIScene_HUD*>(
        groupForPlayer((int)playerId, false)->m_componentQuaternary);
    if (hud != 0)
        hud->ShowDisplayName(show);
}

void ConsoleUIController::SetSelectedItem(unsigned int playerId, const std::wstring& itemName,
                                          const std::wstring& itemDescription)
{
    m_selectedPlayer = (int)playerId;
    m_selectedItemName = itemName;
    m_selectedItemDescription = itemDescription;
    m_selectedItemVisible = true;
}

void ConsoleUIController::ResetSelectedItem()
{
    m_selectedPlayer = 0;
    m_selectedX = 0;
    m_selectedY = 0;
    m_selectedItemVisible = false;
    m_selectedItemName.clear();
    m_selectedItemDescription.clear();
}

void ConsoleUIController::UpdateSelectedItemPos(unsigned int playerId)
{
}

static UIScene_HUD* GetPlayerHUD(UIScene* component)
{
    return static_cast<UIScene_HUD*>(component);
}

int ConsoleUIController::GetHotBarX(unsigned int playerId)
{
    UIScene_HUD* hud = GetPlayerHUD(groupForPlayer((int)playerId, false)->m_componentQuaternary);
    return hud == 0 ? 0 : hud->GetHotBarX();
}

int ConsoleUIController::GetHotBarY(unsigned int playerId)
{
    UIScene_HUD* hud = GetPlayerHUD(groupForPlayer((int)playerId, false)->m_componentQuaternary);
    return hud == 0 ? 0 : hud->GetHotBarY();
}

int ConsoleUIController::GetHotBarWidth(unsigned int playerId)
{
    UIScene_HUD* hud = GetPlayerHUD(groupForPlayer((int)playerId, false)->m_componentQuaternary);
    return hud == 0 ? 0 : hud->GetHotBarWidth();
}

int ConsoleUIController::GetHotBarHeight(unsigned int playerId)
{
    UIScene_HUD* hud = GetPlayerHUD(groupForPlayer((int)playerId, false)->m_componentQuaternary);
    return hud == 0 ? 0 : hud->GetHotBarHeight();
}

bool ConsoleUIController::GetQuickSelectVisible(unsigned int playerId)
{
    UIScene_HUD* hud = GetPlayerHUD(groupForPlayer((int)playerId, false)->m_componentQuaternary);
    return hud != 0 && hud->GetQuickSelectVisible();
}

bool ConsoleUIController::GetDRCEnabled()
{
    UIComponent_WiiUDRC* drc = reinterpret_cast<UIComponent_WiiUDRC*>(
        static_cast<uintptr_t>(m_groups[0]->m_unknown30));
    return drc != 0 && drc->m_drcOutputEnabled;
}

static UIComponent_PressStartToPlay* GetPressStart(UIGroup* group)
{
    return static_cast<UIComponent_PressStartToPlay*>(group->FindScene((EUIScene)0x47));
}

void ConsoleUIController::HandleDLCMountingComplete()
{
    for (int i = 0; i < 6; ++i)
        m_groups[i]->HandleDLCMountingComplete();
}

void ConsoleUIController::HandleDLCInstalled(int packId)
{
    for (int i = 0; i < 6; ++i)
        m_groups[i]->HandleDLCInstalled();
}

void ConsoleUIController::HandleDLCLicenseChange()
{
    for (int i = 0; i < 6; ++i)
        m_groups[i]->HandleDLCLicenseChange();
}

void ConsoleUIController::HandleTMSDLCFileRetrieved(int fileId)
{
}

void ConsoleUIController::HandleTMSBanFileRetrieved(int fileId)
{
    for (int i = 0; i < 6; ++i)
        m_groups[i]->HandleTMSBanFileRetrieved();
}

void ConsoleUIController::HandleInventoryUpdated(int playerId)
{
    groupForPlayer(playerId, false)->HandleMessage((EUIMessage)0, 0);
}

void ConsoleUIController::HandleGameTick()
{
    tickInput();
    if (!IsReloadingSkin()) {
        for (int i = 0; i < 6; ++i) {
            UIScene_HUD* hud = static_cast<UIScene_HUD*>(m_groups[i]->m_componentQuaternary);
            if (hud != 0)
                hud->handleGameTick();
        }
    }
}

void ConsoleUIController::HandleSaveDeviceRemoved(int deviceIndex)
{
    m_groups[1]->HandleSaveDeviceRemoved(deviceIndex);
}

bool ConsoleUIController::IsTutorialVisible(int playerId)
{
    UIComponent_TutorialPopup* popup = static_cast<UIComponent_TutorialPopup*>(
        groupForPlayer(playerId, false)->m_componentSecondary);
    return popup != 0 && popup->IsVisible();
}

void ConsoleUIController::UpdatePlayerBasePositions()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    for (int player = 0; player < 4; ++player) {
        boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = minecraft->mLocalPlayers[player];
        C4JRender::eViewportType viewport = localPlayer != 0
                                                 ? static_cast<C4JRender::eViewportType>(localPlayer->mViewport)
                                                 : static_cast<C4JRender::eViewportType>(0);
        m_groups[player + 2]->SetViewportType(viewport);
        SetMenuDisplayed(player, viewport != (C4JRender::eViewportType)0);
    }
}

void ConsoleUIController::SetEmptyQuadrantLogo(int quadrant)
{
}

void ConsoleUIController::ResetPressStartToPlayUIComponents()
{
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]);
    if (component != 0)
        component->ResetUIComponents();
}

void ConsoleUIController::HideAllGameUIElements()
{
    ResetPressStartToPlayUIComponents();
}

void ConsoleUIController::ShowOtherPlayersBaseScene(unsigned int playerId, bool show)
{
}

void ConsoleUIController::ShowTrialTimer(bool show)
{
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]);
    if (component != 0)
        component->showTrialTimer(show);
}

void ConsoleUIController::SetTrialTimerLimitSecs(unsigned int limitSecs)
{
    m_trialTimerLimit = limitSecs;
}

void ConsoleUIController::UpdateTrialTimer(unsigned int playerId)
{
    unsigned int elapsed = (unsigned int)gApp.getTrialTimer();
    if (elapsed > m_trialTimerLimit)
        elapsed = m_trialTimerLimit;
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]);
    if (component != 0) {
        wchar_t timer[20];
        if (m_trialTimerLimit - elapsed < 180) {
            unsigned int remaining = m_trialTimerLimit - elapsed;
            swprintf(timer, 20, L"%d:%02d", remaining / 60, remaining % 60);
            component->setTrialTimer(std::wstring(timer));
        } else {
            component->setTrialTimer(L"");
        }
    }
    if (elapsed == m_trialTimerLimit && !GetMenuDisplayed((int)playerId)) {
        NavigateToScene((int)playerId, (EUIScene)5, 0, (EUILayer)6, (EUIGroup)7);
        gApp.SetAction((int)playerId, (eXuiAction)0xd, 0);
    }
}

void ConsoleUIController::ReduceTrialTimerValue()
{
    unsigned int elapsed = (unsigned int)gApp.getTrialTimer();
    if (elapsed > m_trialTimerLimit)
        elapsed = m_trialTimerLimit;
    m_trialTimerLimit -= elapsed;
}

void ConsoleUIController::ShowAutosaveCountdownTimer(bool show)
{
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]);
    if (component != 0)
        component->showAutosaveTimer(show);
}

void ConsoleUIController::UpdateAutosaveCountdownTimer(unsigned int remainingSecs)
{
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]);
    if (component != 0) {
        wchar_t text[100];
        swprintf(text, 100, L10N::GetString(0x2a1b8f34), remainingSecs);
        component->setAutosaveTimer(std::wstring(text));
    }
}

void ConsoleUIController::ShowSavingMessage(unsigned int playerId,
                                             C4JStorage::ESavingMessage message)
{
    static const bool visible[3] = { false, true, true };
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]);
    if (component != 0 && (unsigned int)message < 3)
        component->showSaveIcon(visible[(unsigned int)message]);
}

void ConsoleUIController::ShowChestRefillAnimation(bool show)
{
    if (m_groups[1]->IsSceneInStack((EUIScene)0x4b))
        return;
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]);
    if (component != 0) {
        int yOffset = 0;
        UIScene_HUD* hud = static_cast<UIScene_HUD*>(m_groups[2]->m_componentQuaternary);
        if (!show && hud != 0)
            yOffset = hud->GetStatusEffectsYOffset();
        component->ShowChestRefillAnimation(show, yOffset);
    }
}

void ConsoleUIController::HideChestRefillAnimation()
{
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]);
    if (component != 0)
        component->HideChestRefillAnimation();
}

void ConsoleUIController::ShowTimedSplash(unsigned int playerId, const std::wstring& text,
                                          unsigned int durationMs)
{
    UIScene_HUD* hud = static_cast<UIScene_HUD*>(
        m_groups[GetGroup((int)playerId)]->m_componentQuaternary);
    if (hud != 0)
        hud->showTimedSplash(text, durationMs, 1);
}

void ConsoleUIController::ShowPlayerDisplayname(bool show)
{
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]);
    if (component != 0)
        component->showPlayerDisplayName(false);
}

bool ConsoleUIController::PressStartPlaying(unsigned int playerId)
{
    return (m_pressStartPlaying & (1 << playerId)) != 0;
}

void ConsoleUIController::ShowPressStart(unsigned int playerId)
{
    unsigned int mask = 1 << playerId;
    if ((m_pressStartBlocked & mask) == 0) {
        m_pressStartPlaying |= mask;
        UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]);
        if (component != 0)
            component->showPressStart((int)playerId, true);
    }
}

void ConsoleUIController::ClearPressStart()
{
    m_pressStartPlaying = 0;
}

void ConsoleUIController::HidePressStart()
{
    ClearPressStart();
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]);
    if (component != 0)
        for (int player = 0; player < 4; ++player)
            component->showPressStart(player, false);
}

void ConsoleUIController::SetAchievementUnlocked(int playerId, int achievementId, int imageId,
                                                  int value)
{
    unsigned int mask = 1 << playerId;
    if ((m_pressStartBlocked & mask) != 0)
        return;
    m_pressStartPlaying |= mask;
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]);
    if (component != 0)
        component->SetAchievementUnlocked(playerId, achievementId);
}

bool ConsoleUIController::GetAchievementToastShowing(int playerId)
{
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]);
    return component != 0 && component->GetAchievementToastShowing(playerId);
}

bool ConsoleUIController::GetHoldToShowAchievementsActive(bool includeCancelled)
{
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]);
    return component != 0 && component->GetHoldToShowAchievementsActive(includeCancelled);
}

bool ConsoleUIController::GetHoldActionCancelled(int playerId)
{
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]);
    return component != 0 && component->GetHoldActionCancelled(playerId);
}

struct ProfileSignInMenuData
{
    int titleId;
    int messageId;
    wchar_t** messageParameters;
    unsigned int parameterCount;
    unsigned int userIndex;
    int (*callback)(void*, int, int);
    void* callbackData;
    wchar_t* overrideText;
    unsigned int controllerIndex;
};

int ConsoleUIController::RequestProfileSignInMenu(int titleId, int messageId,
                                                   wchar_t** messageParameters,
                                                   unsigned int parameterCount,
                                                   unsigned int userIndex,
                                                   int (*callback)(void*, int, int),
                                                   void* callbackData, wchar_t* overrideText,
                                                   unsigned int controllerIndex)
{
    ProfileSignInMenuData data = { titleId, messageId, messageParameters, parameterCount,
                                   userIndex, callback, callbackData, overrideText,
                                   controllerIndex };
    return NavigateToScene((int)userIndex, (EUIScene)0x50, &data, (EUILayer)3, (EUIGroup)1)
               ? 2 : 1;
}

void ConsoleUIController::RequestAlertMessage(
    int titleId, int messageId, int* messageParameters, unsigned int parameterCount,
    unsigned int userIndex, int (*callback)(void*, int, C4JStorage::EMessageResult),
    void* callbackData, wchar_t* overrideText, bool blocking)
{
    RequestMessageBox(titleId, messageId, messageParameters, parameterCount, userIndex,
                      callback, callbackData, overrideText, 0, true, blocking);
}

int ConsoleUIController::RequestErrorMessage(
    int titleId, int messageId, int* messageParameters, unsigned int parameterCount,
    unsigned int userIndex, int (*callback)(void*, int, C4JStorage::EMessageResult),
    void* callbackData, wchar_t* overrideText, bool blocking)
{
    gApp.setIgnoreMainMenuInputDuringInvite(false);
    return RequestMessageBox(titleId, messageId, messageParameters, parameterCount, userIndex,
                             callback, callbackData, overrideText, 0, false, blocking);
}

int ConsoleUIController::RequestMessageBox(
    int titleId, int messageId, int* messageParameters, unsigned int parameterCount,
    unsigned int userIndex, int (*callback)(void*, int, C4JStorage::EMessageResult),
    void* callbackData, wchar_t* overrideText, unsigned int buttonFlags, bool alert,
    bool blocking)
{
    _QueuedMessageBoxData data;
    data.titleId = titleId;
    data.messageId = messageId;
    data.messageParameters = messageParameters;
    data.parameterCount = parameterCount;
    data.userIndex = userIndex;
    data.callback = callback;
    data.callbackData = callbackData;
    data.overrideText = overrideText;
    data.buttonFlags = buttonFlags;
    data.alert = alert;
    data.blocking = blocking;
    data.type = alert ? 2 : 3;
    if (!IsReloadingSkin()) {
        EUIGroup groupId = !gApp.GetGameStarted() || alert ? (EUIGroup)1 : (EUIGroup)7;
        return NavigateToScene((int)userIndex, (EUIScene)0x4b, &data,
                               (EUILayer)data.type, groupId) ? 2 : 1;
    }
    _QueuedMessageBoxData* queued = new _QueuedMessageBoxData;
    *queued = data;
    queued->messageParameters = new int[parameterCount];
    std::memcpy(queued->messageParameters, messageParameters, parameterCount * sizeof(int));
    EnterCriticalSection(&m_skinMutex);
    m_queuedMessageBoxes.push_back(queued);
    LeaveCriticalSection(&m_skinMutex);
    return 1;
}

void ConsoleUIController::RequestUGCMessageBox(
    int titleId, int messageId, int buttonId,
    int (*callback)(void*, int, C4JStorage::EMessageResult), void* callbackData)
{
    if (titleId == -1)
        titleId = 0x08dc300e;
    if (messageId == -1)
        messageId = (int)0xda0b9859;
    if (buttonId == -1)
        buttonId = gApp.GetPrimaryUserIndex();
    int messageParameter = (int)0xb9a1bf4d;
    RequestAlertMessage(titleId, messageId, &messageParameter, 1, buttonId, callback,
                        callbackData, 0, true);
}

bool ConsoleUIController::IsShowingErrorMessage(int playerId)
{
    return groupForPlayer(playerId, false)->GetTopScene((EUILayer)2) != 0;
}

bool ConsoleUIController::IsShowingAlertMessage(int playerId)
{
    return groupForPlayer(playerId, false)->GetTopScene((EUILayer)3) != 0;
}

void ConsoleUIController::SetWinUserIndex(unsigned int userIndex)
{
    m_winUserIndex = userIndex;
}

unsigned int ConsoleUIController::GetWinUserIndex()
{
    return m_winUserIndex;
}

void ConsoleUIController::ShowUIDebugConsole(bool show)
{
    if (show)
        m_debugConsole = m_groups[1]->addComponent(0, (EUIScene)0x44, (EUILayer)0);
    else {
        m_groups[1]->removeComponent((EUIScene)0x44, (EUILayer)0);
        m_debugConsole = 0;
    }
}

void ConsoleUIController::ShowUIDebugMarketingGuide(bool show)
{
}

bool ConsoleUIController::requestResolutionChange(int resolution)
{
    if (m_resolutionDisableFlags != 0)
        return false;
    if (!TryEnterCriticalSection(&g_uiCriticalSection))
        return false;
    gFui->setResolution(resolution);
    if (resolution == 0) {
        m_screenWidth = 1280.0f;
        m_screenHeight = 720.0f;
    } else {
        m_screenWidth = 1920.0f;
        m_screenHeight = 1080.0f;
    }
    m_groups[2]->ReloadAllLayers();
    m_groups[3]->ReloadAllLayers();
    m_groups[4]->ReloadAllLayers();
    m_groups[5]->ReloadAllLayers();
    m_groups[1]->ReloadAllLayers();
    LeaveCriticalSection(&g_uiCriticalSection);
    return true;
}

void ConsoleUIController::setResolutionChangeDisableFlag(eRESOLUTION_DISABLE_FLAG flag)
{
    m_resolutionDisableFlags |= 1u << (unsigned int)flag;
}

void ConsoleUIController::clearResolutionChangeDisableFlag(eRESOLUTION_DISABLE_FLAG flag)
{
    m_resolutionDisableFlags &= ~(1u << (unsigned int)flag);
}

unsigned int ConsoleUIController::reloadSkinThreadProc(void* context)
{
    ConsoleUIController* controller = static_cast<ConsoleUIController*>(context);
    EnterCriticalSection(&g_uiCriticalSection);
    controller->loadSkins();
    controller->m_groups[2]->ReloadAll();
    controller->m_groups[3]->ReloadAll();
    controller->m_groups[4]->ReloadAll();
    controller->m_groups[5]->ReloadAll();
    controller->m_groups[1]->ReloadAll();
    controller->m_groups[0]->ReloadAll();
    controller->SetEnableTooltips(0, false);
    LeaveCriticalSection(&g_uiCriticalSection);
    return 0;
}

void ConsoleUIController::ReloadSkin()
{
    loadSkins();
    for (int i = 0; i < 6; ++i)
        if (m_groups[i] != 0)
            m_groups[i]->ReloadAll();
    m_skinThreadExpected = false;
}

void ConsoleUIController::StartReloadSkinThread()
{
    m_skinThreadExpected = true;
    ReloadSkin();
}

bool ConsoleUIController::IsReloadingSkin()
{
    return m_skinThreadExpected || m_skinThread != 0;
}

bool ConsoleUIController::IsExpectingOrReloadingSkin()
{
    return IsReloadingSkin() || PendingFontChange();
}

void ConsoleUIController::CleanUpSkinReload()
{
    m_skinThread = 0;
    m_skinThreadExpected = false;
    m_fontChangePending = false;
}

void ConsoleUIController::tickInput()
{
    if (!m_fullscreenMenuDisplayed)
        handleInput();
    ++m_lastInputTime;
}

bool ConsoleUIController::handleKeyPress(unsigned int key, unsigned int character)
{
    bool handled = false;
    for (int i = 0; i < 6 && !handled; ++i)
        if (m_groups[i] != 0)
            m_groups[i]->handleInput(0, (int)key, false, false, true, handled);
    return handled;
}

bool ConsoleUIController::handleInput()
{
    bool handled = false;
    for (int i = 0; i < 6 && !handled; ++i)
        if (m_groups[i] != 0)
            m_groups[i]->handleInput(0, 0, false, false, false, handled);
    return handled;
}

void ConsoleUIController::tick()
{
    SetupFont();
    if (!(!m_cleanupOnReload && !m_autoCleanupOnReload) && !IsReloadingSkin()) {
        CleanUpSkinReload();
        if (!m_cleanupOnReload && CGameNetworkManager::sInstance.IsInSession()) {
            if (m_autoCleanupOnReload)
                CloseAllPlayersScenes(false);
        } else if (g_introRunning != 0) {
            NavigateToScene(g_pApp->GetPrimaryUserIndex(), static_cast<EUIScene>(3), 0,
                            static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
            updateCurrentFont();
            m_autoCleanupOnReload = true;
            m_cleanupOnReload = false;
        } else {
            updateCurrentFont();
            m_autoCleanupOnReload = true;
            m_cleanupOnReload = false;
        }
    }
    for (int i = 0; i < 6; ++i) {
        if (m_playerFlags[i]) {
            if (m_groups[i] != 0) {
                m_groups[i]->closeAllScenes();
                m_groups[i]->ClearTooltips();
            }
            m_playerFlags[i] = false;
        }
    }
    if (m_lastInputTime == 0)
        tickInput();
    m_lastInputTime = 0;
    if (gFui != 0)
        gFui->dispatchEnterFrameEvent();
    for (int i = 0; i < 6; ++i)
        if (m_groups[i] != 0)
            m_groups[i]->tick();
    if (!IsReloadingSkin()) {
        const unsigned long long now = System::processTimeInMilliSecs();
        for (boost::unordered_map<std::wstring, _CachedMovieData>::iterator it = m_movieCache.begin();
             it != m_movieCache.end();) {
            if (it->second.expiryTime < now) {
                delete[] it->second.data.data;
                boost::unordered_map<std::wstring, _CachedMovieData>::iterator expired = it++;
                m_movieCache.erase(expired);
            } else {
                ++it;
            }
        }
    }
    if (gFui != 0)
        gFui->tick();
}

void ConsoleUIController::SendTouchInput(unsigned int playerId, unsigned int action, bool repeat,
                                         bool released, bool pressed)
{
    bool handled = false;
    if (m_groups[1] != 0)
        m_groups[1]->handleInput((int)playerId, (int)action, repeat, released, pressed, handled);
    if (!handled && playerId < 4 && m_groups[GetGroup((int)playerId)] != 0)
        m_groups[GetGroup((int)playerId)]->handleInput((int)playerId, (int)action,
                                                       repeat, released, pressed, handled);
}

void ConsoleUIController::HandleTouchInput(unsigned int playerId, unsigned int action, bool repeat,
                                           bool released, bool pressed)
{
    if (!m_initialized || m_rebuildingTouchBoxes)
        return;
    m_lastInputTime = GetTickCount();
    if (pressed)
        m_touchHeld = true;
    if (released)
        m_touchHeld = false;
    if (m_touchElement != 0)
    {
        UIControl* control = m_touchElement->control;
        const int x = m_touchX - m_touchElement->left;
        const int y = m_touchY - m_touchElement->top;
        if (dynamic_cast<UIControl_DynamicLabel*>(control) != 0)
            static_cast<UIControl_DynamicLabel*>(control)->TouchScroll(y, m_touchHeld);
        else if (dynamic_cast<UIControl_HTMLLabel*>(control) != 0)
            static_cast<UIControl_HTMLLabel*>(control)->TouchScroll(y, m_touchHeld);
        else if (dynamic_cast<UIControl_LeaderboardList*>(control) != 0)
            static_cast<UIControl_LeaderboardList*>(control)->SetTouchFocus(x, y, m_touchHeld);
        else if (dynamic_cast<UIControl_Slider*>(control) != 0 && m_touchElement->right > m_touchElement->left)
            static_cast<UIControl_Slider*>(control)->SetSliderTouchPos(
                (float)x / (float)(m_touchElement->right - m_touchElement->left));
        else if (dynamic_cast<UIControl_TexturePackList*>(control) != 0)
        {
            UIControl_TexturePackList* list = static_cast<UIControl_TexturePackList*>(control);
            if (list->CanTouchTrigger(x, y) != 0)
                list->SetTouchFocus(x, y, m_touchHeld);
        }
        else if (dynamic_cast<UIControl_CheckBox*>(control) != 0)
        {
            UIControl_CheckBox* box = static_cast<UIControl_CheckBox*>(control);
            if (pressed && box->IsEnabled() != 0)
                box->setChecked(box->IsChecked() == 0);
        }
        else if (dynamic_cast<UIControl_List*>(control) != 0)
        {
            UIControl_List* list = static_cast<UIControl_List*>(control);
            if (list->CanTouchFocus(x, y) != 0)
                list->SetTouchFocus(x, y, m_touchHeld);
        }
        else if (dynamic_cast<UIControl_GridList*>(control) != 0)
        {
            UIControl_GridList* list = static_cast<UIControl_GridList*>(control);
            if (list->CanTouchFocus(x, y) != 0)
                list->SetTouchFocus(x, y, m_touchHeld);
        }
    }
    bool handled = false;
    UIGroup* playerGroup = groupForPlayer((int)playerId, false);
    if (playerGroup != 0)
        playerGroup->handleInput((int)playerId, (int)action, repeat, released, pressed, handled);
}

void ConsoleUIController::TouchBoxAdd(UIControl* control, UIScene* scene)
{
    if (control == 0 || scene == 0)
        return;
    scene->updateViewportTouchOffset();
    m_touchElement = 0;
}

void ConsoleUIController::TouchBoxAdd(UIControl* control, EUIGroup groupId, EUILayer layer,
                                      EUIScene scene, UIControl* parent)
{
    if (control == 0 || (int)groupId < 0 || (int)groupId >= 6 || m_groups[(int)groupId] == 0)
        return;
    UIScene* target = m_groups[(int)groupId]->FindScene(scene);
    if (target != 0)
        TouchBoxAdd(control, target);
}

void ConsoleUIController::PlayUISFX(const SoundEvent* soundEvent)
{
    if (soundEvent != 0 && Minecraft::GetInstance()->mSoundEngine != 0)
        Minecraft::GetInstance()->mSoundEngine->playUI(soundEvent, 1.0f, 1.0f);
}

bool ConsoleUIController::TouchBoxHit(UIScene* scene, int x, int y)
{
    m_touchX = x;
    m_touchY = y;
    m_lastTouchElement = m_touchElement;
    m_touchElement = 0;
    return scene != 0;
}

void ConsoleUIController::TouchBoxesClear(UIScene* scene)
{
    if (scene == 0)
        return;
    m_touchElement = 0;
    m_lastTouchElement = 0;
}

void ConsoleUIController::TouchBoxRebuild(UIScene* scene)
{
    m_rebuildingTouchBoxes = true;
    TouchBoxesClear(scene);
    m_rebuildingTouchBoxes = false;
}

void ConsoleUIController::NavigateToHomeMenu(bool immediate)
{
    CloseAllPlayersScenes(immediate);
    NavigateToScene(0, (EUIScene)3, 0, (EUILayer)6, (EUIGroup)7);
}

UIScene* ConsoleUIController::FindScene(EUIScene scene)
{
    for (int i = 0; i < 6; ++i) {
        UIScene* found = m_groups[i]->FindScene(scene);
        if (found != 0)
            return found;
    }
    return 0;
}
