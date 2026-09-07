#ifndef GAME_CORE_MG03SCOREBOARDLAYOUT_HPP
#define GAME_CORE_MG03SCOREBOARDLAYOUT_HPP

#include "game/core/IScoreboardLayout.hpp"

class MG03ScoreboardLayout : public IScoreboardLayout
{
public:
    explicit MG03ScoreboardLayout(EScoreboardType type);
    virtual ~MG03ScoreboardLayout();
    static unsigned int SORTBYSCORE(PlayerInfo* first, PlayerInfo* second);
    void Init(int value);
    virtual uint32_t getStatIcon1(const EScoreboardType& type);
    virtual uint32_t getStatIcon2(const EScoreboardType& type);
    virtual uint32_t getStatIcon3(const EScoreboardType& type);
    virtual uint32_t getStatIcon4(const EScoreboardType& type);
    virtual uint32_t getStatIcon5(const EScoreboardType& type);
    virtual std::wstring getLabel2(const StatsUID& uid);
    virtual std::wstring getLabel3(const StatsUID& uid);
    virtual std::wstring getLabel4(const StatsUID& uid);
    virtual std::wstring getLabel5(const StatsUID& uid);
    virtual std::wstring getLabel6(const StatsUID& uid);
    virtual std::wstring getLabel7(const StatsUID& uid);
    virtual uint32_t getTitleIcon(int column, const EScoreboardType& type);
    virtual std::wstring GetRowLabel(int row, const StatsUID& uid, const EScoreboardType& type);
    virtual uint32_t getNumberOfColumns();
    virtual boost::shared_ptr<ItemInstance> getItemForSlot(int row, int column, int slot, const unsigned int& value, const EScoreboardType& type);
    boost::shared_ptr<ItemInstance> getItemForSlot(int row, int column, const unsigned int& value, const EScoreboardType& type);
    virtual unsigned char GetEndBarIcon(PlayerInfo* info);
    virtual std::wstring GetCustomProgressDetails(const StatsUID& uid, int& value);
    virtual void BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info);
    virtual PlayerInfo* CreatePlayerInfo();
    virtual void AddPlayer(unsigned char smallId);
    virtual void UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo, bool changed);
    std::wstring getStatForRound(const StatsUID& uid, int round);
    virtual bool showLifeSpanProgress();

private:
    EScoreboardType m_scoreboardType;
};

#endif
