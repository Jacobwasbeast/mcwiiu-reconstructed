#include "game/core/MG04ScoreboardLayout.hpp"

#include <algorithm>
#include <cstring>
#include <sstream>

#include "game/core/ClientScoreboard.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StatsUID.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringUtils.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

MG04ScoreboardLayout::MG04ScoreboardLayout() : IScoreboardLayout()
{
}

MG04ScoreboardLayout::~MG04ScoreboardLayout()
{
}

namespace
{
int playerInfoInt(const PlayerInfo* info, unsigned int offset)
{
    return *reinterpret_cast<const int*>(reinterpret_cast<const char*>(info) + offset);
}

int& playerInfoIntMut(PlayerInfo* info, unsigned int offset)
{
    return *reinterpret_cast<int*>(reinterpret_cast<char*>(info) + offset);
}
}

bool MG04ScoreboardLayout::SORTBYSCORE(PlayerInfo* first, PlayerInfo* second)
{
    int firstScore =
        playerInfoInt(first, 0x34) + playerInfoInt(first, 0x3c) + playerInfoInt(first, 0x40) * 2;
    int secondScore =
        playerInfoInt(second, 0x34) + playerInfoInt(second, 0x3c) + playerInfoInt(second, 0x40) * 2;
    if (firstScore != secondScore)
    {
        return secondScore < firstScore;
    }
    if (playerInfoInt(first, 0x40) != playerInfoInt(second, 0x40))
    {
        return playerInfoInt(second, 0x40) < playerInfoInt(first, 0x40);
    }
    if (playerInfoInt(first, 0x3c) != playerInfoInt(second, 0x3c))
    {
        return playerInfoInt(second, 0x3c) < playerInfoInt(first, 0x3c);
    }
    int firstValue =
        playerInfoInt(first, 0x24) + playerInfoInt(first, 0x2c) + playerInfoInt(first, 0x30) * 2;
    int secondValue =
        playerInfoInt(second, 0x24) + playerInfoInt(second, 0x2c) + playerInfoInt(second, 0x30) * 2;
    if (firstValue != secondValue)
    {
        return secondValue < firstValue;
    }
    if (playerInfoInt(first, 0x30) != playerInfoInt(second, 0x30))
    {
        return playerInfoInt(second, 0x30) < playerInfoInt(first, 0x30);
    }
    if (playerInfoInt(first, 0x2c) == playerInfoInt(second, 0x2c))
    {
        return false;
    }
    return playerInfoInt(second, 0x2c) < playerInfoInt(first, 0x2c);
}

void MG04ScoreboardLayout::Init(int value)
{
    m_localPlayerIdx = value;
    ClearPlayersInfo();

    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer =
        Minecraft::GetInstance()->mLocalPlayers[value];

    const int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
    for (int index = 0; index < playerCount; ++index)
    {
        INetworkPlayer* networkPlayer = CGameNetworkManager::sInstance.GetPlayerByIndex(index);
        if (networkPlayer != 0)
        {
            PlayerInfo* info = CreatePlayerInfo();
            BuildPlayerInfo(networkPlayer, info);
            m_players.push_back(info);
        }
    }

    std::sort(m_players.begin(), m_players.end(), SORTBYSCORE);
}

UIString MG04ScoreboardLayout::getLabel2(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_localPlayerIdx]->GetLevel()->GetClientScoreboard();
    const int blocks = scoreboard->GetBlocksPlaced(uid, -1);
    std::wstring text(L"");
    text = replaceAll(text, L"{*BLOCKS*}", _toString<unsigned int>(blocks));
    return UIString(text);
}

UIString MG04ScoreboardLayout::getLabel3(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_localPlayerIdx]->GetLevel()->GetClientScoreboard();
    const int blocks = scoreboard->GetBlocksDestroyed(uid, -1);
    std::wstring text(L10N::GetString(0x782d903f));
    text = replaceAll(text, L"{*BLOCKS*}", _toString<unsigned int>(blocks));
    return UIString(text);
}

UIString MG04ScoreboardLayout::getLabel4(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_localPlayerIdx]->GetLevel()->GetClientScoreboard();
    const int distance = scoreboard->GetTotalDistanceTravelled(uid);
    std::wstring text(L10N::GetString(0x2815dc90));
    text = replaceAll(text, L"{*DELTA*}", _toString<unsigned int>(distance));
    return UIString(text);
}

UIString MG04ScoreboardLayout::getLabel5(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_localPlayerIdx]->GetLevel()->GetClientScoreboard();
    if (scoreboard->IsOverallWinner(uid))
    {
        return UIString(0x9148f7e8);
    }
    return UIString(L"");
}

UIString MG04ScoreboardLayout::getLabel6(const StatsUID&)
{
    return UIString(L"");
}

UIString MG04ScoreboardLayout::getLabel7(const StatsUID&)
{
    return UIString(0xe063a16f);
}

unsigned int MG04ScoreboardLayout::getStatIcon2(const EScoreboardType&)
{
    return 5;
}

unsigned int MG04ScoreboardLayout::getStatIcon3(const EScoreboardType&)
{
    return 5;
}

unsigned int MG04ScoreboardLayout::getStatIcon4(const EScoreboardType&)
{
    return 7;
}

unsigned int MG04ScoreboardLayout::getStatIcon5(const EScoreboardType&)
{
    return 7;
}

unsigned int MG04ScoreboardLayout::getTitleIcon(int column, const EScoreboardType&)
{
    if (static_cast<unsigned int>(column) < 3)
    {
        return 0x77;
    }
    return 0xffffffffu;
}

std::wstring MG04ScoreboardLayout::GetRowLabel(int column, const StatsUID&, const EScoreboardType& type)
{
    if (type == EScoreboardType_Scoreboard)
        return std::wstring(L"");
    if (column >= 0 && column < 3)
    {
        std::wstringstream text;
        text << column;
        return text.str();
    }
    return std::wstring(L"");
}

boost::shared_ptr<ItemInstance> MG04ScoreboardLayout::getItemForSlot(int row, int, const unsigned int&, const EScoreboardType& type)
{
    if (type == EScoreboardType_Scoreboard)
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
    const StatsUID uid = StatsUID::NETWORKPLAYER_SUID(player);
    IScoreboardLayout::BuildPlayerInfo(player, info);

    const std::wstring playerName(player->GetOnlineName());
    boost::shared_ptr<AbstractClientPlayer> localPlayer =
        Minecraft::GetInstance()->GetLocalPlayerByName(playerName);

    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_localPlayerIdx]->GetLevel()->GetClientScoreboard();

    memcpy(reinterpret_cast<char*>(info) + 0x24, scoreboard->GetVotesGiven(uid, -1), 16);
    memcpy(reinterpret_cast<char*>(info) + 0x34, scoreboard->GetVotesReceived(uid, -1), 16);
    playerInfoIntMut(info, 0x44) = scoreboard->GetBlocksPlaced(uid, -1);
    playerInfoIntMut(info, 0x48) = scoreboard->GetBlocksDestroyed(uid, -1);
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

unsigned int MG04ScoreboardLayout::getNumberOfColumns()
{
    return 4;
}
