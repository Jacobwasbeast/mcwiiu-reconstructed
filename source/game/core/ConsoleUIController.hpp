#ifndef GAME_CORE_CONSOLEUICONTROLLER_HPP
#define GAME_CORE_CONSOLEUICONTROLLER_HPP

#include <string>
#include <vector>
#include <boost/unordered_map.hpp>

#include "game/core/IUIController.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class fuiBitmapFont;
class C4JThread;
class UITTFFont;
class UIControl;
class UIScene;
class UIScene_HUD;
class UIGroup;
class Tutorial;
struct _TutorialPopupInfo;
struct _CustomDrawData;

class ConsoleUIController : public IUIController
{
public:
    enum EFont
    {
        EFont_Bitmap = 1,
        EFont_Japanese = 2,
        EFont_ChineseSimplified = 3,
        EFont_ChineseTraditional = 4,
        EFont_Korean = 5
    };

    struct UIELEMENT
    {
        UIControl* control;
        int left;
        int top;
        int right;
        int bottom;
    };

    struct _TextureDetail
    {
        int referenceCount;
        unsigned char* data;
        unsigned int size;
        int texture;
        bool textureLoaded;

        _TextureDetail() : referenceCount(0), data(0), size(0), texture(0), textureLoaded(false) {}
    };

    struct _CachedMovieData
    {
        arrayWithLength<unsigned char> data;
        unsigned long long expiryTime;

        _CachedMovieData() : expiryTime(0) {}
        _CachedMovieData(const arrayWithLength<unsigned char>& value, unsigned long long expiry)
            : data(value), expiryTime(expiry) {}
    };

    struct _QueuedMessageBoxData
    {
        int titleId;
        int messageId;
        int* messageParameters;
        unsigned int parameterCount;
        unsigned int userIndex;
        int (*callback)(void*, int, C4JStorage::EMessageResult);
        void* callbackData;
        wchar_t* overrideText;
        unsigned int buttonFlags;
        bool alert;
        bool blocking;
        int type;

        _QueuedMessageBoxData()
            : titleId(0), messageId(0), messageParameters(0), parameterCount(0), userIndex(0),
              callback(0), callbackData(0), overrideText(0), buttonFlags(0), alert(false),
              blocking(false), type(0) {}
        ~_QueuedMessageBoxData() { delete[] messageParameters; }
    };

    ConsoleUIController();
    virtual ~ConsoleUIController();

    int GetGroup(int playerId);
    void preInit(int width, int height);
    void postInit();
    void init(int width, int height);
    static int TextureSubstitutionCreateCallbackFui(void* context, const std::wstring& name,
                                                     unsigned int* width, unsigned int* height,
                                                     bool* hasAlpha);
    static void TextureSubstitutionDestroyCallbackFui(int texture);
    EFont getFontForLanguage(int language);
    UITTFFont* createFont(EFont font);
    void updateCurrentFont();
    void SetupFont();
    int loadSkin(const std::wstring& name, const std::wstring& path, int texture);
    void loadSkins();
    void renderScenes();
    bool PendingFontChange();
    bool UsingBitmapFont();
    void TouchBoxAdd(UIControl* control, EUIGroup group, EUILayer layer, EUIScene scene,
                     UIControl* parent);
    void TouchBoxAdd(UIControl* control, UIScene* scene);
    bool TouchBoxHit(UIScene* scene, int x, int y);
    void TouchBoxesClear(UIScene* scene);
    void TouchBoxRebuild(UIScene* scene);
    void SendTouchInput(unsigned int playerId, unsigned int action, bool repeat, bool released,
                        bool pressed);
    void HandleTouchInput(unsigned int playerId, unsigned int action, bool repeat, bool released,
                          bool pressed);
    void updateViewportTouchOffset(C4JRender::eViewportType viewport);
    void updateViewportTouchOffset(int x, int y);
    void SetMenuDisplayed(int playerId, bool displayed);
    void setFullscreenMenuDisplayed(bool displayed);
    bool handleKeyPress(unsigned int key, unsigned int character);
    bool handleInput();
    void tickInput();
    static unsigned int reloadSkinThreadProc(void* context);
    void ReloadSkin();
    _CachedMovieData& getMovieData(const std::wstring& path);
    void getRenderDimensions(C4JRender::eViewportType viewport, int& width, int& height);
    void setupRenderPosition(C4JRender::eViewportType viewport);
    void setupRenderPosition(int x, int y);
    void setupCustomDrawGameState();
    void endCustomDrawGameState();
    void registerSubstitutionTexture(const std::wstring& name, int texture);
    void registerSubstitutionTexture(const std::wstring& name, unsigned char* data,
                                     unsigned int size, bool deleteData);
    void unregisterSubstitutionTexture(const std::wstring& name, bool deleteData);
    void FixupMenuDisplayedAfterNavigateBack();
    void NavigateToHomeMenu(bool immediate);
    UIScene* GetTopScene(int playerId, EUILayer layer, EUIGroup group);
    unsigned int RegisterForCallbackId(UIScene* scene);
    void UnregisterCallbackId(unsigned int callbackId);
    UIScene* GetSceneFromCallbackId(unsigned int callbackId);
    void EnterCallbackIdCriticalSection();
    void LeaveCallbackIdCriticalSection();
    void AnimateKeyPress(int playerId, int action, bool repeat, bool released, bool pressed);
    void OverrideSFX(int playerId, int action, bool enabled);
    void ResetPressStartToPlayUIComponents();
    void ClearPressStart();
    void SetAchievementUnlocked(int playerId, int achievementId, int imageId, int value);
    bool GetAchievementToastShowing(int playerId);
    bool GetHoldToShowAchievementsActive(bool includeCancelled);
    bool GetHoldActionCancelled(int playerId);
    void RequestUGCMessageBox(int titleId, int messageId, int buttonId,
                              int (*callback)(void*, int, C4JStorage::EMessageResult),
                              void* callbackData);
    unsigned int GetWinUserIndex();
    UIScene* FindScene(EUIScene scene);

    virtual void tick();
    virtual bool requestResolutionChange(int resolution);
    virtual void setResolutionChangeDisableFlag(eRESOLUTION_DISABLE_FLAG flag);
    virtual void clearResolutionChangeDisableFlag(eRESOLUTION_DISABLE_FLAG flag);
    virtual void render();
    virtual void StartReloadSkinThread();
    virtual bool IsReloadingSkin();
    virtual void CleanUpSkinReload();
    virtual bool NavigateToScene(int playerId, EUIScene scene, void* sceneData, EUILayer layer,
                                 EUIGroup group);
    virtual bool NavigateBack(int playerId, bool closeCurrent, EUIScene scene, EUILayer layer);
    virtual void CloseUIScenes(int playerId, bool immediate);
    virtual void CloseAllPlayersScenes(bool immediate);
    virtual bool IsPauseMenuDisplayed(int playerId);
    virtual bool IsContainerMenuDisplayed(int playerId);
    virtual bool IsIgnorePlayerJoinMenuDisplayed(int playerId);
    virtual bool IsIgnoreAutosaveMenuDisplayed(int playerId);
    virtual void SetIgnoreAutosaveMenuDisplayed(int playerId, bool ignore);
    virtual bool IsSceneInStack(int playerId, EUIScene scene);
    virtual bool GetMenuDisplayed(int playerId);
    virtual void CheckMenuDisplayed();
    virtual void SetTooltipText(unsigned int playerId, unsigned int tooltipIndex, int textId);
    virtual void SetEnableTooltips(unsigned int playerId, bool enable);
    virtual void ShowTooltip(unsigned int playerId, unsigned int tooltipIndex, bool show);
    virtual void SetTooltips(unsigned int playerId, int tooltip0, int tooltip1, int tooltip2,
                             int tooltip3, int tooltip4, int tooltip5, int tooltip6, int tooltip7,
                             int tooltip8, int tooltip9, int tooltip10, int tooltip11,
                             int tooltip12, int tooltip13, int tooltip14, int tooltip15,
                             bool show, bool useGlobalGroup);
    virtual void EnableTooltip(unsigned int playerId, unsigned int tooltipIndex, bool enable);
    virtual void RefreshTooltips(unsigned int playerId);
    virtual void PlayUISFX(const SoundEvent* soundEvent);
    virtual void ShowUIDebugConsole(bool show);
    virtual void ShowUIDebugMarketingGuide(bool show);
    virtual void DisplayGamertag(unsigned int playerId, bool show);
    virtual void SetSelectedItem(unsigned int playerId, const std::wstring& itemName,
                                 const std::wstring& itemDescription);
    virtual void ResetSelectedItem();
    virtual void UpdateSelectedItemPos(unsigned int playerId);
    virtual void HandleDLCMountingComplete();
    virtual void HandleDLCInstalled(int packId);
    virtual void HandleDLCLicenseChange();
    virtual void HandleTMSDLCFileRetrieved(int fileId);
    virtual void HandleTMSBanFileRetrieved(int fileId);
    virtual void HandleInventoryUpdated(int playerId);
    virtual void HandleGameTick();
    virtual void HandleSaveDeviceRemoved(int deviceIndex);
    virtual void SetTutorialDescription(int playerId, _TutorialPopupInfo* popupInfo);
    virtual void SetTutorialVisible(int playerId, bool visible);
    virtual bool IsTutorialVisible(int playerId);
    virtual void UpdatePlayerBasePositions();
    virtual void SetEmptyQuadrantLogo(int quadrant);
    virtual void HideAllGameUIElements();
    virtual void ShowOtherPlayersBaseScene(unsigned int playerId, bool show);
    virtual void ShowTrialTimer(bool show);
    virtual void SetTrialTimerLimitSecs(unsigned int limitSecs);
    virtual void UpdateTrialTimer(unsigned int remainingSecs);
    virtual void ReduceTrialTimerValue();
    virtual void ShowAutosaveCountdownTimer(bool show);
    virtual void UpdateAutosaveCountdownTimer(unsigned int remainingSecs);
    virtual void ShowSavingMessage(unsigned int playerId, C4JStorage::ESavingMessage message);
    virtual bool PressStartPlaying(unsigned int playerId);
    virtual void ShowPressStart(unsigned int playerId);
    virtual void ShowChestRefillAnimation(bool show);
    virtual void SetWinUserIndex(unsigned int userIndex);
    virtual void ShowTimedSplash(unsigned int playerId, const std::wstring& text,
                                 unsigned int durationMs);
    virtual bool IsShowingErrorMessage(int playerId);
    virtual bool IsShowingAlertMessage(int playerId);
    virtual bool getCleanupOnReload();
    virtual void setCleanupOnReload(bool cleanupOnReload);
    virtual bool IsExpectingOrReloadingSkin();
    virtual void SetTooltips(unsigned int playerId, int* tooltips, bool show,
                             bool useGlobalGroup);
    virtual bool IsToolTipDynamic(unsigned int playerId, unsigned int tooltipIndex,
                                  EControllerActions& action);
    virtual int GetHotBarX(unsigned int playerId);
    virtual int GetHotBarY(unsigned int playerId);
    virtual int GetHotBarWidth(unsigned int playerId);
    virtual int GetHotBarHeight(unsigned int playerId);
    virtual bool GetQuickSelectVisible(unsigned int playerId);
    virtual bool GetDRCEnabled();
    virtual void SetTutorial(int playerId, Tutorial* tutorial);
    virtual void RemoveInteractSceneReference(int playerId, UIScene* scene);
    virtual void HideChestRefillAnimation();
    virtual void ShowPlayerDisplayname(bool show);
    virtual void HidePressStart();
    virtual int RequestProfileSignInMenu(int titleId, int messageId, wchar_t** messageParameters,
                                         unsigned int parameterCount, unsigned int userIndex,
                                         int (*callback)(void*, int, int), void* callbackData,
                                         wchar_t* overrideText, unsigned int controllerIndex);
    virtual void RequestAlertMessage(int titleId, int messageId, int* messageParameters,
                                     unsigned int parameterCount, unsigned int userIndex,
                                     int (*callback)(void*, int, C4JStorage::EMessageResult),
                                     void* callbackData, wchar_t* overrideText, bool blocking);
    virtual int RequestErrorMessage(int titleId, int messageId, int* messageParameters,
                                    unsigned int parameterCount, unsigned int userIndex,
                                    int (*callback)(void*, int, C4JStorage::EMessageResult),
                                    void* callbackData, wchar_t* overrideText, bool blocking);
    virtual int RequestMessageBox(int titleId, int messageId, int* messageParameters,
                                  unsigned int parameterCount, unsigned int userIndex,
                                  int (*callback)(void*, int, C4JStorage::EMessageResult),
                                  void* callbackData, wchar_t* overrideText,
                                  unsigned int buttonFlags, bool alert, bool blocking);

    void shutdown();
    void setBlockOrigin(int x, int y);
    static void handleUnlockFullVersionCallback();
    UIScene_HUD* GetHUD(unsigned int playerId);
    void showComponent(int playerId, EUIScene scene, EUILayer layer, EUIGroup group, bool show);
    void removeComponent(EUIScene scene, EUILayer layer, EUIGroup group);
    void SetSysUIShowing(bool showing);
    static void SetSystemUIShowing(void* context, bool showing);
    void setupCustomDrawMatrices(UIScene* scene, _CustomDrawData* data);
    void setupCustomDrawGameStateAndMatrices(UIScene* scene, _CustomDrawData* data);
    void endCustomDrawMatrices();
    void endCustomDrawGameStateAndMatrices();
    void logDebugString(const std::string& text);

private:
    UIGroup* groupForPlayer(int playerId, bool forceGlobal);
    UIGroup* group(EUIGroup groupId);
    UIScene* playerComponent(unsigned int playerId, EUIScene scene);

    UIScene* m_debugConsole;
    int m_unknown08;
    OSFastMutex m_renderMutex;
    OSFastMutex m_touchMutex;
    OSFastMutex m_skinMutex;
    unsigned char m_unknown90[0x360];
    float m_screenWidth;
    float m_screenHeight;
    bool m_initialized;
    unsigned char m_unknown3F9[3];
    int m_renderOffsetX;
    int m_renderOffsetY;
    OSFastMutex m_fontMutex;
    int m_currentFont;
    EFont m_pendingFont;
    bool m_cleanupOnReload;
    bool m_autoCleanupOnReload;
    bool m_fontChangePending;
    unsigned char m_unknown43B;
    UITTFFont* m_font;
    UITTFFont* m_oldFont;
    fuiBitmapFont* m_bitmapFont;
    fuiBitmapFont* m_bitmapFontAlt;
    std::vector<UIELEMENT*> m_touchBoxes[6][8][0x53];
    bool m_touchHeld;
    unsigned char m_unknownFD55[3];
    int m_touchX;
    int m_touchY;
    bool m_rebuildingTouchBoxes;
    unsigned char m_unknownFD61[3];
    int m_resolution;
    int m_requestedResolution;
    unsigned int m_resolutionDisableFlags;
    unsigned int m_lastInputTime;
    unsigned int m_trialTimerLimit;
    unsigned int m_trialTimerValue;
    unsigned int m_trialTimerTick;
    unsigned int m_autosaveTimerTick;
    bool m_menuDisplayed[4];
    bool m_menuDisplayPending[4];
    int m_menuDisplayDelay[4];
    bool m_playerFlags[6];
    unsigned char m_unknownFDAE[2];
    int m_selectedPlayer;
    int m_selectedX;
    int m_selectedY;
    bool m_selectedItemVisible;
    unsigned char m_unknownFDBD[3];
    std::wstring m_selectedItemName;
    std::wstring m_selectedItemDescription;
    boost::unordered_map<std::wstring, _TextureDetail> m_substitutionTextures;
    boost::unordered_map<std::wstring, _CachedMovieData> m_movieCache;
    std::vector<_QueuedMessageBoxData*> m_queuedMessageBoxes;
    unsigned int m_winUserIndex;
    bool m_skinThreadExpected;
    unsigned char m_unknownFE05[3];
    C4JThread* m_skinThread;
    OSFastMutex m_skinThreadMutex;
    boost::unordered_map<unsigned int, UIScene*> m_callbackScenes;
    OSFastMutex m_callbackMutex;
    int m_nextCallbackId;
    int m_touchOffsetX;
    int m_touchOffsetY;
    UIELEMENT* m_touchElement;
    UIELEMENT* m_lastTouchElement;
    UIGroup* m_groups[6];
    int m_pressStartPlaying;
    int m_pressStartBlocked;
    bool m_fullscreenMenuDisplayed;
    int m_customDrawLeft;
    int m_customDrawTop;
    int m_customDrawRight;
    int m_customDrawBottom;
};

extern OSFastMutex g_uiCriticalSection;
extern ConsoleUIController gConsoleUIController;

#endif
