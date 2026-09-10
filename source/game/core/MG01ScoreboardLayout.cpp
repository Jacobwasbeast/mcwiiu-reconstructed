#include "game/core/MG01ScoreboardLayout.hpp"

#include "game/core/ClientScoreboard.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StatsUID.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringUtils.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

MG01ScoreboardLayout::MG01ScoreboardLayout(EScoreboardType)
    : IScoreboardLayout()
{
}

MG01ScoreboardLayout::~MG01ScoreboardLayout()
{
}

namespace
{
int playerInfoInt(const PlayerInfo* info, unsigned int offset)
{
    return *reinterpret_cast<const int*>(reinterpret_cast<const char*>(info) + offset);
}
}

unsigned int MG01ScoreboardLayout::SORTBYSCORE(PlayerInfo* first, PlayerInfo* second)
{
    const char* firstBytes = reinterpret_cast<const char*>(first);
    const char* secondBytes = reinterpret_cast<const char*>(second);

    if (firstBytes[0x38] == 0)
    {
        if (secondBytes[0x38] != 0)
        {
            return 0;
        }
    }
    else if (secondBytes[0x38] == 0)
    {
        return 1;
    }

    int firstValue = playerInfoInt(first, 0x24);
    int secondValue = playerInfoInt(second, 0x24);
    if (firstValue > secondValue)
    {
        return 1;
    }
    if (firstValue < secondValue)
    {
        return 0;
    }

    firstValue = playerInfoInt(first, 0x28);
    secondValue = playerInfoInt(second, 0x28);
    if (firstValue > secondValue)
    {
        return 1;
    }
    if (firstValue < secondValue)
    {
        return 0;
    }

    firstValue = playerInfoInt(first, 0x2c);
    secondValue = playerInfoInt(second, 0x2c);
    if (firstValue > secondValue)
    {
        return 1;
    }
    if (firstValue < secondValue)
    {
        return 0;
    }

    firstValue = playerInfoInt(first, 0x30);
    secondValue = playerInfoInt(second, 0x30);
    if (firstValue > secondValue)
    {
        return 1;
    }
    if (firstValue < secondValue)
    {
        return 0;
    }

    firstValue = playerInfoInt(first, 0x34);
    secondValue = playerInfoInt(second, 0x34);
    if (secondValue > firstValue)
    {
        return 1;
    }
    if (secondValue < firstValue)
    {
        return 0;
    }

    if (firstBytes[0x39] == 0)
    {
        return 0;
    }
    if (secondBytes[0x39] != 0)
    {
        return 0;
    }
    return 1;
}

void MG01ScoreboardLayout::Init(int value)
{
    m_localPlayerIdx = value;
}

unsigned int MG01ScoreboardLayout::getNumberOfColumns()
{
    if (Minecraft::GetInstance()->GetMiniGame()->GetRoundCount() == 1)
    {
        return 2;
    }
    return 3;
}

boost::shared_ptr<ItemInstance> MG01ScoreboardLayout::getItemForSlot(int, int, const unsigned int&, const EScoreboardType&)
{
    return boost::shared_ptr<ItemInstance>();
}

UIString MG01ScoreboardLayout::getLabel2(const StatsUID&)
{
    return UIString(0x5450e4b5);
}

UIString MG01ScoreboardLayout::getLabel3(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_localPlayerIdx]->GetLevel()->GetClientScoreboard();
    const int rounds = static_cast<int>(Minecraft::GetInstance()->GetMiniGame()->GetRoundCount());
    int total = 0;
    for (int round = 0; round < rounds; ++round)
    {
        total += scoreboard->GetDamageDealt(uid, round);
    }
    std::wstring text(L10N::GetString(0x7cea6c57));
    text = replaceAll(text, L"{*DAMAGE*}", _toString<int>(total));
    return UIString(text);
}

UIString MG01ScoreboardLayout::getLabel4(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_localPlayerIdx]->GetLevel()->GetClientScoreboard();
    const int rounds = static_cast<int>(Minecraft::GetInstance()->GetMiniGame()->GetRoundCount());
    int total = 0;
    for (int round = 0; round < rounds; ++round)
    {
        total += scoreboard->GetDamageReceived(uid, round);
    }
    std::wstring text(L10N::GetString(0x2b967c02));
    text = replaceAll(text, L"{*DAMAGE*}", _toString<int>(total));
    return UIString(text);
}

UIString MG01ScoreboardLayout::getLabel5(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_localPlayerIdx]->GetLevel()->GetClientScoreboard();
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (!miniGame->HasMultipleRounds() && miniGame->GetLivesPerRound() == 1
        && scoreboard->GetSurvived(uid, -1))
    {
        if (!scoreboard->IsWinner(uid, -1))
        {
            return UIString(0x2473f5a1);
        }
    }
    else
    {
        if (!scoreboard->HasStats(uid))
        {
            return UIString(0xfebed1b8);
        }
        if (!scoreboard->IsOverallWinner(uid))
        {
            return UIString(0xea95ecf3);
        }
    }
    return UIString(0x9148f7e8);
}

UIString MG01ScoreboardLayout::getLabel6(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_localPlayerIdx]->GetLevel()->GetClientScoreboard();
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (!miniGame->HasMultipleRounds() && miniGame->GetLivesPerRound() == 1)
    {
        if (!scoreboard->GetSurvived(uid, -1))
        {
            return UIString(scoreboard->GetKiller(uid, -1, -1));
        }
    }
    else
    {
        if (!scoreboard->IsOverallWinner(uid) && scoreboard->HasStats(uid))
        {
            return UIString(scoreboard->GetNemesis(uid));
        }
    }
    return UIString(L"");
}

UIString MG01ScoreboardLayout::getLabel7(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_localPlayerIdx]->GetLevel()->GetClientScoreboard();
    if (!scoreboard->HasStats(uid))
    {
        return UIString(L"");
    }
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (!miniGame->HasMultipleRounds() && miniGame->GetLivesPerRound() == 1)
    {
        return UIString(0x5f46455c);
    }
    return UIString(0x51f24780);
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
    if (column == static_cast<int>(getNumberOfColumns()) - 3)
    {
        return 4;
    }
    if (column != static_cast<int>(getNumberOfColumns()) - 2 &&
        column == static_cast<int>(getNumberOfColumns()) - 1 &&
        type != EScoreboardType_Scoreboard)
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
