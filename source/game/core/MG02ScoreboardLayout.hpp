#ifndef GAME_CORE_MG02SCOREBOARDLAYOUT_HPP
#define GAME_CORE_MG02SCOREBOARDLAYOUT_HPP

#include "game/core/IScoreboardLayout.hpp"

class MG02ScoreboardLayout : public IScoreboardLayout
{
public:
    MG02ScoreboardLayout();
    virtual ~MG02ScoreboardLayout();
    static bool SORTBYSCORE(PlayerInfo* first, PlayerInfo* second);
    void Init(int type);
    virtual uint32_t getNumberOfColumns();
    virtual std::wstring getLabel2(const StatsUID& uid);
    virtual std::wstring getLabel3(const StatsUID& uid);
    virtual std::wstring getLabel4(const StatsUID& uid);
    virtual std::wstring getLabel5(const StatsUID& uid);
    virtual std::wstring getLabel6(const StatsUID& uid);
    virtual std::wstring getLabel7(const StatsUID& uid);
    virtual uint32_t getStatIcon1(const EScoreboardType& type);
    virtual uint32_t getStatIcon2(const EScoreboardType& type);
    virtual uint32_t getStatIcon3(const EScoreboardType& type);
    virtual uint32_t getStatIcon4(const EScoreboardType& type);
    virtual uint32_t getStatIcon5(const EScoreboardType& type);
    virtual uint32_t getTitleIcon(int column, const EScoreboardType& type);
    virtual std::wstring GetRowLabel(int row, const StatsUID& uid, const EScoreboardType& type);
    virtual uint64_t getItemForSlot(int column, int row, int slot, const unsigned int& value, const EScoreboardType& type);
    uint64_t getItemForSlot(int column, int row, const unsigned int& value, const EScoreboardType& type);
    virtual uint32_t GetEndBarIcon(PlayerInfo* info);
    virtual void BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info);
    virtual PlayerInfo* CreatePlayerInfo();
    virtual void AddPlayer(unsigned char id);
    virtual void UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo, bool changed);
    virtual bool showLifeSpanProgress();
};

#endif
