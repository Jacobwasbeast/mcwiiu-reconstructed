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
struct FriendSessionInfo;
struct SAVE_DETAILS;
class IUIScene_LoadCreateJoin
{
public:
    IUIScene_LoadCreateJoin();
    ~IUIScene_LoadCreateJoin();
    uint32_t GetCustomGameModeID();
    void UpdateGamesList();
    static void UpdateGamesListCallback(void*);
    void _init(bool);
    uint64_t AddMatchmakingButton(std::wstring, int, std::wstring);
    uint64_t AddGameModeButton(std::wstring, int, std::wstring);
    void AddDefaultButtons();
    static uint32_t HandleTelemetrySavesReturned(void*, SAVE_DETAILS*, bool);
    uint64_t GetSaveInfo();
    void _tick();
    void _updateTooltips();
    void handleTimer(int);
    static uint32_t DeleteSaveDialogReturned(void*, int, C4JStorage::EMessageResult);
    static void SaveOptionsDialogReturned(void*, int, C4JStorage::EMessageResult);
    uint32_t handleKeyDown(int, int, bool);
    void LoadLevelGen(LevelGenerationOptions*);
    void LoadSaveFromDisk(File*, ESavePlatform);
    void LoadGameMode(EMiniGameId);
    void HandleLoadListPress(int);
    void HandleNewListPress(int, int);
    static int TexturePackDialogReturned(void*, int, C4JStorage::EMessageResult);
    void CheckAndJoinGame(int);
    void HandleGamesListPressed(int);
    void SetIgnoreInput(bool);
    uint32_t getActiveTab();
    static void RenableInput(void*, int, int);
    virtual void AddCreateListButton(const std::wstring& label, int id) = 0;
    virtual void SetCreateListIcon(int index, unsigned char* data, unsigned int size,
                                   const std::wstring& name) = 0;
private:
    uint32_t m_state0;
    uint32_t m_state4;
    std::vector<FriendSessionInfo*> m_friendSessions;
    int m_userIndex;
    int m_state10;
    int m_state14;
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
    int m_state24;
    int m_state28;
    int m_state2c;
    int m_state30;
    int m_state34;
    int m_state38;
    int m_state3c;
    int m_state40;
    int m_state44;
    int m_state48;
    std::vector<LevelGenerationOptions*> m_levelGenerationOptions;
    std::vector<int> m_buttonIds;
    unsigned char m_state70;
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
    uint32_t m_state8c;
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
    uint32_t m_state9c;
    unsigned char m_statea0;
    unsigned char m_statea1;
    unsigned char m_statea2;
    unsigned char m_statea3;
    uint32_t m_statea4;
    unsigned char m_statea8;
    unsigned char m_statea9;
    unsigned char m_stateaa;
    unsigned char m_stateab;
    unsigned char m_stateac;
};
#endif
