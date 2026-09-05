#ifndef GAME_CORE_IUISCENE_LOADCREATEJOIN_HPP
#define GAME_CORE_IUISCENE_LOADCREATEJOIN_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include "game/core/FileHeader.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "platform/wiiu/C4JStorage.hpp"

class File;
class LevelGenerationOptions;
class FriendSessionInfo;
struct SAVE_DETAILS;

class IUIScene_LoadCreateJoin
{
public:
    IUIScene_LoadCreateJoin();
    ~IUIScene_LoadCreateJoin();

    uint32_t GetCustomGameModeID();
    void UpdateGamesList();
    static void UpdateGamesListCallback(void* context);
    void _init(bool customGame);
    uint64_t AddMatchmakingButton(std::wstring label, int id, std::wstring description);
    uint64_t AddGameModeButton(std::wstring label, int id, std::wstring description);
    void AddDefaultButtons();
    static uint32_t HandleTelemetrySavesReturned(void* context, SAVE_DETAILS* details, bool complete);
    uint64_t GetSaveInfo();
    void _tick();
    void _updateTooltips();
    void handleTimer(int timer);
    static uint32_t DeleteSaveDialogReturned(void* context, int userIndex,
                                             C4JStorage::EMessageResult result);
    static void SaveOptionsDialogReturned(void* context, int userIndex,
                                          C4JStorage::EMessageResult result);
    uint32_t handleKeyDown(int key, int action, bool repeat);
    void LoadLevelGen(LevelGenerationOptions* options);
    void LoadSaveFromDisk(File* file, ESavePlatform platform);
    void LoadGameMode(EMiniGameId gameMode);
    void HandleLoadListPress(int index);
    void HandleNewListPress(int index, int variant);
    static void TexturePackDialogReturned(void* context, int userIndex,
                                          C4JStorage::EMessageResult result);
    void CheckAndJoinGame(int index);
    void HandleGamesListPressed(int index);
    void SetIgnoreInput(bool ignore);
    uint32_t getActiveTab();
    static void RenableInput(void* context, int userIndex, int action);

private:
    uint32_t m_primaryVtable;
    uint32_t m_secondaryVtable;
    std::vector<FriendSessionInfo*> m_friendSessions;
    int m_userIndex;
    unsigned char m_ignoreInput;
    unsigned char m_state19;
    unsigned char m_state1a;
    unsigned char m_state1b;
    unsigned char m_state1c;
    unsigned char m_state1d;
    unsigned char m_state1e;
    unsigned char m_state1f;
    unsigned char m_customGame;
    unsigned char m_state21;
    unsigned char m_state22;
    unsigned char m_state23;
    int m_selectedGame;
    int m_selectedSave;
    int m_activeList;
    int m_timer;
    int m_gameMode;
    int m_selectedTexturePack;
    int m_currentButton;
    int m_tooltipState;
    int m_pendingAction;
    int m_status;
    int m_state58;
    int m_state5c;
    std::vector<LevelGenerationOptions*> m_levelGenerationOptions;
    std::vector<int> m_buttonIds;
    uint32_t m_state6c;
    unsigned char m_ready;
    unsigned char m_state71;
    unsigned char m_state72;
    unsigned char m_state73;
    int m_state74;
    unsigned char m_state78;
    unsigned char m_state79;
    unsigned char m_state7a;
    unsigned char m_state7b;
    unsigned char m_state7c;
    unsigned char m_state7d;
    unsigned char m_state7e;
    unsigned char m_state7f;
    unsigned char m_state80;
    unsigned char m_state81;
    unsigned char m_state82;
    unsigned char m_state83;
    unsigned char m_state84;
    unsigned char m_state85;
    unsigned char m_state86;
    unsigned char m_state87;
    unsigned char m_state88;
    unsigned char m_state89;
    unsigned char m_state8a;
    unsigned char m_state8b;
    uint32_t m_saveData;
    unsigned char m_state90;
    unsigned char m_state91;
    unsigned char m_state92;
    unsigned char m_state93;
    unsigned char m_state94;
    unsigned char m_state95;
    unsigned char m_state96;
    unsigned char m_state97;
    unsigned char m_state98;
    unsigned char m_state99;
    unsigned char m_state9a;
    unsigned char m_state9b;
    uint32_t m_texturePack;
    unsigned char m_statea0;
    unsigned char m_statea1;
    unsigned char m_statea2;
    unsigned char m_statea3;
    uint32_t m_scene;
    unsigned char m_statea8;
    unsigned char m_statea9;
    unsigned char m_stateaa;
    unsigned char m_stateab;
};

#endif
