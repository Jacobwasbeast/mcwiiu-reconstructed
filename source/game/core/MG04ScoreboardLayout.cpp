#include "game/core/MG04ScoreboardLayout.hpp"

#include <sstream>

#include "game/core/StatsUID.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/network/INetworkPlayer.hpp"

MG04ScoreboardLayout::MG04ScoreboardLayout() : IScoreboardLayout()
{
}

MG04ScoreboardLayout::~MG04ScoreboardLayout()
{
}

bool MG04ScoreboardLayout::SORTBYSCORE(PlayerInfo* first, PlayerInfo* second)
{
    if (first == 0)
    {
        return second != 0;
    }
    if (second == 0)
    {
        return false;
    }
    if (first->voiceStatus != second->voiceStatus)
    {
        return first->voiceStatus > second->voiceStatus;
    }
    if (first->colour != second->colour)
    {
        return first->colour > second->colour;
    }
    if (first->smallId != second->smallId)
    {
        return first->smallId > second->smallId;
    }
    return first->name > second->name;
}

void MG04ScoreboardLayout::Init(int value)
{
    m_playerCount = value;
    ClearPlayersInfo();
}

std::wstring MG04ScoreboardLayout::getLabel2(const StatsUID&)
{
    return L"Blocks Placed: {BLOCKS}";
}

std::wstring MG04ScoreboardLayout::getLabel3(const StatsUID&)
{
    return L"Blocks Destroyed: {BLOCKS}";
}

std::wstring MG04ScoreboardLayout::getLabel4(const StatsUID&)
{
    return L"Distance: {DELTA}";
}

std::wstring MG04ScoreboardLayout::getLabel5(const StatsUID&)
{
    return L"Winner";
}

std::wstring MG04ScoreboardLayout::getLabel6(const StatsUID&)
{
    return L"";
}

std::wstring MG04ScoreboardLayout::getLabel7(const StatsUID&)
{
    return L"Overall Winner";
}

uint32_t MG04ScoreboardLayout::getStatIcon2(const EScoreboardType&)
{
    return 5;
}

uint32_t MG04ScoreboardLayout::getStatIcon3(const EScoreboardType&)
{
    return 5;
}

uint32_t MG04ScoreboardLayout::getStatIcon4(const EScoreboardType&)
{
    return 7;
}

uint32_t MG04ScoreboardLayout::getStatIcon5(const EScoreboardType&)
{
    return 7;
}

uint32_t MG04ScoreboardLayout::getTitleIcon(int column, const EScoreboardType& type)
{
    if (static_cast<unsigned int>(type) < 3)
    {
        return 0x77;
    }
    return 0xffffffffu;
}

std::wstring MG04ScoreboardLayout::GetRowLabel(int column, const StatsUID&, const EScoreboardType& type)
{
    if (type == EScoreboardType_Unknown)
        return std::wstring(L"---");
    if (column >= 0 && column < 3)
    {
        std::wstringstream text;
        text << column;
        return text.str();
    }
    return std::wstring(L"---");
}

boost::shared_ptr<ItemInstance> MG04ScoreboardLayout::getItemForSlot(int row, int, const unsigned int&, const EScoreboardType& type)
{
    if (type == EScoreboardType_Unknown)
    {
        if (row == 3)
        {
            return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 3));
        }
        return boost::shared_ptr<ItemInstance>();
    }

    if (row == 0)
    {
        return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::ironIngot));
    }
    if (row == 1)
    {
        return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::goldIngot));
    }
    if (row == 2)
    {
        return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::diamond));
    }
    if (row == 3)
    {
        return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 3));
    }
    return boost::shared_ptr<ItemInstance>();
}

unsigned char MG04ScoreboardLayout::GetEndBarIcon(PlayerInfo* info)
{
    return static_cast<unsigned char>(info == 0 ? 0x6e : 0);
}

void MG04ScoreboardLayout::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info)
{
    IScoreboardLayout::BuildPlayerInfo(player, info);
}

PlayerInfo* MG04ScoreboardLayout::CreatePlayerInfo()
{
    PlayerInfo* info = new PlayerInfo;
    info->smallId = 0;
    info->voiceStatus = 0;
    info->colour = 0;
    info->name.clear();
    return info;
}

void MG04ScoreboardLayout::AddPlayer(unsigned char smallId)
{
    PlayerInfo* info = CreatePlayerInfo();
    info->smallId = smallId;
    m_players.push_back(info);
}

void MG04ScoreboardLayout::UpdatePlayer(int index, PlayerInfo*, PlayerInfo* newInfo, bool)
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

bool MG04ScoreboardLayout::showLifeSpanProgress()
{
    return true;
}

uint32_t MG04ScoreboardLayout::getNumberOfColumns()
{
    return 4;
}
