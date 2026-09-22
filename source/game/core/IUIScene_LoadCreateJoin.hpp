#ifndef GAME_CORE_IUISCENE_LOADCREATEJOIN_HPP
#define GAME_CORE_IUISCENE_LOADCREATEJOIN_HPP
#include <stdint.h>
#include <string>
#include <vector>
#include <coreinit/fastmutex.h>
#include "game/core/FileHeader.hpp"
#include "game/core/IUIPartial_OnlineRestricted.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "platform/wiiu/C4JStorage.hpp"
class File;
class _SaveListDetails;
class LevelGenerationOptions;
struct FriendSessionInfo;
struct SAVE_DETAILS;
struct JoinSceneData
{
    FriendSessionInfo* session;
    int userIndex;
    unsigned char customGame;
};
class IUIScene_LoadCreateJoin : public virtual IUIPartial_OnlineRestricted
{
public:
    IUIScene_LoadCreateJoin();
    ~IUIScene_LoadCreateJoin();
    virtual uint32_t GetCustomGameModeID();
    void UpdateGamesList();
    static void UpdateGamesListCallback(void*);
    void _init(bool);
    uint64_t AddMatchmakingButton(std::wstring, int, std::wstring);
    uint64_t AddGameModeButton(std::wstring, int, std::wstring);
    void AddDefaultButtons();
    static int HandleTelemetrySavesReturned(void*, SAVE_DETAILS*, bool);
    void GetSaveInfo();
    void _tick();
    void _updateTooltips();
    void handleTimer(int);
    static uint32_t DeleteSaveDialogReturned(void*, int, C4JStorage::EMessageResult);
    static int SaveOptionsDialogReturned(void*, int, C4JStorage::EMessageResult);
    uint32_t handleKeyDown(int, int, bool);
    void LoadLevelGen(LevelGenerationOptions*);
    void LoadSaveFromDisk(File*, ESavePlatform);
    void LoadGameMode(EMiniGameId);
    void HandleLoadListPress(int);
    std::vector<File*>& GetLoadFileList();
    char* GetSaveDataBase();
    int& GetSelectedCustomGameModeId();
    void HandleNewListPress(int, int);
    static int TexturePackDialogReturned(void*, int, C4JStorage::EMessageResult);
    void CheckAndJoinGame(int);
    void HandleGamesListPressed(int);
    virtual void SetIgnoreInput(bool);
    virtual void Initialise() = 0;
    virtual int GetPad() = 0;
    virtual void SelectTab(int tab) = 0;
    virtual bool GetHideLoadTab() = 0;
    virtual bool DoesLoadListHaveFocus() = 0;
    virtual void ClearLoadList() = 0;
    virtual void AddLoadListButton(const std::wstring& label, int id) = 0;
    virtual void SetLoadListIcon(int index, unsigned char* data, unsigned int size,
                                   const std::wstring& name) = 0;
    virtual void ScrollLoadListToSelection(unsigned int selection) = 0;
    virtual unsigned int GetLoadListSize() = 0;
    virtual bool DoesMashUpWorldHaveFocus() = 0;
    virtual void ClearCreateList() = 0;
    virtual void ShowSaveListTimer(bool show) = 0;
    virtual void ShowJoinListTimer(bool show) = 0;
    virtual void AddCreateListButton(const std::wstring& label, int id) = 0;
    virtual void SetCreateListIcon(int index, unsigned char* data, unsigned int size,
                                   const std::wstring& name) = 0;
    virtual bool DoesGamesListHaveFocus() = 0;
    virtual void InitFocusJoinList(int focus) = 0;
    virtual bool ClearJoinList() = 0;
    virtual unsigned int GetJoinListSize() = 0;
    virtual unsigned int GetJoinListSelection() = 0;
    virtual bool IsJoinListEnabled() = 0;
    virtual void SetJoinListEnabled(bool enabled) = 0;
    virtual void SetGamesListSelection(unsigned int selection) = 0;
    virtual bool AddJoinListButton(const std::wstring& label, int id, unsigned char* data,
                                   unsigned int size, const std::wstring& textureName,
                                   unsigned char* data1, unsigned int size1,
                                   const std::wstring& textureName1) = 0;
    virtual void ShowNoGamesLabel(bool show) = 0;
    virtual void updateTooltips() = 0;
protected:
    std::vector<FriendSessionInfo*> m_friendSessions;
    SAVE_DETAILS* m_saveDetails;
    JoinSceneData* m_joinSceneData;
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
    std::vector<LevelGenerationOptions*> m_levelGenerationOptions;
    std::vector<File*>* m_state5c;
    _SaveListDetails* m_saveListDetails;
    int m_saveDetailsCount;
    std::vector<int> m_buttonIds;
    unsigned char m_state70;
    unsigned char m_state71;
    unsigned char m_state72;
    unsigned char m_state73;
    int m_state74;
    int m_selectedCustomGameModeId;
    unsigned char m_state79;
    unsigned char m_state7a;
    unsigned char m_state7b;
    unsigned char m_state7c;
    unsigned char m_state7d;
    unsigned char m_state7e;
    unsigned char m_state7f;
    OSFastMutex m_lock;
};
#endif

