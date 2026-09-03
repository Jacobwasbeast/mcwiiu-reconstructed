#include "game/core/IScoreboardLayout.hpp"
#include "game/network/INetworkPlayer.hpp"
#include <cwchar>

IScoreboardLayout::IScoreboardLayout() : m_playerCount(0), m_players()
{
}

IScoreboardLayout::~IScoreboardLayout()
{
    ClearPlayersInfo();
}

void IScoreboardLayout::ClearPlayersInfo()
{
    std::vector<PlayerInfo*>::iterator it;
    for (it = m_players.begin(); it != m_players.end(); ++it)
    {
        delete *it;
    }
    m_players.clear();
}

uint64_t IScoreboardLayout::getItemForSlot(const wchar_t* value, const unsigned int& slot, const EScoreboardType& type)
{
    int header = -1;
    int stat = -1;
    if (value != 0)
    {
        swscanf(value, L"slot_header_%d", &header);
        swscanf(value, L"slot_stat_%d", &stat);
    }
    return getItemForSlot(header, -1, -1, slot, type);
}

const wchar_t* IScoreboardLayout::getIcon(int index)
{
    static const wchar_t empty[] = L"";
    if (index < 0 || index >= 0x77)
    {
        return empty;
    }
    return empty;
}

bool IScoreboardLayout::doesExist(const unsigned int& index)
{
    return index < m_players.size() && m_players[index] != 0;
}

std::wstring IScoreboardLayout::getNameOfPlayerAt(const unsigned int& index)
{
    if (!doesExist(index))
    {
        return std::wstring();
    }
    return m_players[index]->name;
}

unsigned char IScoreboardLayout::getSmallIDOfPlayerAt(const unsigned int& index)
{
    if (!doesExist(index))
    {
        return 0;
    }
    return m_players[index]->smallId;
}

int IScoreboardLayout::getVoiceStatusOfPlayerAt(const unsigned int& index)
{
    if (!doesExist(index))
    {
        return 0;
    }
    return m_players[index]->voiceStatus;
}

int IScoreboardLayout::getColorStateOfPlayerAt(const unsigned int& index)
{
    if (!doesExist(index))
    {
        return 0;
    }
    return m_players[index]->colour;
}

int IScoreboardLayout::GetPlayerCount()
{
    return static_cast<int>(m_players.size());
}

PlayerInfo* IScoreboardLayout::GetPlayer(int index)
{
    return m_players[index];
}

bool IScoreboardLayout::RemovePlayer(int index)
{
    if (index < 0 || index >= static_cast<int>(m_players.size()))
    {
        return false;
    }
    delete m_players[index];
    m_players.erase(m_players.begin() + index);
    return true;
}

void IScoreboardLayout::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info)
{
    if (player == 0 || info == 0)
    {
        return;
    }
    info->smallId = player->GetSmallId();
    info->name = player->GetDisplayName();
    info->voiceStatus = 0;
    if (player->IsTalking())
    {
        info->voiceStatus = 1;
    }
    info->colour = 0;
}

uint32_t IScoreboardLayout::getTitleIcon(int, const EScoreboardType&)
{
    return 0xffffffffu;
}

std::wstring IScoreboardLayout::GetRowLabel(int, const StatsUID&, const EScoreboardType&)
{
    return std::wstring();
}

uint32_t IScoreboardLayout::getNumberOfColumns()
{
    return 2;
}

std::wstring IScoreboardLayout::GetCustomProgressDetails(const StatsUID&, int&)
{
    return std::wstring();
}

PlayerInfo* IScoreboardLayout::getPlayersAt(const unsigned int& index)
{
    return GetPlayer(static_cast<int>(index));
}
