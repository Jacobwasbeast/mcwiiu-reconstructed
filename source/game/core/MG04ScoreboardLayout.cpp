#include "game/core/MG04ScoreboardLayout.hpp"
#include "game/core/StringIds.hpp"

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
}

bool MG04ScoreboardLayout::SORTBYSCORE(PlayerInfo* first, PlayerInfo* second)
{
    const MG04PlayerInfo* firstInfo = static_cast<const MG04PlayerInfo*>(first);
    const MG04PlayerInfo* secondInfo = static_cast<const MG04PlayerInfo*>(second);
    int firstScore = firstInfo->votesReceived[0] + firstInfo->votesReceived[2] + firstInfo->votesReceived[3] * 2;
    int secondScore = secondInfo->votesReceived[0] + secondInfo->votesReceived[2] + secondInfo->votesReceived[3] * 2;
    if (firstScore != secondScore)
    {
        return firstScore > secondScore;
    }
    if (firstInfo->votesReceived[3] != secondInfo->votesReceived[3])
    {
        return firstInfo->votesReceived[3] > secondInfo->votesReceived[3];
    }
    if (firstInfo->votesReceived[2] != secondInfo->votesReceived[2])
    {
        return firstInfo->votesReceived[2] > secondInfo->votesReceived[2];
    }
    int firstValue = firstInfo->votesGiven[0] + firstInfo->votesGiven[2] + firstInfo->votesGiven[3] * 2;
    int secondValue = secondInfo->votesGiven[0] + secondInfo->votesGiven[2] + secondInfo->votesGiven[3] * 2;
    if (firstValue != secondValue)
    {
        return firstValue > secondValue;
    }
    if (firstInfo->votesGiven[3] != secondInfo->votesGiven[3])
    {
        return firstInfo->votesGiven[3] > secondInfo->votesGiven[3];
    }
    if (firstInfo->votesGiven[2] == secondInfo->votesGiven[2])
    {
        return false;
    }
    return firstInfo->votesGiven[2] > secondInfo->votesGiven[2];
}

void MG04ScoreboardLayout::Init(int value)
{
    m_userIndex = value;
    ClearPlayersInfo();

    const int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer =
        Minecraft::GetInstance()->mLocalPlayers[value];

    for (int index = 0; index < playerCount; ++index)
    {
        INetworkPlayer* networkPlayer = CGameNetworkManager::sInstance.GetPlayerByIndex(index);
        if (networkPlayer != 0)
        {
            PlayerInfo* info = new MG04PlayerInfo();
            BuildPlayerInfo(networkPlayer, info);
            m_players.push_back(info);
        }
    }

    std::sort(m_players.begin(), m_players.end(), SORTBYSCORE);
}

UIString MG04ScoreboardLayout::getLabel2(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const int blocks = scoreboard->GetBlocksPlaced(uid, -1);
    std::wstring text(L"");
    text = replaceAll(text, L"{*BLOCKS*}", _toString<unsigned int>(blocks));
    return UIString(text);
}

UIString MG04ScoreboardLayout::getLabel3(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const int blocks = scoreboard->GetBlocksDestroyed(uid, -1);
    std::wstring text(L10N::GetString(IDS_MG_BLOCKS_DESTROYED));
    text = replaceAll(text, L"{*BLOCKS*}", _toString<unsigned int>(blocks));
    return UIString(text);
}

UIString MG04ScoreboardLayout::getLabel4(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const int distance = scoreboard->GetTotalDistanceTravelled(uid);
    std::wstring text(L10N::GetString(IDS_MG_DISTANCE_TRAVELLED));
    text = replaceAll(text, L"{*DELTA*}", _toString<unsigned int>(distance));
    return UIString(text);
}

UIString MG04ScoreboardLayout::getLabel5(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    if (scoreboard->IsOverallWinner(uid))
    {
        return UIString(IDS_MG_WINNER);
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

std::wstring MG04ScoreboardLayout::GetRowLabel(int column, const StatsUID& uid, const EScoreboardType& type)
{
    if (type != EScoreboardType_Scoreboard)
    {
        ClientScoreboard* scoreboard =
            Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
        if (column == 0)
            return _toString<unsigned short>(scoreboard->GetVotesReceived(uid, -1)[1]);
        if (column == 1)
            return _toString<unsigned short>(scoreboard->GetVotesReceived(uid, -1)[2]);
        if (column == 2)
            return _toString<unsigned short>(scoreboard->GetVotesReceived(uid, -1)[3]);
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
    if (info != 0)
    {
        return 0;
    }
    return 0x6e;
}

void MG04ScoreboardLayout::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* baseInfo)
{
    MG04PlayerInfo* info = static_cast<MG04PlayerInfo*>(baseInfo);
    const StatsUID uid = StatsUID::NETWORKPLAYER_SUID(player);
    IScoreboardLayout::BuildPlayerInfo(player, info);

    boost::shared_ptr<AbstractClientPlayer> localPlayer =
        Minecraft::GetInstance()->GetLocalPlayerByName(player->GetOnlineName());

    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();

    memcpy(info->votesReceived, scoreboard->GetVotesReceived(uid, -1), sizeof(info->votesReceived));
    memcpy(info->votesGiven, scoreboard->GetVotesGiven(uid, -1), sizeof(info->votesGiven));
    info->blocksPlaced = scoreboard->GetBlocksPlaced(uid, -1);
    info->blocksDestroyed = scoreboard->GetBlocksDestroyed(uid, -1);
}

PlayerInfo* MG04ScoreboardLayout::CreatePlayerInfo()
{
    return new MG04PlayerInfo();
}

void MG04ScoreboardLayout::AddPlayer(unsigned char smallId)
{
    PlayerInfo* info = new MG04PlayerInfo();
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(smallId);
    BuildPlayerInfo(player, info);
    m_players.push_back(info);
}

void MG04ScoreboardLayout::UpdatePlayer(int index, PlayerInfo*, PlayerInfo* newInfo, bool)
{
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
