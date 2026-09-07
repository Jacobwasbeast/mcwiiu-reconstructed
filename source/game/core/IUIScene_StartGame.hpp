#ifndef GAME_CORE_IUISCENE_STARTGAME_HPP
#define GAME_CORE_IUISCENE_STARTGAME_HPP

#include <stdint.h>
#include <vector>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_TexturePackList.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_HTMLLabel.hpp"
#include "game/ui/UIControl_BitmapIcon.hpp"
#include "game/ui/UIControl_Slider.hpp"
#include "game/ui/UIControl_CheckBox.hpp"
#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"
#include "platform/wiiu/C4JStorage.hpp"

class UILayer;
class IUIPartial_OnlineRestricted;
class LevelGenerationOptions;

class IUIScene_StartGame : public UIScene
{
public:
    struct TEXTUREPACKSORTSTRUCT
    {
        int values[16];
    };

    IUIScene_StartGame(int sceneType, void* data, UILayer* layer, bool customGame);
    virtual ~IUIScene_StartGame();

    uint32_t GetCustomGameModeID();
    virtual uint32_t mapElementsAndNames();
    static bool SortTexturePacks(const TEXTUREPACKSORTSTRUCT& first,
                                 const TEXTUREPACKSORTSTRUCT& second);
    void UpdateTexturePackDescription(int packId);
    void OrganizeTexturePacks();
    void _init();
    uint64_t RequestPrivMessageBox();
    void FindAndJoinGame(unsigned int playerId);
    void HostGame(unsigned int playerId);
    void StartGame(unsigned int playerId);
    static uint32_t StartGame_SignInReturned(void* context, bool signedIn, int userIndex);
    virtual void tick();
    virtual void handleReload();
    virtual void handleSelectionChanged(int selection);
    virtual void handleSliderMove(int id, int value);
    virtual void handleTouchBoxRebuild();
    virtual void handleTimerComplete(int timerId);
    virtual void HandleDLCMountingComplete();
    void UpdateCurrentTexturePack(int packIndex);
    static uint32_t TrialTexturePackWarningReturned(void* context, int userIndex,
                                               C4JStorage::EMessageResult result);
    static uint64_t UnlockTexturePackReturned(void* context, int userIndex,
                                         C4JStorage::EMessageResult result);
    static uint32_t TexturePackDialogReturned(void* context, int userIndex,
                                         C4JStorage::EMessageResult result);
    static uint32_t TexturePackDialogBattleMiniGameReturned(void* context, int userIndex,
                                                       C4JStorage::EMessageResult result);
    void CheckPermissionsAndContinue(bool fromLoad);
    static uint64_t WiiUDRCWarningReturned(void* context, int userIndex,
                                      C4JStorage::EMessageResult result);
    void StartSharedLaunchFlow();
    static uint32_t DeleteSaveDataReturned(void* context, bool deleted);
    static uint32_t DeleteSaveDialogReturned(void* context, int userIndex,
                                        C4JStorage::EMessageResult result);
    static uint32_t LoadDataComplete(void* context);
    static uint32_t LoadSaveDataReturned(void* context, bool loaded, bool complete);
    static uint32_t CheckCreativeReturned(void* context, int userIndex,
                                     C4JStorage::EMessageResult result);
    void LaunchGame();
    int getCurrentTextureSelectorIndex(int packId);
    static uint32_t TrophyDialogReturned(void* context, int userIndex,
                                   C4JStorage::EMessageResult result);

private:
    UIControl_TexturePackList m_texturePackList;
    UIControl m_startButton;
    UIControl_Label m_texturePackName;
    UIControl_HTMLLabel m_texturePackDescription;
    UIControl_BitmapIcon m_texturePackIcon;
    UIControl_BitmapIcon m_texturePackComparison;
    UIControl_Slider m_seedSlider;
    UIControl_Slider m_sizeSlider;
    UIControl_CheckBox m_bonusChest;
    LaunchMoreOptionsMenuInitData m_launchData;
    bool m_bonusChestEnabled;
    bool m_bonusChestEnabledNet;
    bool m_bonusChestAllowed;
    bool m_loadDataFailed;
    bool m_hasSaveData;
    bool m_isTrial;
    bool m_isCreative;
    bool m_isBattle;
    bool m_isLobby;
    bool m_dlcMounting;
    bool m_touchBoxRebuild;
    bool m_signInPending;
    int m_userIndex;
    int m_selectedTexturePack;
    int m_previousTexturePack;
    int m_pendingTexturePack;
    int m_customGameMode;
    int m_texturePackId;
    int m_texturePackState;
    int m_textureSelectorState;
    std::vector<TEXTUREPACKSORTSTRUCT> m_texturePacks;
    LevelGenerationOptions* m_levelGenerationOptions;
    IUIPartial_OnlineRestricted* m_onlineRestricted;
};

#endif
