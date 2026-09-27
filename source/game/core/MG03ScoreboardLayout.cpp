#include "game/core/MG03ScoreboardLayout.hpp"
#include "game/core/StringIds.hpp"

#include <algorithm>
#include <sstream>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StatsUID.hpp"
#include "game/core/System.hpp"
#include "game/core/L10N.hpp"
#include "game/core/StringConversion.hpp"
#include "game/ui/UIScene_HUD.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

MG03ScoreboardLayout::MG03ScoreboardLayout(EScoreboardType type)
    : IScoreboardLayout()
    , m_type(type)
{
}

MG03ScoreboardLayout::~MG03ScoreboardLayout()
{
}

namespace
{
std::wstring formatProgressTime(int milliseconds)
{
    std::wstringstream text;
    text << (milliseconds / 60000) << L":";
    const int seconds = (milliseconds / 1000) % 60;
    const int millis = milliseconds % 1000;
    if (seconds < 10)
        text << L"0";
    text << seconds << L":";
    if (millis < 100)
        text << L"0";
    if (millis < 10)
        text << L"0";
    text << millis;
    return text.str();
}

}

unsigned int MG03ScoreboardLayout::SORTBYSCORE(PlayerInfo* first, PlayerInfo* second)
{
    if (first->highlight == 0)
    {
        if (second->highlight != 0)
        {
            return 1;
        }
    }
    else if (second->highlight == 0)
    {
        return 0;
    }

    int secondScore = second->points;
    int firstScore = first->points;
    if (secondScore < firstScore)
    {
        return 1;
    }
    if (secondScore <= firstScore)
    {
        if (first->endZoneEntered == 0)
        {
            if (second->endZoneEntered != 0)
            {
                return 0;
            }
        }
        else if (second->endZoneEntered == 0)
        {
            return 1;
        }

        int firstValue = first->lifeSpan;
        int secondValue = second->lifeSpan;
        if (firstValue <= secondValue)
        {
            if (firstValue < secondValue)
            {
                return 1;
            }
            if (first->damageDealt < second->damageDealt)
            {
                return 1;
            }
        }
        if (firstValue < secondValue ||
            (firstValue <= secondValue && first->damageDealt <= second->damageDealt))
        {
            secondScore = second->secondaryTargets;
            firstScore = first->secondaryTargets;
            if (secondScore < firstScore)
            {
                return 1;
            }
            if (secondScore <= firstScore &&
                second->mapTraversed < first->mapTraversed)
            {
                return 1;
            }
        }
    }
    return 0;
}

void MG03ScoreboardLayout::Init(int value)
{
    m_userIndex = value;
    ClearPlayersInfo();
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[value];
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

std::wstring MG03ScoreboardLayout::getStatForRound(const StatsUID& uid, int round)
{
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex];
    ClientScoreboard* scoreboard = localPlayer->GetLevel()->GetClientScoreboard();
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();

    if (miniGame->ArePointsActive())
    {
        return _toString<int>(scoreboard->GetPoints(uid, round));
    }

    if (scoreboard->HasStats(uid))
    {
        return UIScene_HUD::formatTime(scoreboard->GetLifeSpan(uid, round), false);
    }

    return L10N::GetString(IDS_DNF);
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
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (!scoreboard->HasStats(uid))
    {
        return UIString(IDS_MG_SPECTATOR);
    }
    if (scoreboard->IsOverallGlideWinner(uid, miniGame->ArePointsActive()))
    {
        return UIString(IDS_MG_WINNER);
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
    if (type != EScoreboardType_Scoreboard || (column != 0 && column != 2))
        return std::wstring();

    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    ClientMasterGameMode* gameMode = Minecraft::GetInstance()->GetClientMasterGameMode();
    const bool best = gameMode->IsWaitingToStart() || gameMode->IsRoundStarting();
    int round = -1;

    if (miniGame->ArePointsActive())
    {
        if (column == 0)
        {
            const int points = best ? scoreboard->GetBestPoints(uid, &round)
                                    : scoreboard->GetPoints(uid, -1);
            return scoreboard->HasStats(uid) ? _toString<int>(points) : L"---";
        }

        if (best)
            scoreboard->GetBestPoints(uid, &round);
        return scoreboard->HasStats(uid) ?
            UIScene_HUD::formatTime(scoreboard->GetLifeSpan(uid, best ? round : -1), false) : L"---";
    }

    if (column == 0)
    {
        const int life = best ? scoreboard->GetBestLifeSpan(uid, &round)
                              : scoreboard->GetLifeSpan(uid, -1);
        return scoreboard->HasStats(uid) ? UIScene_HUD::formatTime(life, false) : L"---";
    }

    return L"---";
}

boost::shared_ptr<ItemInstance> MG03ScoreboardLayout::getItemForSlot(int row, int column, const unsigned int& value, const EScoreboardType& type)
{
    if (type != EScoreboardType_Scoreboard || row == -1)
    {
        return boost::shared_ptr<ItemInstance>();
    }

    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (miniGame->ArePointsActive())
    {
        if (row == 4)
        {
            return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 3));
        }
    }
    else if (row == 2)
    {
        return boost::shared_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 3));
    }

    return boost::shared_ptr<ItemInstance>();
}

unsigned char MG03ScoreboardLayout::GetEndBarIcon(PlayerInfo* info)
{
    return info->teamInvite != 0 ? 2 : 0;
}

void MG03ScoreboardLayout::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info)
{
    IScoreboardLayout::BuildPlayerInfo(player, info);

    Minecraft* minecraft = Minecraft::GetInstance();
    const std::wstring& playerName = player->GetDisplayName();
    boost::shared_ptr<AbstractClientPlayer> localPlayer = minecraft->GetLocalPlayerByName(playerName);

    bool isLocalPlayerActive = false;
    if (localPlayer != 0 && localPlayer->isAlive())
    {
        isLocalPlayerActive = true;
    }

    const MiniGameDef* miniGame = minecraft->GetMiniGame();
    ClientMasterGameMode* gameMode = minecraft->GetClientMasterGameMode();
    ClientScoreboard* scoreboard =
        minecraft->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const StatsUID uid = StatsUID::NETWORKPLAYER_SUID(player);

    int round = -1;
    int lifespan = 0;
    int points = 0;
    int secondaryTargets = 0;
    bool endZoneEntered = false;
    double mapTraversed = 100.0;

    const bool useBestStats =
        m_type == EScoreboardType_Award || gameMode->IsWaitingToStart() || gameMode->IsRoundStarting();

    if (miniGame->ArePointsActive())
    {
        if (useBestStats)
        {
            points = scoreboard->GetBestPoints(uid, &round);
            lifespan = scoreboard->GetLifeSpan(uid, round);
            secondaryTargets = scoreboard->GetSecondaryTargets(uid, round);
            endZoneEntered = scoreboard->GetEndZoneEntered(uid, round);
            mapTraversed = scoreboard->GetPercentageOfMapTraversed(uid, round);
        }
        else
        {
            points = scoreboard->GetPoints(uid, -1);
            endZoneEntered = scoreboard->GetEndZoneEntered(uid, -1);
            if (!endZoneEntered)
            {
                lifespan = static_cast<int>(System::processTimeInMilliSecs() - gameMode->GetLocalRoundTimeStart());
                if (gameMode->IsInLobbyState() || !scoreboard->HasStats(uid))
                {
                    lifespan = 0;
                }
                secondaryTargets = scoreboard->GetSecondaryTargets(uid, -1);
                endZoneEntered = scoreboard->GetEndZoneEntered(uid, -1);
                mapTraversed = scoreboard->GetPercentageOfMapTraversed(uid, -1);
            }
            else
            {
                lifespan = scoreboard->GetLifeSpan(uid, -1);
                secondaryTargets = scoreboard->GetSecondaryTargets(uid, -1);
                endZoneEntered = scoreboard->GetEndZoneEntered(uid, -1);
            }
        }
    }
    else if (useBestStats)
    {
        lifespan = scoreboard->GetBestLifeSpan(uid, &round);
        endZoneEntered = scoreboard->GetEndZoneEntered(uid, round);
        mapTraversed = scoreboard->GetPercentageOfMapTraversed(uid, round);
    }
    else
    {
        endZoneEntered = scoreboard->GetEndZoneEntered(uid, -1);
        if (!endZoneEntered)
        {
            lifespan = static_cast<int>(System::processTimeInMilliSecs() - gameMode->GetLocalRoundTimeStart());
            if (gameMode->IsInLobbyState() || !scoreboard->HasStats(uid))
            {
                lifespan = 0;
            }
            endZoneEntered = scoreboard->GetEndZoneEntered(uid, -1);
            mapTraversed = scoreboard->GetPercentageOfMapTraversed(uid, -1);
        }
        else
        {
            lifespan = scoreboard->GetLifeSpan(uid, -1);
            endZoneEntered = scoreboard->GetEndZoneEntered(uid, -1);
        }
    }

    const unsigned char highlight = (isLocalPlayerActive && !endZoneEntered) ? 1 : 0;
    info->highlight = highlight;
    info->endZoneEntered = static_cast<unsigned char>(endZoneEntered);
    info->damageDealt = lifespan >> 31;
    info->points = points;
    info->secondaryTargets = secondaryTargets;
    info->mapTraversed = mapTraversed;
    info->lifeSpan = lifespan;

    MultiPlayerGameMode* localGameMode = minecraft->mLocalPlayers[m_userIndex]->GetLocalGameMode();
    info->teamInvite = static_cast<unsigned char>(localGameMode->IsTeamInvite(playerName));
    info->colour = static_cast<short>(gApp.GetPlayerColour(info->smallId));
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
    PlayerInfo* info = m_players[index];
    info->smallId = newInfo->smallId;
    info->voiceStatus = newInfo->voiceStatus;
    info->colour = newInfo->colour;
    info->name = newInfo->name;
}

std::wstring MG03ScoreboardLayout::GetCustomProgressDetails(const StatsUID& uid, int& value)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    int round = -1;
    int personal;
    int total;
    std::wstring details;

    if (miniGame->ArePointsActive())
    {
        personal = scoreboard->GetBestPoints(uid, &round);
        total = scoreboard->GetBestPoints();
        value = total == 0 ? 100 : static_cast<int>((static_cast<float>(personal) / total) * 100.0f);
        details = _toString<int>(personal == total ? personal : personal - total);
    }
    else
    {
        personal = scoreboard->GetBestLifeSpan(uid, &round);
        total = scoreboard->GetBestLifeSpan();
        value = total == 0 ? 100 : static_cast<int>((static_cast<float>(personal) / total) * 100.0f);
        details = formatProgressTime(personal == total ? personal : total - personal);
    }

    if (!scoreboard->HasStats(uid))
    {
        value = 0;
        return L10N::GetString(IDS_DNF);
    }
    return details;
}

bool MG03ScoreboardLayout::showLifeSpanProgress()
{
    return false;
}
