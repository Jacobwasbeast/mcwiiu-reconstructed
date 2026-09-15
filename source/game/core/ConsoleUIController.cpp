#include "game/core/ConsoleUIController.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <cwchar>

#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/client/render/CFontData.hpp"
#include "game/client/render/Renderer.hpp"
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
#include "game/ui/UIString.hpp"
#include "game/ui/UITTFFont.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiBitmapFont.hpp"
#include "game/ui/fuiFile.hpp"
#include "platform/wiiu/C4JThread.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/UIComponent_WiiUDRC.hpp"
#include "vendor/_TutorialPopupInfo.hpp"

extern "C" void OSReport(const char*, ...);
#define UI_TRACE(step) OSReport("[boot][ui] %s\n", step)

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
      m_initialized(false), m_renderOffsetX(0), m_renderOffsetY(0), m_lastViewport(-1),
      m_viewportPositionDirty(true), m_currentFont(0),
      m_pendingFont(EFont_None), m_cleanupOnReload(false), m_autoCleanupOnReload(true),
      m_fontChangePending(false), m_unknown43B(0), m_font(0), m_oldFont(0),
      m_bitmapFont(0), m_bitmapFontAlt(0), m_touchHoldPad(0), m_touchHeld(false), m_touchX(0), m_touchY(0),
      m_rebuildingTouchBoxes(false), m_skinScenes(), m_resolution(0), m_requestedResolution(0),
      m_resolutionDisableFlags(0), m_homeMenuPending(false), m_lastInputTime(0), m_trialTimerLimit(0),
      m_trialTimerValue(0), m_trialTimerTick(0), m_autosaveTimerTick(0),
      m_selectedPlayer(0), m_selectedX(0), m_selectedY(0), m_selectedItemVisible(false),
      m_winUserIndex(0), m_skinThreadExpected(false), m_skinThread(0), m_nextCallbackId(0),
      m_touchOffsetX(0), m_touchOffsetY(0), m_touchElement(0), m_lastTouchElement(0),
      m_lastUISFXTimeHigh(0), m_lastUISFXTimeLow(0),
      m_pressStartPlaying(0), m_pressStartBlocked(0), m_fullscreenMenuDisplayed(false),
      m_customDrawLeft(0), m_customDrawTop(0), m_customDrawRight(0), m_customDrawBottom(0)
{
    std::memset(m_actionTimer, 0, sizeof(m_actionTimer));
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
    UI_TRACE("postInit: entered");
    gFui->setCustomTextureFn(TextureSubstitutionCreateCallbackFui,
                             TextureSubstitutionDestroyCallbackFui, this);
    UI_TRACE("postInit: customTextureFn set");
    SetupFont();
    UI_TRACE("postInit: SetupFont done");
    loadSkins();
    UI_TRACE("postInit: loadSkins DONE");
    m_groups[0] = new UIGroup((EUIGroup)0, -1);
    m_groups[1] = new UIGroup((EUIGroup)1, -1);
    m_groups[2] = new UIGroup((EUIGroup)2, 0);
    m_groups[3] = new UIGroup((EUIGroup)3, 1);
    m_groups[4] = new UIGroup((EUIGroup)4, 2);
    m_groups[5] = new UIGroup((EUIGroup)5, 3);
    UI_TRACE("postInit: groups built");
    NavigateToScene(0, (EUIScene)1, 0, (EUILayer)6, (EUIGroup)7);
}

void ConsoleUIController::init(int width, int height)
{
    UI_TRACE("init: entered");
    gFui->setScreenSize((float)width, (float)height);
    UI_TRACE("init: screen size set");
    preInit(width, height);
    UI_TRACE("init: preInit done");
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
    if (font == EFont_Bitmap)
        return new UITTFFont("Mojangles_TTF_bitmap", "Common/Media/font/JPN/DFGMaruGothic-Md.ttf",
                             "Common/Media/font/KOR/BOKMSD.ttf",
                             "Common/Media/font/CHT/DFHeiMedium-B5.ttf", 0);
    if (font == EFont_Japanese)
        return new UITTFFont("Mojangles_TTF_jaJP", "Common/Media/font/JPN/DFGMaruGothic-Md.ttf",
                             "Common/Media/font/KOR/BOKMSD.ttf",
                             "Common/Media/font/CHT/DFHeiMedium-B5.ttf", 0x3f);
    if (font == EFont_ChineseSimplified)
        return new UITTFFont("Mojangled_TTF_cnCN", "Common/Media/font/CHS/MSYH.ttf", 0x3f);
    if (font == EFont_ChineseTraditional)
        return new UITTFFont("Mojangles_TTF_cnTD", "Common/Media/font/CHT/DFHeiMedium-B5.ttf",
                             "Common/Media/font/KOR/BOKMSD.ttf",
                             "Common/Media/font/JPN/DFGMaruGothic-Md.ttf", 0x3f);
    if (font == EFont_Korean)
        return new UITTFFont("Mojangles_TTF_koKR", "Common/Media/font/KOR/BOKMSD.ttf",
                             "Common/Media/font/JPN/DFGMaruGothic-Md.ttf",
                             "Common/Media/font/CHT/DFHeiMedium-B5.ttf", 0x3f);
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
    EnterCriticalSection(&m_fontMutex);
    const int primaryPad = gProfile->GetPrimaryPad();
    if (m_fontChangePending) {
        if (m_currentFont != m_pendingFont) {
            gApp.SetAction(primaryPad, static_cast<eXuiAction>(0x26), NULL);
            m_fontChangePending = gApp.GetXuiAction(primaryPad) != 0x26;
            LeaveCriticalSection(&m_fontMutex);
            return;
        }
        m_fontChangePending = false;
    }
    if (m_currentFont == m_pendingFont && UIString::setCurrentLanguage() != 0) {
        m_pendingFont = getFontForLanguage(static_cast<int>(UIString::getCurrentLanguage()));
        gApp.mDLCManager.LanguageChanged();
        L10N::loadStringTable();
        if (m_pendingFont == m_currentFont) {
            if (gApp.GetGameStarted() != 0) {
                gApp.SetAction(primaryPad, static_cast<eXuiAction>(0x26), NULL);
                LeaveCriticalSection(&m_fontMutex);
                return;
            }
        } else {
            UITTFFont* replaced = m_font;
            if (replaced != 0) {
                m_font = 0;
                m_oldFont = replaced;
            }
            if (m_bitmapFont == 0) {
                m_bitmapFont = new fuiBitmapFont(g_mojangles7FontData, 0x3f);
                m_bitmapFont->registerFont();
            }
            if (m_bitmapFontAlt == 0) {
                m_bitmapFontAlt = new fuiBitmapFont(g_mojangles11FontData, 0x3f);
                m_bitmapFontAlt->registerFont();
            }
            if (m_pendingFont == EFont_Bitmap) {
                m_bitmapFont->registerFont();
                m_bitmapFontAlt->registerFont();
                m_font = createFont(m_pendingFont);
                gFui->setFontIndirection("Mojangles7", m_font->getFontName().c_str());
                gFui->setFontIndirection("Mojangles11", m_font->getFontName().c_str());
            } else if (m_pendingFont != EFont_None) {
                m_font = createFont(m_pendingFont);
                gFui->setFontIndirection("Mojangles7", m_font->getFontName().c_str());
                gFui->setFontIndirection("Mojangles11", m_font->getFontName().c_str());
            }
            if (m_currentFont != 0) {
                gApp.SetAction(primaryPad, static_cast<eXuiAction>(0x26), NULL);
                bool stillPending = true;
                if (!m_fontChangePending && gApp.GetXuiAction(primaryPad) == 0x26)
                    stillPending = false;
                m_fontChangePending = stillPending;
                LeaveCriticalSection(&m_fontMutex);
                return;
            }
            updateCurrentFont();
        }
    }
    LeaveCriticalSection(&m_fontMutex);
}

int ConsoleUIController::loadSkin(const std::wstring& name, const std::wstring& path, int fileIndex)
{
    (void)path;
    int scene = 0;
    if (name.size() != 0 && gApp.hasArchiveFile(name, false)) {
        arrayWithLength<unsigned char> bytes = gApp.getArchiveFile(name, false);
        scene = gFui->load(bytes, false, fileIndex);
        delete[] bytes.data;
    }
    return scene;
}

void ConsoleUIController::loadSkins()
{
    std::wstring platformSkin = L"";
    platformSkin = L"skinWiiU.fui";
    if (m_screenHeight >= 1080.0f) {
        m_skinScenes[0] = loadSkin(platformSkin, L"platformskinHD.fui", 0);
    } else {
        m_skinScenes[0] = loadSkin(platformSkin, L"platformskin.fui", 0);
    }
    m_skinScenes[1] = loadSkin(L"skinGraphics.fui", L"skinGraphics.fui", 0);
    m_skinScenes[2] = loadSkin(L"skinGraphicsHud.fui", L"skinGraphicsHud.fui", 0);
    m_skinScenes[3] = loadSkin(L"skinGraphicsInGame.fui", L"skinGraphicsInGame.fui", 0);
    m_skinScenes[5] = loadSkin(L"skinGraphicsLabels.fui", L"skinGraphicsLabels.fui", 0);
    m_skinScenes[6] = loadSkin(L"skinLabels.fui", L"skinLabels.fui", 0);
    m_skinScenes[7] = loadSkin(L"skinInGame.fui", L"skinInGame.fui", 0);
    m_skinScenes[8] = loadSkin(L"skinHud.fui", L"skinHud.fui", 0);
    m_skinScenes[10] = loadSkin(L"skin.fui", L"skin.fui", 0);
}

void ConsoleUIController::renderScenes()
{
    PIXBeginNamedEvent(0.0f, "Rendering FUI scenes");
    if (!gApp.GetGameStarted())
    {
        PIXBeginNamedEvent(0.0f, "Rendering fullscreen scenes");
        m_groups[1]->render();
        PIXEndNamedEvent();
        PIXBeginNamedEvent(0.0f, "Rendering DRC scenes");
        gRenderer->StartDRCFrame(GetDRCEnabled());
        gFui->setScreenSize(854.0f, 480.0f);
        if (!GetDRCEnabled())
        {
            const float clearColour[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            gRenderer->SetClearColour(clearColour);
            gRenderer->Clear(3);
        }
    }
    else
    {
        if (!m_groups[1]->hidesLowerScenes())
        {
            PIXBeginNamedEvent(0.0f, "Rendering layer %d scenes", 2);
            m_groups[2]->render();
            PIXEndNamedEvent();
            PIXBeginNamedEvent(0.0f, "Rendering layer %d scenes", 3);
            m_groups[3]->render();
            PIXEndNamedEvent();
            PIXBeginNamedEvent(0.0f, "Rendering layer %d scenes", 4);
            m_groups[4]->render();
            PIXEndNamedEvent();
            PIXBeginNamedEvent(0.0f, "Rendering layer %d scenes", 5);
            m_groups[5]->render();
            PIXEndNamedEvent();
        }
        PIXBeginNamedEvent(0.0f, "Rendering fullscreen scenes");
        m_groups[1]->render();
        PIXEndNamedEvent();
        PIXBeginNamedEvent(0.0f, "Rendering DRC scenes");
        gRenderer->StartDRCFrame(GetDRCEnabled());
        gFui->setScreenSize(854.0f, 480.0f);
        if (!GetDRCEnabled())
        {
            const float clearColour[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            gRenderer->SetClearColour(clearColour);
            gRenderer->Clear(3);
        }
    }
    m_groups[0]->render();
    gFui->setScreenSize(1280.0f, 720.0f);
    PIXEndNamedEvent();
    PIXEndNamedEvent();
}

void ConsoleUIController::render()
{
    gRenderer->Clear(1);
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
    gFui->setOrigin((float)x, (float)y);
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
    if (!m_viewportPositionDirty && m_lastViewport == (int)viewport)
        return;
    m_lastViewport = (int)viewport;
    m_viewportPositionDirty = false;
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
    m_renderOffsetX = x;
    m_renderOffsetY = y;
    gFui->setOrigin((float)x, (float)y);
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
    if (!displayed)
    {
        if (playerId == 0xff)
        {
            for (int i = 0; i < 4; ++i)
            {
                m_menuDisplayPending[i] = true;
                m_menuDisplayDelay[i] = 10;
            }
            return;
        }
        m_menuDisplayPending[playerId] = true;
        m_menuDisplayDelay[playerId] = 10;
        return;
    }
    if (playerId == 0xff)
    {
        for (int i = 0; i < 4; ++i)
        {
            g_pInput->SetMenuDisplayed(i, true);
            m_menuDisplayed[i] = true;
            m_menuDisplayPending[i] = false;
        }
        return;
    }
    g_pInput->SetMenuDisplayed(playerId, true);
    m_menuDisplayed[playerId] = true;
    m_menuDisplayPending[playerId] = false;
}

void ConsoleUIController::setFullscreenMenuDisplayed(bool displayed)
{
    if (m_groups[1] != 0)
        m_groups[1]->showComponent((int)m_winUserIndex, (EUIScene)0x46, (EUILayer)0, displayed);
    for (int i = 2; i < 6; ++i)
        if (m_groups[i] != 0)
            m_groups[i]->showComponent(i - 2, (EUIScene)0x46, (EUILayer)0, !displayed);
}

bool ConsoleUIController::NavigateToScene(int playerId, EUIScene scene, void* sceneData,
                                          EUILayer layer, EUIGroup groupId)
{
    if (GetMenuDisplayed(playerId))
    {
        unsigned int sceneValue = (unsigned int)scene;
        if (sceneValue < 0x2e)
        {
            if (4 < sceneValue &&
                (sceneValue < 0xd || (0x1c < sceneValue && (sceneValue < 0x1f || sceneValue == 0x2a))))
                return false;
        }
        else
        {
            if (sceneValue < 0x30)
                return false;
            if (0x31 < sceneValue)
            {
                if (sceneValue < 0x34)
                    return false;
                if (0x34 < sceneValue)
                {
                    if (sceneValue < 0x38)
                        return false;
                    if (0x3b < sceneValue && sceneValue < 0x3e)
                        return false;
                }
            }
        }
    }

    PerformanceTimer timer;
    int menuPlayer = 0xff;
    bool navigated;
    if ((int)scene == 4)
    {
        groupId = (EUIGroup)1;
        layer = (EUILayer)4;
        EnterCriticalSection(&m_renderMutex);
        SetMenuDisplayed(menuPlayer, true);
        navigated = m_groups[1]->NavigateToScene(playerId, scene, sceneData, layer);
    }
    else if ((int)scene == 0x30)
    {
        groupId = (EUIGroup)1;
        EnterCriticalSection(&m_renderMutex);
        SetMenuDisplayed(menuPlayer, true);
        navigated = m_groups[1]->NavigateToScene(playerId, scene, sceneData, (EUILayer)6);
    }
    else
    {
        if ((int)scene == 0x1f)
            layer = (EUILayer)4;
        if ((int)groupId == 7)
        {
            if (gApp.GetGameStarted())
            {
                if (playerId == 0xff || (unsigned int)playerId > 3)
                    groupId = (EUIGroup)1;
                else
                {
                    groupId = (EUIGroup)(playerId + 2);
                    menuPlayer = playerId;
                }
            }
            else
            {
                groupId = (EUIGroup)1;
                layer = (EUILayer)4;
            }
        }
        EnterCriticalSection(&m_renderMutex);
        SetMenuDisplayed(menuPlayer, true);
        navigated = m_groups[(int)groupId]->NavigateToScene(playerId, scene, sceneData, layer);
    }
    if (navigated && (int)groupId == 1)
        setFullscreenMenuDisplayed(true);
    LeaveCriticalSection(&m_renderMutex);
    timer.PrintElapsedTime(L"Navigate to scene");
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
    {
        if (gApp.GetGameStarted())
        {
            groupId = (EUIGroup)GetGroup(playerId);
            return m_groups[(int)groupId]->GetTopScene(layer);
        }
        groupId = (EUIGroup)1;
        layer = (EUILayer)4;
    }
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
    int groupIndex;
    UIGroup* playerGroup;
    if (!gApp.GetGameStarted() && !immediate)
    {
        groupIndex = 1;
        EnterCriticalSection(&g_uiCriticalSection);
        playerGroup = m_groups[1];
    }
    else
    {
        groupIndex = GetGroup(playerId);
        EnterCriticalSection(&g_uiCriticalSection);
        playerGroup = m_groups[groupIndex];
    }
    playerGroup->closeAllScenes();
    playerGroup->ClearTooltips();
    Tooltips::ForceTooltipReload(playerId);
    if (playerGroup->m_componentSecondary != 0)
    {
        _TutorialPopupInfo popupInfo;
        static_cast<UIComponent_TutorialPopup*>(playerGroup->m_componentSecondary)
            ->SetTutorialDescription(&popupInfo);
    }
    if (groupIndex == 1)
        setFullscreenMenuDisplayed(false);
    bool sharedDisplayed = m_groups[1]->GetMenuDisplayed() != 0;
    SetMenuDisplayed(0, sharedDisplayed || m_groups[2]->GetMenuDisplayed() != 0);
    SetMenuDisplayed(1, sharedDisplayed || m_groups[3]->GetMenuDisplayed() != 0);
    SetMenuDisplayed(2, sharedDisplayed || m_groups[4]->GetMenuDisplayed() != 0);
    SetMenuDisplayed(3, sharedDisplayed || m_groups[5]->GetMenuDisplayed() != 0);
    LeaveCriticalSection(&g_uiCriticalSection);
}

void ConsoleUIController::CloseAllPlayersScenes(bool immediate)
{
    EnterCriticalSection(&g_uiCriticalSection);
    m_groups[1]->ClearTooltips();
    m_groups[0]->closeAllScenes();
    m_groups[0]->ClearTooltips();
    m_groups[1]->closeAllScenes();
    m_groups[1]->ClearTooltips();
    m_groups[2]->closeAllScenes();
    m_groups[2]->ClearTooltips();
    m_groups[3]->closeAllScenes();
    m_groups[3]->ClearTooltips();
    m_groups[4]->closeAllScenes();
    m_groups[4]->ClearTooltips();
    m_groups[5]->closeAllScenes();
    m_groups[5]->ClearTooltips();
    if (!m_groups[1]->GetMenuDisplayed())
    {
        SetMenuDisplayed(0, false);
        SetMenuDisplayed(1, false);
        SetMenuDisplayed(2, false);
        SetMenuDisplayed(3, false);
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

static UIComponent_Tooltips* GetTooltips(UIScene* component)
{
    return static_cast<UIComponent_Tooltips*>(component);
}

void ConsoleUIController::AnimateKeyPress(int playerId, int action, bool repeat, bool released,
                                          bool pressed)
{
    if (!released)
        return;
    UIGroup* group = (!gApp.GetGameStarted() || playerId == 0xff || playerId > 3)
        ? m_groups[1] : m_groups[playerId + 2];
    UIComponent_Tooltips* tooltips = GetTooltips(group->m_component);
    if (tooltips != 0) {
        bool handled = false;
        tooltips->handleInput(playerId, action, repeat, released, pressed, handled);
    }
}

void ConsoleUIController::OverrideSFX(int playerId, int action, bool enabled)
{
    UIComponent_Tooltips* tooltips = GetTooltips(groupForPlayer(playerId, false)->m_component);
    if (tooltips != 0)
        tooltips->overrideSFX(playerId, action, enabled);
}

void ConsoleUIController::SetTooltipText(unsigned int playerId, unsigned int tooltipIndex,
                                         int textId)
{
    UIComponent_Tooltips* tooltips = GetTooltips(groupForPlayer((int)playerId, false)->m_component);
    if (tooltips != 0)
        tooltips->SetTooltipText(tooltipIndex, textId);
}

void ConsoleUIController::SetEnableTooltips(unsigned int playerId, bool enable)
{
    int groupId = 1;
    if (gApp.GetGameStarted())
        groupId = GetGroup((int)playerId);
    UIComponent_Tooltips* tooltips = GetTooltips(m_groups[groupId]->m_component);
    if (tooltips != 0)
        tooltips->SetEnableTooltips(enable);
}

void ConsoleUIController::ShowTooltip(unsigned int playerId, unsigned int tooltipIndex, bool show)
{
    UIComponent_Tooltips* tooltips = GetTooltips(groupForPlayer((int)playerId, false)->m_component);
    if (tooltips != 0)
        tooltips->ShowTooltip(tooltipIndex, show);
}

void ConsoleUIController::SetTooltips(unsigned int playerId, int* tooltips, bool show,
                                      bool useGlobalGroup)
{
    if (IsReloadingSkin())
        return;
    UIComponent_Tooltips* component = GetTooltips(groupForPlayer((int)playerId, useGlobalGroup)->m_component);
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
    UIComponent_Tooltips* component = GetTooltips(groupForPlayer((int)playerId, useGlobalGroup)->m_component);
    if (component != 0)
        component->SetTooltips((int)playerId, tooltip0, tooltip1, tooltip2, tooltip3, tooltip4,
                               tooltip5, tooltip6, tooltip7, tooltip8, tooltip9, tooltip10,
                               tooltip11, tooltip12, tooltip13, tooltip14, tooltip15, show);
}

void ConsoleUIController::EnableTooltip(unsigned int playerId, unsigned int tooltipIndex,
                                        bool enable)
{
    UIComponent_Tooltips* tooltips = GetTooltips(groupForPlayer((int)playerId, false)->m_component);
    if (tooltips != 0)
        tooltips->EnableTooltip(tooltipIndex, enable);
}

bool ConsoleUIController::IsToolTipDynamic(unsigned int playerId, unsigned int tooltipIndex,
                                           EControllerActions& action)
{
    UIComponent_Tooltips* tooltips = GetTooltips(groupForPlayer((int)playerId, false)->m_component);
    return tooltips != 0 && tooltips->IsToolTipDynamic(tooltipIndex, action);
}

void ConsoleUIController::RefreshTooltips(unsigned int playerId)
{
}

void ConsoleUIController::DisplayGamertag(unsigned int playerId, bool show)
{
    UIGroup* group = m_groups[playerId + 2];
    UIScene_HUD* hud = static_cast<UIScene_HUD*>(group->m_componentQuaternary);
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
    for (int i = 0; i < 6; ++i)
    {
        UIScene_HUD* hud = static_cast<UIScene_HUD*>(m_groups[i]->m_componentQuaternary);
        if (hud != 0)
            hud->SetSelectedLabel(std::wstring());
    }
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
    UIComponent_WiiUDRC* drc = m_groups[0]->m_drcComponent;
    return drc != 0 && drc->m_drcOutputEnabled;
}

static UIComponent_PressStartToPlay* GetPressStart(UIScene* component)
{
    return static_cast<UIComponent_PressStartToPlay*>(component);
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
    if (!IsExpectingOrReloadingSkin()) {
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
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]->m_componentTertiary);
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
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]->m_componentTertiary);
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
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]->m_componentTertiary);
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
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]->m_componentTertiary);
    if (component != 0)
        component->showAutosaveTimer(show);
}

void ConsoleUIController::UpdateAutosaveCountdownTimer(unsigned int remainingSecs)
{
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]->m_componentTertiary);
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
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]->m_componentTertiary);
    if (component != 0 && (unsigned int)message < 3)
        component->showSaveIcon(visible[(unsigned int)message]);
}

void ConsoleUIController::ShowChestRefillAnimation(bool show)
{
    if (m_groups[1]->IsSceneInStack((EUIScene)0x4b))
        return;
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]->m_componentTertiary);
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
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]->m_componentTertiary);
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
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]->m_componentTertiary);
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
        UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]->m_componentTertiary);
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
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]->m_componentTertiary);
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
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]->m_componentTertiary);
    if (component != 0)
        component->SetAchievementUnlocked(playerId, achievementId);
}

bool ConsoleUIController::GetAchievementToastShowing(int playerId)
{
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]->m_componentTertiary);
    return component != 0 && component->GetAchievementToastShowing(playerId);
}

bool ConsoleUIController::GetHoldToShowAchievementsActive(bool includeCancelled)
{
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]->m_componentTertiary);
    return component != 0 && component->GetHoldToShowAchievementsActive(includeCancelled);
}

bool ConsoleUIController::GetHoldActionCancelled(int playerId)
{
    UIComponent_PressStartToPlay* component = GetPressStart(m_groups[1]->m_componentTertiary);
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
    _MessageBoxData data;
    data.titleId = titleId;
    data.messageId = messageId;
    data.messageParameters = messageParameters;
    data.parameterCount = parameterCount;
    data.userIndex = userIndex;
    data.callback = callback;
    data.callbackData = callbackData;
    data.overrideText = overrideText;
    data.buttonFlags = buttonFlags;
    data.blocking = blocking;
    if (!IsReloadingSkin()) {
        EUIGroup groupId = !gApp.GetGameStarted() || alert ? (EUIGroup)1 : (EUIGroup)7;
        return NavigateToScene((int)userIndex, (EUIScene)0x4b, &data,
                               alert ? (EUILayer)2 : (EUILayer)3, groupId) ? 2 : 1;
    }
    _QueuedMessageBoxData* queued = new _QueuedMessageBoxData;
    static_cast<_MessageBoxData&>(*queued) = data;
    queued->queuedUserIndex = userIndex;
    queued->type = 2;
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
    m_groups[0]->DestroyAll();
    m_groups[1]->DestroyAll();
    m_groups[2]->DestroyAll();
    m_groups[3]->DestroyAll();
    m_groups[4]->DestroyAll();
    m_groups[5]->DestroyAll();
    for (int i = 0; i < 11; ++i)
    {
        if (m_skinScenes[i] != 0)
        {
            gFui->unloadScene((fuiFile*)m_skinScenes[i]);
            m_skinScenes[i] = 0;
        }
    }
    NavigateToScene(0, (EUIScene)0x4c, 0, (EUILayer)1, (EUIGroup)1);
}

void ConsoleUIController::StartReloadSkinThread()
{
    if (m_skinThread != 0)
        m_skinThread->Run();
}

bool ConsoleUIController::IsReloadingSkin()
{
    EnterCriticalSection(&m_skinThreadMutex);
    bool result = m_skinThreadExpected || m_skinThread != 0;
    LeaveCriticalSection(&m_skinThreadMutex);
    return result;
}

bool ConsoleUIController::IsExpectingOrReloadingSkin()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mTexturePackRepository->needsUIUpdate())
        return true;
    return IsReloadingSkin() || PendingFontChange();
}

void ConsoleUIController::CleanUpSkinReload()
{
    EnterCriticalSection(&m_skinThreadMutex);
    m_skinThread = 0;
    LeaveCriticalSection(&m_skinThreadMutex);

    EnterCriticalSection(&m_touchMutex);
    for (std::vector<_QueuedMessageBoxData*>::iterator it = m_queuedMessageBoxes.begin();
         it != m_queuedMessageBoxes.end(); ++it)
    {
        _QueuedMessageBoxData* queued = *it;
        NavigateToScene(queued->queuedUserIndex, (EUIScene)0x4b, queued,
                        (EUILayer)queued->type, (EUIGroup)1);
        delete queued;
    }
    m_queuedMessageBoxes.clear();
    LeaveCriticalSection(&m_touchMutex);
}

void ConsoleUIController::tickInput()
{
    if (!m_skinThreadExpected)
    {
        handleInput();
        ++m_lastInputTime;
    }
}

bool ConsoleUIController::handleKeyPress(unsigned int key, unsigned int character)
{
    bool handled = false;
    if (character == 0x14)
    {
        bool newTouch = false;
        bool touchRepeat = false;
        bool touchReleased = false;
        bool touchPressed = false;
        if (m_touchHeld && key > 4 && g_pInput->ButtonPressed((int)key, 0x14, 0) != 0)
            m_touchHeld = false;
        unsigned short* touchData = g_pInput->GetTouchPadData((int)key, C_4JInput::ETouchType_0, true);
        int touchX = 0;
        int touchY = 0;
        bool touchDataValid = false;
        if (touchData != 0 && touchData[2] == 1 && touchData[3] == 0)
        {
            touchX = touchData[0];
            touchY = touchData[1];
            touchDataValid = true;
        }
        if (!IsReloadingSkin() && !m_rebuildingTouchBoxes && touchData != 0 && GetDRCEnabled())
        {
            if (touchDataValid)
            {
                if (!m_touchHeld)
                {
                    m_touchElement = 0;
                    m_lastTouchElement = 0;
                    UIGroup* touchGroup = m_groups[1];
                    UIScene* touchScene = touchGroup->getCurrentScene((int)key);
                    if (touchScene == 0)
                    {
                        touchGroup = key < 4 ? m_groups[2 + key] : m_groups[2];
                        touchScene = touchGroup->getCurrentScene((int)key);
                    }
                    bool hitFound = false;
                    if (touchScene != 0)
                        hitFound = TouchBoxHit(touchScene, touchX, touchY);
                    if (!hitFound && touchGroup->m_component != 0)
                        hitFound = TouchBoxHit(touchGroup->m_component, touchX, touchY);
                    if (hitFound)
                    {
                        touchRepeat = true;
                        m_touchHoldPad = (int)key;
                        newTouch = true;
                        m_touchHeld = true;
                        m_actionTimer[key][0x14] = GetTickCount() + 300;
                    }
                }
                else if (m_touchHoldPad == (int)key)
                {
                    UIGroup* touchGroup = m_groups[1];
                    UIScene* touchScene = touchGroup->getCurrentScene((int)key);
                    if (touchScene == 0)
                    {
                        touchGroup = key < 4 ? m_groups[2 + key] : m_groups[2];
                        touchScene = touchGroup->getCurrentScene((int)key);
                    }
                    bool hitFound = false;
                    if (touchScene != 0)
                        hitFound = TouchBoxHit(touchScene, touchX, touchY);
                    if (!hitFound && touchGroup->m_component != 0)
                        hitFound = TouchBoxHit(touchGroup->m_component, touchX, touchY);
                    if (hitFound)
                    {
                        newTouch = true;
                        unsigned int now = GetTickCount();
                        if (m_actionTimer[key][0x14] != 0 &&
                            (unsigned int)m_actionTimer[key][0x14] < now)
                        {
                            touchReleased = true;
                            touchRepeat = true;
                            m_actionTimer[key][0x14] = now + 100;
                        }
                    }
                }
            }
            else if (m_touchHeld && m_touchHoldPad == (int)key)
            {
                newTouch = true;
                m_touchHeld = false;
                touchPressed = true;
                m_actionTimer[key][0x14] = 0;
            }
            HandleTouchInput(key, 0x14, touchRepeat, touchReleased, touchPressed);
            if (newTouch)
                return newTouch;
        }
    }

    unsigned int actionByte = character & 0xff;
    bool buttonDown = g_pInput->ButtonDown((int)key, actionByte) != 0;
    bool buttonPressed = g_pInput->ButtonPressed((int)key, actionByte, 0) != 0;
    bool buttonReleased = g_pInput->ButtonReleased((int)key, actionByte) != 0;
    bool repeatFired = false;

    if (buttonPressed)
    {
        m_actionTimer[key][character] = GetTickCount() + 300;
    }
    else if (buttonReleased)
    {
        m_actionTimer[key][character] = 0;
    }
    else if (buttonDown)
    {
        unsigned int now = GetTickCount();
        if (m_actionTimer[key][character] != 0 &&
            (unsigned int)m_actionTimer[key][character] < now)
        {
            m_actionTimer[key][character] = now + 100;
            repeatFired = true;
            buttonReleased = true;
        }
    }

    if (!repeatFired && !buttonPressed && !buttonReleased)
        return handled;

    UIComponent_PressStartToPlay* pressStart =
        static_cast<UIComponent_PressStartToPlay*>(m_groups[1]->m_componentTertiary);
    unsigned int toastShowing = pressStart->GetAchievementToastShowing((int)key);
    if ((toastShowing != 0 || pressStart->GetHoldToShowAchievementsActive(false)) &&
        !IsSceneInStack((int)key, (EUIScene)5) &&
        !pressStart->handleHold((int)key, (int)character, repeatFired, buttonReleased, buttonPressed,
                                handled))
    {
        buttonReleased = true;
    }

    if (!handled)
    {
        m_groups[1]->handleInput((int)key, (int)character, repeatFired, buttonReleased, buttonPressed,
                                 handled);
        if (!handled && key < 4)
            m_groups[2 + key]->handleInput((int)key, (int)character, repeatFired, buttonReleased,
                                           buttonPressed, handled);
    }
    return handled;
}

bool ConsoleUIController::handleInput()
{
    for (int pad = 0; pad < 4; ++pad)
    {
        if (g_pInput->IsPadConnected(pad) != 0 && g_pInput->IsPadLocked(pad) != 0)
        {
            for (unsigned int action = 0; action <= 0x17; ++action)
                handleKeyPress((unsigned int)pad, action);
        }
    }
    for (int pad = 4; pad < 9; ++pad)
    {
        if (g_pInput->IsPadConnected(pad) != 0 && g_pInput->IsPadDRCController(pad) != 0)
        {
            UIComponent_WiiUDRC* drc = m_groups[0]->m_drcComponent;
            if (drc != 0)
                drc->handleDRCTouchInput((unsigned int)pad);
        }
        if (!gApp.GetGameStarted() || gApp.m_signInResult != 0)
        {
            if (g_pInput->IsPadLocked(pad) == 0 && g_pInput->IsPadConnected(pad) != 0)
            {
                for (unsigned int action = 0; action <= 0x17; ++action)
                    handleKeyPress((unsigned int)pad, action);
            }
        }
    }
    return false;
}

void ConsoleUIController::tick()
{
    SetupFont();
    if ((m_homeMenuPending || m_cleanupOnReload) && !IsReloadingSkin()) {
        CleanUpSkinReload();
        if (!m_homeMenuPending && CGameNetworkManager::sInstance.IsInSession()) {
            if (m_autoCleanupOnReload)
                CloseAllPlayersScenes(false);
        } else if (g_introRunning != 0) {
            NavigateToScene(gProfile->GetPrimaryPad(), static_cast<EUIScene>(3), 0,
                            static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
            updateCurrentFont();
            m_autoCleanupOnReload = true;
            m_homeMenuPending = false;
            m_cleanupOnReload = false;
        }
        updateCurrentFont();
        m_autoCleanupOnReload = true;
        m_homeMenuPending = false;
        m_cleanupOnReload = false;
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
    if (m_rebuildingTouchBoxes)
        return;
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
    if (m_lastTouchElement != 0 && m_lastTouchElement->control != 0)
    {
        UIControl* control = m_lastTouchElement->control;
        const int x = m_touchX - m_lastTouchElement->left;
        const int y = m_touchY - m_lastTouchElement->top;
        bool triggered = false;
        if (dynamic_cast<UIControl_TexturePackList*>(control) != 0)
            triggered = static_cast<UIControl_TexturePackList*>(control)->CanTouchTrigger(x, y) != 0;
        else if (dynamic_cast<UIControl_List*>(control) != 0)
            triggered = static_cast<UIControl_List*>(control)->CanTouchTrigger(x, y);
        else if (dynamic_cast<UIControl_GridList*>(control) != 0)
            triggered = static_cast<UIControl_GridList*>(control)->CanTouchTrigger(x, y);
        if (triggered)
            pressed = true;
    }
    if (repeat || released || pressed)
        SendTouchInput(playerId, action, repeat, released, pressed);
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
    UIELEMENT* elem = new UIELEMENT;
    elem->control = control;
    double width = control->getWidth();
    double height = control->getHeight();
    double parentX = 0.0;
    double parentY = 0.0;
    if (parent != 0)
    {
        parentX = parent->getXPos();
        parentY = parent->getYPos();
    }
    if (dynamic_cast<UIControl_Slider*>(control) != 0)
    {
        width = static_cast<UIControl_Slider*>(control)->GetRealWidth();
    }
    else if (dynamic_cast<UIControl_TexturePackList*>(control) != 0)
    {
        height = static_cast<UIControl_TexturePackList*>(control)->GetRealHeight();
    }
    else if (dynamic_cast<UIControl_DynamicLabel*>(control) != 0)
    {
        UIControl_DynamicLabel* label = static_cast<UIControl_DynamicLabel*>(control);
        width = label->GetRealWidth();
        height = label->GetRealHeight();
    }
    else if (dynamic_cast<UIControl_HTMLLabel*>(control) != 0)
    {
        UIControl_HTMLLabel* label = static_cast<UIControl_HTMLLabel*>(control);
        width = label->GetRealWidth();
        height = label->GetRealHeight();
    }
    int left = (int)(control->getXPos() + parentX + m_touchOffsetX);
    int top = (int)(control->getYPos() + parentY + m_touchOffsetY);
    int right = (int)(control->getXPos() + width + parentX + m_touchOffsetX);
    int bottom = (int)(control->getYPos() + height + parentY + m_touchOffsetY);
    elem->left = left;
    elem->top = top;
    elem->right = right;
    elem->bottom = bottom;
    if (left != right && top != bottom)
    {
        EnterCriticalSection(&m_touchMutex);
        m_touchBoxes[(int)groupId][(int)layer][(int)scene].push_back(elem);
        LeaveCriticalSection(&m_touchMutex);
    }
}

void ConsoleUIController::PlayUISFX(const SoundEvent* soundEvent)
{
    const unsigned long long now = System::processTimeInMilliSecs();
    const unsigned int nowLow = static_cast<unsigned int>(now);
    const int nowHigh = static_cast<int>(now >> 32);
    if (nowHigh != m_lastUISFXTimeHigh + (int)(nowLow < m_lastUISFXTimeLow) ||
        nowLow - m_lastUISFXTimeLow > 9)
    {
        m_lastUISFXTimeHigh = nowHigh;
        m_lastUISFXTimeLow = nowLow;
        Minecraft::GetInstance()->mSoundEngine->playUI(soundEvent, 1.0f, 1.0f);
    }
}

bool ConsoleUIController::TouchBoxHit(UIScene* scene, int x, int y)
{
    int groupId = 0;
    int layerId = 0;
    for (int groupIndex = 0; groupIndex < 6; ++groupIndex) {
        UIGroup* group = m_groups[groupIndex];
        if (group == 0)
            continue;
        for (int layerIndex = 0; layerIndex < 8; ++layerIndex) {
            if (group->m_layers[layerIndex] == scene->m_layer) {
                groupId = group->m_group;
                layerId = layerIndex;
                goto foundTouchBoxBucket;
            }
        }
    }
foundTouchBoxBucket:
    const int sceneId = scene->getSceneType();
    if (x == 0xc && y == 0x2c4) {
        x = m_touchX;
        y = m_touchY;
    } else {
        m_touchX = x;
        m_touchY = y;
    }
    EnterCriticalSection(&m_touchMutex);
    std::vector<UIELEMENT*>& touchBoxes = m_touchBoxes[groupId][layerId][sceneId];
    for (std::vector<UIELEMENT*>::iterator it = touchBoxes.begin(); it != touchBoxes.end(); ++it) {
        UIELEMENT* element = *it;
        if (element == 0 || element->control == 0)
            continue;
        if (!element->control->getVisible())
            continue;
        if (element->left > x || x > element->right)
            continue;
        if (element->top > y || y > element->bottom)
            continue;
        if (!m_touchHeld)
            m_lastTouchElement = element;
        m_touchElement = element;
        LeaveCriticalSection(&m_touchMutex);
        return true;
    }
    LeaveCriticalSection(&m_touchMutex);
    m_touchElement = 0;
    return false;
}

void ConsoleUIController::TouchBoxesClear(UIScene* scene)
{
    EnterCriticalSection(&m_touchMutex);
    m_touchHeld = false;
    int groupId = 0;
    int layerId = 0;
    for (int groupIndex = 0; groupIndex < 6; ++groupIndex) {
        UIGroup* group = m_groups[groupIndex];
        if (group == 0)
            continue;
        for (int layerIndex = 0; layerIndex < 8; ++layerIndex) {
            if (group->m_layers[layerIndex] == scene->m_layer) {
                groupId = group->m_group;
                layerId = layerIndex;
                goto foundTouchBoxBucket;
            }
        }
    }
foundTouchBoxBucket:
    {
        const int sceneId = scene->getSceneType();
        std::vector<UIELEMENT*>& touchBoxes = m_touchBoxes[groupId][layerId][sceneId];
        for (std::vector<UIELEMENT*>::iterator it = touchBoxes.begin(); it != touchBoxes.end(); ++it)
            delete *it;
        touchBoxes.clear();
    }
    LeaveCriticalSection(&m_touchMutex);
}

void ConsoleUIController::TouchBoxRebuild(UIScene* scene)
{
    m_rebuildingTouchBoxes = true;
    TouchBoxesClear(scene);
    int groupId = 0;
    int layerId = 0;
    for (int groupIndex = 0; groupIndex < 6; ++groupIndex) {
        UIGroup* group = m_groups[groupIndex];
        if (group == 0)
            continue;
        for (int layerIndex = 0; layerIndex < 8; ++layerIndex) {
            if (group->m_layers[layerIndex] == scene->m_layer) {
                groupId = group->m_group;
                layerId = layerIndex;
                goto foundTouchBoxRebuildGroup;
            }
        }
    }
foundTouchBoxRebuildGroup:
    {
        const int sceneId = scene->getSceneType();
        for (std::vector<UIControl*>::iterator it = scene->m_controls.begin();
             it != scene->m_controls.end(); ++it)
        {
            UIControl* control = *it;
            bool interactive = dynamic_cast<UIControl_CheckBox*>(control) != 0 ||
                                dynamic_cast<UIControl_DynamicLabel*>(control) != 0 ||
                                dynamic_cast<UIControl_HTMLLabel*>(control) != 0 ||
                                dynamic_cast<UIControl_LeaderboardList*>(control) != 0 ||
                                dynamic_cast<UIControl_Slider*>(control) != 0 ||
                                dynamic_cast<UIControl_TexturePackList*>(control) != 0 ||
                                dynamic_cast<UIControl_List*>(control) != 0 ||
                                dynamic_cast<UIControl_GridList*>(control) != 0;
            if (interactive && control->getVisible())
            {
                control->UpdateControl();
                TouchBoxAdd(control, (EUIGroup)groupId, (EUILayer)layerId, (EUIScene)sceneId, 0);
            }
        }
    }
    m_rebuildingTouchBoxes = false;
}

void ConsoleUIController::NavigateToHomeMenu(bool immediate)
{
    CloseAllPlayersScenes(false);
    gApp.SetLiveLinkRequired(false);
    gApp.setIgnoreMainMenuInputDuringInvite(false);

    Minecraft* minecraft = Minecraft::GetInstance();
    minecraft->mTexturePackRepository->selectTexturePackById(0, false);
    if (!immediate)
    {
        minecraft->mSoundEngine->SetStreamingSounds(0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18,
                                                     0x1b, 0x1c, 0x1f, 0x20, 0x29, 0x2a, 0x2a, 0x3e,
                                                     0x44, 0x57);
    }
    if (minecraft->mTexturePackRepository->needsUIUpdate())
    {
        m_homeMenuPending = true;
        return;
    }
    NavigateToScene(gProfile->GetPrimaryPad(), (EUIScene)3, 0, (EUILayer)6, (EUIGroup)7);
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
