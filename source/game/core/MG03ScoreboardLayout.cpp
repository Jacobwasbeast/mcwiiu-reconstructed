#include "game/core/MG03ScoreboardLayout.hpp"
#include "game/core/StringFormat.hpp"
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

unsigned int MG03ScoreboardLayout::SORTBYSCORE(PlayerInfo* first, PlayerInfo* second)
{
    const MG03PlayerInfo* firstInfo = static_cast<const MG03PlayerInfo*>(first);
    const MG03PlayerInfo* secondInfo = static_cast<const MG03PlayerInfo*>(second);
    if (firstInfo->highlight == 0)
    {
        if (secondInfo->highlight != 0)
        {
            return 1;
        }
    }
    else if (secondInfo->highlight == 0)
    {
        return 0;
    }

    int secondScore = secondInfo->points;
    int firstScore = firstInfo->points;
    if (secondScore < firstScore)
    {
        return 1;
    }
    if (secondScore <= firstScore)
    {
        if (firstInfo->endZoneEntered == 0)
        {
            if (secondInfo->endZoneEntered != 0)
            {
                return 0;
            }
        }
        else if (secondInfo->endZoneEntered == 0)
        {
            return 1;
        }

        if (firstInfo->lifeSpan < secondInfo->lifeSpan)
        {
            return 1;
        }
        if (!(firstInfo->lifeSpan > secondInfo->lifeSpan))
        {
            secondScore = secondInfo->secondaryTargets;
            firstScore = firstInfo->secondaryTargets;
            if (secondScore < firstScore)
            {
                return 1;
            }
            if (secondScore <= firstScore &&
                secondInfo->mapTraversed < firstInfo->mapTraversed)
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
            PlayerInfo* info = new MG03PlayerInfo();
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
        return formatwstr(L"%d", scoreboard->GetPoints(uid, round));
    }

    if (scoreboard->GetEndZoneEntered(uid, round))
    {
        return UIScene_HUD::formatTime(static_cast<unsigned int>(scoreboard->GetLifeSpan(uid, round)), false);
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
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    ClientMasterGameMode* gameMode = Minecraft::GetInstance()->GetClientMasterGameMode();
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();

    if (!miniGame->ArePointsActive())
    {
        if (column == 0)
        {
            if (type == EScoreboardType_Award || gameMode->IsWaitingToStart() || gameMode->IsRoundStarting())
            {
                int round = -1;
                const unsigned int lifeSpan = scoreboard->GetBestLifeSpan(uid, &round);
                if (scoreboard->GetEndZoneEntered(uid, round))
                    return UIScene_HUD::formatTime(lifeSpan, false);
                if ((gameMode->IsWaitingToStart() || gameMode->IsRoundStarting()) && scoreboard->mCurrentRound == 0)
                    return L"---";
                return L10N::GetString(IDS_DNF);
            }
            if (scoreboard->GetEndZoneEntered(uid, -1))
                return UIScene_HUD::formatTime(static_cast<unsigned int>(scoreboard->GetLifeSpan(uid, -1)), false);
            return L"---";
        }
    }
    else
    {
        if (column == 0)
        {
            int points;
            if (type == EScoreboardType_Award || gameMode->IsWaitingToStart() || gameMode->IsRoundStarting())
            {
                int round = -1;
                points = scoreboard->GetBestPoints(uid, &round);
                if (!scoreboard->GetEndZoneEntered(uid, round))
                    return L10N::GetString(IDS_DNF);
            }
            else
            {
                points = scoreboard->GetPoints(uid, -1);
            }
            return formatwstr(L"%d", points);
        }
        if (column == 2)
        {
            if (type == EScoreboardType_Award || gameMode->IsWaitingToStart() || gameMode->IsRoundStarting())
            {
                int round = -1;
                scoreboard->GetBestPoints(uid, &round);
                if (scoreboard->GetEndZoneEntered(uid, round))
                    return UIScene_HUD::formatTime(static_cast<unsigned int>(scoreboard->GetLifeSpan(uid, round)), false);
                return L10N::GetString(IDS_DNF);
            }
            if (scoreboard->GetEndZoneEntered(uid, -1))
                return UIScene_HUD::formatTime(static_cast<unsigned int>(scoreboard->GetLifeSpan(uid, -1)), false);
            return L"---";
        }
    }
    return L"";
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

unsigned char MG03ScoreboardLayout::GetEndBarIcon(PlayerInfo* baseInfo)
{
    const MG03PlayerInfo* info = static_cast<const MG03PlayerInfo*>(baseInfo);
    return info->teamInvite != 0 ? 2 : 0;
}

void MG03ScoreboardLayout::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* baseInfo)
{
    MG03PlayerInfo* info = static_cast<MG03PlayerInfo*>(baseInfo);
    IScoreboardLayout::BuildPlayerInfo(player, info);

    boost::shared_ptr<AbstractClientPlayer> localPlayer =
        Minecraft::GetInstance()->GetLocalPlayerByName(player->GetOnlineName());

    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    ClientMasterGameMode* gameMode = Minecraft::GetInstance()->GetClientMasterGameMode();
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();

    const bool isSpectating = localPlayer != 0 && localPlayer->isSpectator();

    const StatsUID uid = StatsUID::NETWORKPLAYER_SUID(player);

    int round = -1;
    int lifespan = 0;
    int points = 0;
    int secondaryTargets = 0;
    unsigned char endZoneEntered = 0;
    double mapTraversed = 100.0;

    if (miniGame->ArePointsActive())
    {
        if (m_type == EScoreboardType_Award || gameMode->IsWaitingToStart() || gameMode->IsRoundStarting())
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
    else if (m_type == EScoreboardType_Award || gameMode->IsWaitingToStart() || gameMode->IsRoundStarting())
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

    const unsigned char highlight = (isSpectating && !endZoneEntered) ? 1 : 0;
    info->highlight = highlight;
    info->endZoneEntered = endZoneEntered;
    info->points = points;
    info->secondaryTargets = secondaryTargets;
    info->mapTraversed = mapTraversed;
    info->lifeSpan = lifespan;

    MultiPlayerGameMode* localGameMode = Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLocalGameMode();
    info->teamInvite = static_cast<unsigned char>(localGameMode->IsTeamInvite(player->GetOnlineName()));
    info->colour = static_cast<short>(gApp.GetPlayerColour(info->smallId));
}

PlayerInfo* MG03ScoreboardLayout::CreatePlayerInfo()
{
    return new MG03PlayerInfo();
}

void MG03ScoreboardLayout::AddPlayer(unsigned char smallId)
{
    PlayerInfo* info = new MG03PlayerInfo();
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
    std::wstring details = L"";
    int round = -1;

    if (!miniGame->ArePointsActive())
    {
        const unsigned int personal = scoreboard->GetBestLifeSpan(uid, &round);
        const unsigned int best = scoreboard->GetBestLifeSpan();
        const unsigned int behind = personal - best;
        if (behind == 0)
        {
            value = 100;
            details = formatwstr(L"%d:%.2d.%.3d", (personal / 60000) % 60, (personal / 1000) % 60, personal % 1000);
        }
        else
        {
            value = static_cast<int>((static_cast<float>(best) / static_cast<float>(personal)) * 100.0f);
            details = formatwstr(L"+%d:%.2d.%.3d", (behind / 60000) % 60, (behind / 1000) % 60, behind % 1000);
        }
    }
    else
    {
        const int personal = scoreboard->GetBestPoints(uid, &round);
        const int best = scoreboard->GetBestPoints();
        if (best - personal == 0)
        {
            value = 100;
            details = formatwstr(L"%d", personal);
        }
        else
        {
            value = static_cast<int>((static_cast<float>(static_cast<unsigned int>(personal)) /
                                      static_cast<float>(static_cast<unsigned int>(best))) * 100.0f);
            details = formatwstr(L"-%d", best - personal);
        }
    }

    if (!scoreboard->GetEndZoneEntered(uid, round))
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
