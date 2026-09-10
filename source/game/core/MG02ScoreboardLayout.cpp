#include "game/core/MG02ScoreboardLayout.hpp"

#include <algorithm>

#include "game/core/ClientScoreboard.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StatsUID.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringUtils.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

MG02ScoreboardLayout::MG02ScoreboardLayout()
    : IScoreboardLayout()
{
}

MG02ScoreboardLayout::~MG02ScoreboardLayout()
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
}

bool MG02ScoreboardLayout::SORTBYSCORE(PlayerInfo* first, PlayerInfo* second)
{
    const char* firstBytes = reinterpret_cast<const char*>(first);
    const char* secondBytes = reinterpret_cast<const char*>(second);

    if (firstBytes[0x28] == 0)
    {
        if (secondBytes[0x28] != 0)
        {
            return false;
        }
    }
    else if (secondBytes[0x28] == 0)
    {
        return true;
    }

    if (playerInfoInt(second, 0x40) < playerInfoInt(first, 0x40))
    {
        return true;
    }
    if (playerInfoInt(second, 0x40) > playerInfoInt(first, 0x40))
    {
        return false;
    }

    if (playerInfoUint(first, 0x2c) > playerInfoUint(second, 0x2c))
    {
        return true;
    }
    if (playerInfoUint(first, 0x2c) < playerInfoUint(second, 0x2c))
    {
        return false;
    }

    if (playerInfoUint(first, 0x30) > playerInfoUint(second, 0x30))
    {
        return true;
    }
    if (playerInfoUint(first, 0x30) < playerInfoUint(second, 0x30))
    {
        return false;
    }

    if (playerInfoUint(second, 0x34) < playerInfoUint(first, 0x34))
    {
        return true;
    }
    if (playerInfoUint(second, 0x34) > playerInfoUint(first, 0x34))
    {
        return false;
    }

    return playerInfoInt(first, 0x24) > playerInfoInt(second, 0x24);
}

void MG02ScoreboardLayout::Init(int value)
{
    m_localPlayerIdx = value;
    ClearPlayersInfo();
}

unsigned int MG02ScoreboardLayout::getNumberOfColumns()
{
    if (Minecraft::GetInstance()->GetMiniGame()->GetLivesPerRound() == 1)
    {
        return 2;
    }
    return 3;
}

UIString MG02ScoreboardLayout::getLabel2(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_localPlayerIdx]->GetLevel()->GetClientScoreboard();
    const int blocks = scoreboard->GetTotalBlocksDestroyed(uid);
    std::wstring text(L10N::GetString(0x782d903f));
    text = replaceAll(text, L"{*BLOCKS*}", _toString<unsigned int>(blocks));
    return UIString(text);
}

UIString MG02ScoreboardLayout::getLabel3(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_localPlayerIdx]->GetLevel()->GetClientScoreboard();
    const int jumps = scoreboard->GetTotalJumps(uid);
    std::wstring text(L10N::GetString(0xd6524620));
    text = replaceAll(text, L"{*JUMPS*}", _toString<unsigned int>(jumps));
    return UIString(text);
}

UIString MG02ScoreboardLayout::getLabel4(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_localPlayerIdx]->GetLevel()->GetClientScoreboard();
    const int distance = scoreboard->GetTotalDistanceTravelled(uid);
    std::wstring text(L10N::GetString(0x2815dc90));
    text = replaceAll(text, L"{*DELTA*}", _toString<unsigned int>(distance));
    return UIString(text);
}

UIString MG02ScoreboardLayout::getLabel5(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_localPlayerIdx]->GetLevel()->GetClientScoreboard();
    if (scoreboard->IsOverallWinner(uid))
    {
        return UIString(0x9148f7e8);
    }
    return UIString(L"");
}

UIString MG02ScoreboardLayout::getLabel6(const StatsUID&)
{
    return UIString(L"");
}

UIString MG02ScoreboardLayout::getLabel7(const StatsUID&)
{
    return UIString(0xe063a16f);
}

unsigned int MG02ScoreboardLayout::getStatIcon2(const EScoreboardType&) { return 5; }
unsigned int MG02ScoreboardLayout::getStatIcon3(const EScoreboardType&) { return 6; }
unsigned int MG02ScoreboardLayout::getStatIcon4(const EScoreboardType&) { return 7; }
unsigned int MG02ScoreboardLayout::getStatIcon5(const EScoreboardType&) { return 0xffffffff; }

unsigned int MG02ScoreboardLayout::getTitleIcon(int column, const EScoreboardType& type)
{
    if (column == 0)
        return 4;
    if (column == (int)getNumberOfColumns() - 1 && type != EScoreboardType_Scoreboard)
        return 3;
    return 0xffffffff;
}

std::wstring MG02ScoreboardLayout::GetRowLabel(int row, const StatsUID& uid, const EScoreboardType&)
{
    if (row == 0)
    {
        ClientScoreboard* scoreboard =
            Minecraft::GetInstance()->mLocalPlayers[m_localPlayerIdx]->GetLevel()->GetClientScoreboard();
        return _toString<int>(scoreboard->GetRoundsWon(uid));
    }
    if (row == 1 && getNumberOfColumns() > 2)
    {
        ClientScoreboard* scoreboard =
            Minecraft::GetInstance()->mLocalPlayers[m_localPlayerIdx]->GetLevel()->GetClientScoreboard();
        return _toString<int>(scoreboard->GetDeaths(uid, -1));
    }
    return std::wstring(L"");
}

boost::shared_ptr<ItemInstance> MG02ScoreboardLayout::getItemForSlot(int column, int, const unsigned int&, const EScoreboardType& type)
{
    if (column != -1)
    {
        if (type == EScoreboardType_Scoreboard)
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
    return info == 0 ? 0x6e : (unsigned int)(info->voiceStatus ^ 1);
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
