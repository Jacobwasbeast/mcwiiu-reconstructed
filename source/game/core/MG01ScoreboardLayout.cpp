#include "game/core/MG01ScoreboardLayout.hpp"

#include "game/network/INetworkPlayer.hpp"

MG01ScoreboardLayout::MG01ScoreboardLayout(EScoreboardType type)
    : IScoreboardLayout(), m_scoreboardType(type)
{
}

MG01ScoreboardLayout::~MG01ScoreboardLayout()
{
}

unsigned int MG01ScoreboardLayout::SORTBYSCORE(PlayerInfo* first, PlayerInfo* second)
{
    if (first->voiceStatus == 0 && second->voiceStatus != 0)
    {
        return 0;
    }
    if (first->voiceStatus != 0 && second->voiceStatus == 0)
    {
        return 1;
    }
    if (first->smallId != second->smallId)
    {
        return first->smallId > second->smallId;
    }
    if (first->colour != second->colour)
    {
        return first->colour > second->colour;
    }
    return first->name > second->name;
}

void MG01ScoreboardLayout::Init(int value)
{
    m_playerCount = value;
}

unsigned int MG01ScoreboardLayout::getNumberOfColumns()
{
    return 3;
}

boost::shared_ptr<ItemInstance> MG01ScoreboardLayout::getItemForSlot(int, int, const unsigned int&, const EScoreboardType&)
{
    return boost::shared_ptr<ItemInstance>();
}

std::wstring MG01ScoreboardLayout::getLabel2(const StatsUID&)
{
    return std::wstring();
}

std::wstring MG01ScoreboardLayout::getLabel3(const StatsUID&)
{
    return std::wstring();
}

std::wstring MG01ScoreboardLayout::getLabel4(const StatsUID&)
{
    return std::wstring();
}

std::wstring MG01ScoreboardLayout::getLabel5(const StatsUID&)
{
    return std::wstring();
}

std::wstring MG01ScoreboardLayout::getLabel6(const StatsUID&)
{
    return std::wstring();
}

std::wstring MG01ScoreboardLayout::getLabel7(const StatsUID&)
{
    return std::wstring();
}

unsigned int MG01ScoreboardLayout::getStatIcon2(const EScoreboardType&)
{
    return 0xffffffffu;
}

unsigned int MG01ScoreboardLayout::getStatIcon3(const EScoreboardType&)
{
    return 0;
}

unsigned int MG01ScoreboardLayout::getStatIcon4(const EScoreboardType&)
{
    return 1;
}

unsigned int MG01ScoreboardLayout::getStatIcon5(const EScoreboardType&)
{
    return 0xffffffffu;
}

unsigned int MG01ScoreboardLayout::getTitleIcon(int column, const EScoreboardType& type)
{
    const unsigned int columns = getNumberOfColumns();
    if (column == static_cast<int>(columns) - 3)
    {
        return 4;
    }
    if (column == static_cast<int>(columns) - 1 && type == EScoreboardType_Unknown)
    {
        return 3;
    }
    return 0xffffffffu;
}

std::wstring MG01ScoreboardLayout::GetRowLabel(int, const StatsUID&, const EScoreboardType&)
{
    return std::wstring();
}

void MG01ScoreboardLayout::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info)
{
    IScoreboardLayout::BuildPlayerInfo(player, info);
}

PlayerInfo* MG01ScoreboardLayout::CreatePlayerInfo()
{
    PlayerInfo* info = new PlayerInfo;
    info->smallId = 0;
    info->voiceStatus = 0;
    info->colour = 0;
    info->name.clear();
    return info;
}

void MG01ScoreboardLayout::AddPlayer(unsigned char smallId)
{
    PlayerInfo* info = CreatePlayerInfo();
    info->smallId = smallId;
    m_players.push_back(info);
}

void MG01ScoreboardLayout::UpdatePlayer(int index, PlayerInfo*, PlayerInfo* newInfo, bool)
{
    if (index < 0 || index >= static_cast<int>(m_players.size()) || newInfo == 0)
    {
        return;
    }
    PlayerInfo* info = m_players[index];
    info->smallId = newInfo->smallId;
    info->voiceStatus = newInfo->voiceStatus;
    info->colour = newInfo->colour;
    info->name = newInfo->name;
}

unsigned char MG01ScoreboardLayout::GetEndBarIcon(PlayerInfo* info)
{
    if (info != 0 && info->voiceStatus != 0)
    {
        return 2;
    }
    return info == 0 ? 1 : static_cast<unsigned char>(info->smallId ^ 1);
}

bool MG01ScoreboardLayout::showLifeSpanProgress()
{
    return 1;
}
