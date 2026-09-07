#ifndef GAME_CORE_MG04SCOREBOARDLAYOUT_HPP
#define GAME_CORE_MG04SCOREBOARDLAYOUT_HPP

#include "game/core/IScoreboardLayout.hpp"

class MG04ScoreboardLayout : public IScoreboardLayout
{
public:
    MG04ScoreboardLayout();
    virtual ~MG04ScoreboardLayout();
    static bool SORTBYSCORE(PlayerInfo* first, PlayerInfo* second);
    void Init(int value);
    std::wstring getLabel2(const StatsUID& uid);
    std::wstring getLabel3(const StatsUID& uid);
    std::wstring getLabel4(const StatsUID& uid);
    std::wstring getLabel5(const StatsUID& uid);
    std::wstring getLabel6(const StatsUID& uid);
    std::wstring getLabel7(const StatsUID& uid);
    uint32_t getStatIcon2(const EScoreboardType& type);
    uint32_t getStatIcon3(const EScoreboardType& type);
    uint32_t getStatIcon4(const EScoreboardType& type);
    uint32_t getStatIcon5(const EScoreboardType& type);
    uint32_t getTitleIcon(int column, const EScoreboardType& type);
    std::wstring GetRowLabel(int column, const StatsUID& uid, const EScoreboardType& type);
    virtual boost::shared_ptr<ItemInstance> getItemForSlot(int row, int column, const unsigned int& slot, const EScoreboardType& type);
    virtual unsigned char GetEndBarIcon(PlayerInfo* info);
    void BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info);
    PlayerInfo* CreatePlayerInfo();
    void AddPlayer(unsigned char smallId);
    void UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo, bool changed);
    virtual bool showLifeSpanProgress();
    uint32_t getNumberOfColumns();
};

#endif
