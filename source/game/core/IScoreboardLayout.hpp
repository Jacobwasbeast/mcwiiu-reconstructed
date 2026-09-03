#ifndef GAME_CORE_ISCOREBOARDLAYOUT_HPP
#define GAME_CORE_ISCOREBOARDLAYOUT_HPP

#include <stdint.h>
#include <string>
#include <vector>

class INetworkPlayer;
class StatsUID;

enum EScoreboardType
{
    EScoreboardType_Unknown = 0
};

struct PlayerInfo
{
    unsigned char smallId;
    unsigned char voiceStatus;
    unsigned short colour;
    std::wstring name;
};

class IScoreboardLayout
{
public:
    IScoreboardLayout();
    virtual ~IScoreboardLayout();

    virtual uint32_t getStatIcon1(const EScoreboardType&) = 0;
    virtual uint32_t getStatIcon2(const EScoreboardType&) = 0;
    virtual uint32_t getStatIcon3(const EScoreboardType&) = 0;
    virtual uint32_t getStatIcon4(const EScoreboardType&) = 0;
    virtual uint32_t getStatIcon5(const EScoreboardType&) = 0;
    virtual std::wstring getLabel2(const StatsUID&) = 0;
    virtual std::wstring getLabel3(const StatsUID&) = 0;
    virtual std::wstring getLabel4(const StatsUID&) = 0;
    virtual std::wstring getLabel5(const StatsUID&) = 0;
    virtual std::wstring getLabel6(const StatsUID&) = 0;
    virtual std::wstring getLabel7(const StatsUID&) = 0;
    virtual uint32_t getEndBarIcon(PlayerInfo*) = 0;

    virtual uint32_t getTitleIcon(int, const EScoreboardType&);
    virtual std::wstring GetRowLabel(int, const StatsUID&, const EScoreboardType&);
    virtual uint32_t getNumberOfColumns();
    virtual uint64_t getItemForSlot(int, int, int, const unsigned int&, const EScoreboardType&) = 0;
    virtual std::wstring GetCustomProgressDetails(const StatsUID&, int&);
    virtual int GetPlayerCount();
    virtual PlayerInfo* GetPlayer(int);
    virtual bool RemovePlayer(int);
    virtual void BuildPlayerInfo(INetworkPlayer*, PlayerInfo*);

    uint64_t getItemForSlot(const wchar_t*, const unsigned int&, const EScoreboardType&);

    void ClearPlayersInfo();
    static const wchar_t* getIcon(int);
    bool doesExist(const unsigned int&);
    std::wstring getNameOfPlayerAt(const unsigned int&);
    unsigned char getSmallIDOfPlayerAt(const unsigned int&);
    int getVoiceStatusOfPlayerAt(const unsigned int&);
    int getColorStateOfPlayerAt(const unsigned int&);
    PlayerInfo* getPlayersAt(const unsigned int&);

protected:
    int m_playerCount;
    std::vector<PlayerInfo*> m_players;
};

#endif
