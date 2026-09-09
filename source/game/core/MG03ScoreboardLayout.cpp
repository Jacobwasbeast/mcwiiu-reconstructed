#include "game/core/MG03ScoreboardLayout.hpp"

#include <algorithm>

#include "game/core/StatsUID.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"

MG03ScoreboardLayout::MG03ScoreboardLayout(EScoreboardType)
    : IScoreboardLayout()
{
}

MG03ScoreboardLayout::~MG03ScoreboardLayout()
{
}

unsigned int MG03ScoreboardLayout::SORTBYSCORE(PlayerInfo* first, PlayerInfo* second)
{
    if (first == 0)
        return second != 0;
    if (second == 0)
        return false;
    if (first->voiceStatus != second->voiceStatus)
        return first->voiceStatus == 0;
    if (first->colour != second->colour)
        return first->colour > second->colour;
    if (first->smallId != second->smallId)
        return first->smallId < second->smallId;
    return first->name < second->name;
}

void MG03ScoreboardLayout::Init(int value)
{
    m_playerCount = value;
    ClearPlayersInfo();
    const int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
    for (int index = 0; index < playerCount; ++index)
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerByIndex(index);
        if (player != 0)
        {
            PlayerInfo* info = CreatePlayerInfo();
            BuildPlayerInfo(player, info);
            m_players.push_back(info);
        }
    }
    std::sort(m_players.begin(), m_players.end(), SORTBYSCORE);
}

uint32_t MG03ScoreboardLayout::getNumberOfColumns()
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    return miniGame->ArePointsActive() ? 5 : 3;
}

std::wstring MG03ScoreboardLayout::getStatForRound(const StatsUID&, int)
{
    return std::wstring(L"---");
}

std::wstring MG03ScoreboardLayout::getLabel2(const StatsUID& uid)
{
    return getStatForRound(uid, 0);
}

std::wstring MG03ScoreboardLayout::getLabel3(const StatsUID& uid)
{
    return getStatForRound(uid, 1);
}

std::wstring MG03ScoreboardLayout::getLabel4(const StatsUID& uid)
{
    return getStatForRound(uid, 2);
}

std::wstring MG03ScoreboardLayout::getLabel5(const StatsUID&)
{
    return std::wstring(L"Winner");
}

std::wstring MG03ScoreboardLayout::getLabel6(const StatsUID&)
{
    return std::wstring(L"px = 0");
}

std::wstring MG03ScoreboardLayout::getLabel7(const StatsUID&)
{
    return std::wstring(L"px = 0--");
}

uint32_t MG03ScoreboardLayout::getStatIcon2(const EScoreboardType&)
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    return miniGame->ArePointsActive() ? 0x70 : 0x74;
}

uint32_t MG03ScoreboardLayout::getStatIcon3(const EScoreboardType&)
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (miniGame->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
        return 0xffffffffu;
    return miniGame->ArePointsActive() ? 0x71 : 0x75;
}

uint32_t MG03ScoreboardLayout::getStatIcon4(const EScoreboardType&)
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (miniGame->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
        return 0xffffffffu;
    return miniGame->ArePointsActive() ? 0x72 : 0x76;
}

uint32_t MG03ScoreboardLayout::getStatIcon5(const EScoreboardType&)
{
    return 0xffffffffu;
}

uint32_t MG03ScoreboardLayout::getTitleIcon(int column, const EScoreboardType& type)
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    const bool pointsActive = miniGame->ArePointsActive();
    if (pointsActive)
    {
        if (column == 0)
            return 0x6f;
        if (column == 2)
            return 0x73;
        if (column == 4)
            return type == EScoreboardType_Unknown ? 0x77 : 3;
        return 0xffffffffu;
    }
    if (column == 0)
        return 0x73;
    if (column == 2)
        return type == EScoreboardType_Unknown ? 3 : 0xffffffffu;
    return 0xffffffffu;
}

std::wstring MG03ScoreboardLayout::GetRowLabel(int column, const StatsUID& uid, const EScoreboardType& type)
{
    if (type != EScoreboardType_Unknown)
        return std::wstring();
    if (column == 0)
        return getStatForRound(uid, 0);
    if (column == 2)
        return getStatForRound(uid, 2);
    return std::wstring(L"---");
}

boost::shared_ptr<ItemInstance> MG03ScoreboardLayout::getItemForSlot(int row, int column, const unsigned int& value, const EScoreboardType& type)
{
    return boost::shared_ptr<ItemInstance>();
}

unsigned char MG03ScoreboardLayout::GetEndBarIcon(PlayerInfo* info)
{
    return info != 0 && info->voiceStatus != 0 ? 2 : 0;
}

void MG03ScoreboardLayout::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info)
{
    IScoreboardLayout::BuildPlayerInfo(player, info);
}

PlayerInfo* MG03ScoreboardLayout::CreatePlayerInfo()
{
    PlayerInfo* info = new PlayerInfo;
    info->smallId = 0;
    info->voiceStatus = 0;
    info->colour = 0;
    info->name.clear();
    return info;
}

void MG03ScoreboardLayout::AddPlayer(unsigned char smallId)
{
    PlayerInfo* info = CreatePlayerInfo();
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(smallId);
    BuildPlayerInfo(player, info);
    m_players.push_back(info);
}

void MG03ScoreboardLayout::UpdatePlayer(int index, PlayerInfo*, PlayerInfo* newInfo, bool)
{
    if (index < 0 || index >= static_cast<int>(m_players.size()) || newInfo == 0)
        return;
    PlayerInfo* info = m_players[index];
    info->smallId = newInfo->smallId;
    info->voiceStatus = newInfo->voiceStatus;
    info->colour = newInfo->colour;
    info->name = newInfo->name;
}

std::wstring MG03ScoreboardLayout::GetCustomProgressDetails(const StatsUID&, int& value)
{
    value = 0;
    return std::wstring(L"---");
}

bool MG03ScoreboardLayout::showLifeSpanProgress()
{
    return false;
}
