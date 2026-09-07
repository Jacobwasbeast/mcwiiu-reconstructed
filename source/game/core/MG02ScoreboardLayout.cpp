#include "game/core/MG02ScoreboardLayout.hpp"

#include <algorithm>

#include "game/core/StatsUID.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/network/INetworkPlayer.hpp"

MG02ScoreboardLayout::MG02ScoreboardLayout()
    : IScoreboardLayout()
{
}

MG02ScoreboardLayout::~MG02ScoreboardLayout()
{
}

bool MG02ScoreboardLayout::SORTBYSCORE(PlayerInfo* first, PlayerInfo* second)
{
    if (first == 0)
        return second != 0;
    if (second == 0)
        return false;
    if (first->voiceStatus != second->voiceStatus)
        return first->voiceStatus != 0;
    if (first->colour != second->colour)
        return first->colour < second->colour;
    if (first->smallId != second->smallId)
        return first->smallId < second->smallId;
    return first->name < second->name;
}

void MG02ScoreboardLayout::Init(int value)
{
    m_playerCount = value;
    ClearPlayersInfo();
}

uint32_t MG02ScoreboardLayout::getNumberOfColumns()
{
    return m_playerCount == 1 ? 2 : 3;
}

std::wstring MG02ScoreboardLayout::getLabel2(const StatsUID&) { return std::wstring(L"Blocks Destroyed: {BLOCKS}"); }
std::wstring MG02ScoreboardLayout::getLabel3(const StatsUID&) { return std::wstring(L"Jumps: {JUMPS}"); }
std::wstring MG02ScoreboardLayout::getLabel4(const StatsUID&) { return std::wstring(L"Distance: {DELTA}"); }
std::wstring MG02ScoreboardLayout::getLabel5(const StatsUID&) { return std::wstring(L"Winner"); }
std::wstring MG02ScoreboardLayout::getLabel6(const StatsUID&) { return std::wstring(L"px = 0"); }
std::wstring MG02ScoreboardLayout::getLabel7(const StatsUID&) { return std::wstring(L"Overall Winner"); }

uint32_t MG02ScoreboardLayout::getStatIcon1(const EScoreboardType& type) { return getStatIcon2(type); }
uint32_t MG02ScoreboardLayout::getStatIcon2(const EScoreboardType&) { return 5; }
uint32_t MG02ScoreboardLayout::getStatIcon3(const EScoreboardType&) { return 6; }
uint32_t MG02ScoreboardLayout::getStatIcon4(const EScoreboardType&) { return 7; }
uint32_t MG02ScoreboardLayout::getStatIcon5(const EScoreboardType&) { return 0xffffffff; }

uint32_t MG02ScoreboardLayout::getTitleIcon(int column, const EScoreboardType& type)
{
    if (column == 0)
        return 4;
    if (column == (int)getNumberOfColumns() - 1 && type != EScoreboardType_Unknown)
        return 3;
    return 0xffffffff;
}

std::wstring MG02ScoreboardLayout::GetRowLabel(int row, const StatsUID&, const EScoreboardType& type)
{
    if (type == EScoreboardType_Unknown && row == 0)
        return std::wstring(L"0");
    if (type == EScoreboardType_Unknown && row == 1 && getNumberOfColumns() > 2)
        return std::wstring(L"0");
    return std::wstring(L"---");
}

boost::shared_ptr<ItemInstance> MG02ScoreboardLayout::getItemForSlot(int column, int row, int, const unsigned int& value, const EScoreboardType& type)
{
    return getItemForSlot(column, row, value, type);
}

boost::shared_ptr<ItemInstance> MG02ScoreboardLayout::getItemForSlot(int column, int, const unsigned int&, const EScoreboardType& type)
{
    if (column != -1)
    {
        if (type == EScoreboardType_Unknown)
        {
            int auxValue = 3;
            if (column == 1)
            {
                if (getNumberOfColumns() > 2)
                    auxValue = 0;
            }
            else if (column != 2)
            {
                return ItemInstance::EMPTY;
            }
            return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, auxValue));
        }
        if (getNumberOfColumns() > 2 && column == 1)
            return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 0));
    }
    return ItemInstance::EMPTY;
}

unsigned char MG02ScoreboardLayout::GetEndBarIcon(PlayerInfo* info)
{
    return info == 0 ? 0x6e : (uint32_t)(info->voiceStatus ^ 1);
}

void MG02ScoreboardLayout::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info)
{
    IScoreboardLayout::BuildPlayerInfo(player, info);
}

PlayerInfo* MG02ScoreboardLayout::CreatePlayerInfo()
{
    return new PlayerInfo();
}

void MG02ScoreboardLayout::AddPlayer(unsigned char id)
{
    PlayerInfo* info = CreatePlayerInfo();
    info->smallId = id;
    m_players.push_back(info);
}

void MG02ScoreboardLayout::UpdatePlayer(int index, PlayerInfo*, PlayerInfo* info, bool)
{
    if (index >= 0 && index < (int)m_players.size() && info != 0)
        *m_players[index] = *info;
}

bool MG02ScoreboardLayout::showLifeSpanProgress()
{
    return true;
}
