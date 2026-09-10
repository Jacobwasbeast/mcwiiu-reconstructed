#include "game/core/MG03ScoreboardLayout.hpp"

#include <algorithm>

#include "game/core/ClientScoreboard.hpp"
#include "game/core/StatsUID.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

MG03ScoreboardLayout::MG03ScoreboardLayout(EScoreboardType)
    : IScoreboardLayout()
{
}

MG03ScoreboardLayout::~MG03ScoreboardLayout()
{
}

namespace
{
int playerInfoInt(const PlayerInfo* info, unsigned int offset)
{
    return *reinterpret_cast<const int*>(reinterpret_cast<const char*>(info) + offset);
}

unsigned int playerInfoUint(const PlayerInfo* info, unsigned int offset)
{
    return *reinterpret_cast<const unsigned int*>(reinterpret_cast<const char*>(info) + offset);
}

double playerInfoDouble(const PlayerInfo* info, unsigned int offset)
{
    return *reinterpret_cast<const double*>(reinterpret_cast<const char*>(info) + offset);
}
}

unsigned int MG03ScoreboardLayout::SORTBYSCORE(PlayerInfo* first, PlayerInfo* second)
{
    const char* firstBytes = reinterpret_cast<const char*>(first);
    const char* secondBytes = reinterpret_cast<const char*>(second);

    if (firstBytes[0x38] == 0)
    {
        if (secondBytes[0x38] != 0)
        {
            return 1;
        }
    }
    else if (secondBytes[0x38] == 0)
    {
        return 0;
    }

    int secondScore = playerInfoInt(second, 0x28);
    int firstScore = playerInfoInt(first, 0x28);
    if (secondScore < firstScore)
    {
        return 1;
    }
    if (secondScore <= firstScore)
    {
        if (firstBytes[0x39] == 0)
        {
            if (secondBytes[0x39] != 0)
            {
                return 0;
            }
        }
        else if (secondBytes[0x39] == 0)
        {
            return 1;
        }

        int firstValue = playerInfoInt(first, 0x30);
        int secondValue = playerInfoInt(second, 0x30);
        if (firstValue <= secondValue)
        {
            if (firstValue < secondValue)
            {
                return 1;
            }
            if (playerInfoUint(first, 0x34) < playerInfoUint(second, 0x34))
            {
                return 1;
            }
        }
        if (firstValue < secondValue ||
            (firstValue <= secondValue && playerInfoUint(first, 0x34) <= playerInfoUint(second, 0x34)))
        {
            secondScore = playerInfoInt(second, 0x24);
            firstScore = playerInfoInt(first, 0x24);
            if (secondScore < firstScore)
            {
                return 1;
            }
            if (secondScore <= firstScore &&
                playerInfoDouble(second, 0x40) < playerInfoDouble(first, 0x40))
            {
                return 1;
            }
        }
    }
    return 0;
}

void MG03ScoreboardLayout::Init(int value)
{
    m_localPlayerIdx = value;
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

unsigned int MG03ScoreboardLayout::getNumberOfColumns()
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    return miniGame->ArePointsActive() ? 5 : 3;
}

std::wstring MG03ScoreboardLayout::getStatForRound(const StatsUID&, int)
{
    return std::wstring(L"---");
}

UIString MG03ScoreboardLayout::getLabel2(const StatsUID& uid)
{
    return UIString(getStatForRound(uid, 0));
}

UIString MG03ScoreboardLayout::getLabel3(const StatsUID& uid)
{
    if (Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
    {
        return UIString(L"");
    }
    return UIString(getStatForRound(uid, 1));
}

UIString MG03ScoreboardLayout::getLabel4(const StatsUID& uid)
{
    if (Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
    {
        return UIString(L"");
    }
    return UIString(getStatForRound(uid, 2));
}

UIString MG03ScoreboardLayout::getLabel5(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_localPlayerIdx]->GetLevel()->GetClientScoreboard();
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (!scoreboard->HasStats(uid))
    {
        return UIString(0xfebed1b8);
    }
    if (scoreboard->IsOverallGlideWinner(uid, miniGame->ArePointsActive()))
    {
        return UIString(0x9148f7e8);
    }
    return UIString(L"");
}

UIString MG03ScoreboardLayout::getLabel6(const StatsUID&)
{
    return UIString(L"");
}

UIString MG03ScoreboardLayout::getLabel7(const StatsUID&)
{
    return UIString(L"");
}

unsigned int MG03ScoreboardLayout::getStatIcon2(const EScoreboardType&)
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    return miniGame->ArePointsActive() ? 0x70 : 0x74;
}

unsigned int MG03ScoreboardLayout::getStatIcon3(const EScoreboardType&)
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (miniGame->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
        return 0xffffffffu;
    return miniGame->ArePointsActive() ? 0x71 : 0x75;
}

unsigned int MG03ScoreboardLayout::getStatIcon4(const EScoreboardType&)
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (miniGame->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
        return 0xffffffffu;
    return miniGame->ArePointsActive() ? 0x72 : 0x76;
}

unsigned int MG03ScoreboardLayout::getStatIcon5(const EScoreboardType&)
{
    return 0xffffffffu;
}

unsigned int MG03ScoreboardLayout::getTitleIcon(int column, const EScoreboardType& type)
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    const bool pointsActive = miniGame->ArePointsActive();
    if (pointsActive)
    {
        if (column == 0)
            return 0x6f;
        if (column == 2)
            return 0x73;
        if (column != 4)
            return 0xffffffffu;
    }
    else
    {
        if (column == 0)
            return 0x73;
        if (column != 2)
            return 0xffffffffu;
    }
    return type == EScoreboardType_Scoreboard ? 0x77 : 3;
}

std::wstring MG03ScoreboardLayout::GetRowLabel(int column, const StatsUID& uid, const EScoreboardType& type)
{
    if (type != EScoreboardType_Scoreboard)
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
