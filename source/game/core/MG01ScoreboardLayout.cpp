#include "game/core/MG01ScoreboardLayout.hpp"
#include "game/core/StringIds.hpp"

#include <algorithm>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/core/StatsUID.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringUtils.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

MG01ScoreboardLayout::MG01ScoreboardLayout(EScoreboardType type)
    : IScoreboardLayout()
{
    m_type = type;
}

MG01ScoreboardLayout::~MG01ScoreboardLayout()
{
}

bool MG01ScoreboardLayout::SORTBYSCORE(PlayerInfo* first, PlayerInfo* second)
{
    const MG01PlayerInfo* firstInfo = static_cast<const MG01PlayerInfo*>(first);
    const MG01PlayerInfo* secondInfo = static_cast<const MG01PlayerInfo*>(second);
    if (firstInfo->localPlayer == 0)
    {
        if (secondInfo->localPlayer != 0)
        {
            return 0;
        }
    }
    else if (secondInfo->localPlayer == 0)
    {
        return 1;
    }

    int firstValue = firstInfo->roundsWon;
    int secondValue = secondInfo->roundsWon;
    if (firstValue > secondValue)
    {
        return 1;
    }
    if (firstValue < secondValue)
    {
        return 0;
    }

    firstValue = firstInfo->kills;
    secondValue = secondInfo->kills;
    if (firstValue > secondValue)
    {
        return 1;
    }
    if (firstValue < secondValue)
    {
        return 0;
    }

    firstValue = firstInfo->lifeSpan;
    secondValue = secondInfo->lifeSpan;
    if (firstValue > secondValue)
    {
        return 1;
    }
    if (firstValue < secondValue)
    {
        return 0;
    }

    firstValue = firstInfo->damageDealt;
    secondValue = secondInfo->damageDealt;
    if (firstValue > secondValue)
    {
        return 1;
    }
    if (firstValue < secondValue)
    {
        return 0;
    }

    firstValue = firstInfo->damageReceived;
    secondValue = secondInfo->damageReceived;
    if (secondValue > firstValue)
    {
        return 1;
    }
    if (secondValue < firstValue)
    {
        return 0;
    }

    if (firstInfo->survived == 0)
    {
        return 0;
    }
    if (secondInfo->survived != 0)
    {
        return 0;
    }
    return 1;
}

void MG01ScoreboardLayout::Init(int value)
{
    m_userIndex = value;
    ClearPlayersInfo();

    const unsigned int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer =
        Minecraft::GetInstance()->mLocalPlayers[value];

    for (unsigned int index = 0; index < playerCount; ++index)
    {
        INetworkPlayer* networkPlayer = CGameNetworkManager::sInstance.GetPlayerByIndex(index);
        if (networkPlayer != 0)
        {
            PlayerInfo* info = new MG01PlayerInfo();
            BuildPlayerInfo(networkPlayer, info);
            m_players.push_back(info);
        }
    }

    std::sort(m_players.begin(), m_players.end(), SORTBYSCORE);
}

unsigned int MG01ScoreboardLayout::getNumberOfColumns()
{
    if (Minecraft::GetInstance()->GetMiniGame()->GetRoundCount() == 1)
    {
        return 2;
    }
    return 3;
}

boost::shared_ptr<ItemInstance> MG01ScoreboardLayout::getItemForSlot(int column, int row, const unsigned int& slot, const EScoreboardType& type)
{
    boost::shared_ptr<ItemInstance> item = ItemInstance::EMPTY;

    if (column == -1)
    {
        if (type == EScoreboardType_Award)
        {
            ClientScoreboard* scoreboard =
                Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
            const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
            const bool multiRoundOrMultiLife =
                miniGame->HasMultipleRounds() || miniGame->GetLivesPerRound() != 1;

            if (slot < m_players.size() && m_players[slot] != 0)
            {
                INetworkPlayer* networkPlayer =
                    CGameNetworkManager::sInstance.GetPlayerBySmallId(m_players[slot]->smallId);
                const StatsUID uid = StatsUID::NETWORKPLAYER_SUID(networkPlayer);

                if (row != 1)
                {
                    if (row == 4 && networkPlayer != 0 && scoreboard->HasStats(uid))
                    {
                        not_null_ptr<ItemInstance> weapon;
                        if (multiRoundOrMultiLife)
                        {
                            weapon = scoreboard->GetWeaknessWeapon(uid);
                        }
                        else
                        {
                            weapon = scoreboard->GetKillerWeapon(uid, -1);
                        }
                        if (!weapon->isEmpty())
                        {
                            item = weapon;
                        }
                        else
                        {
                            item = not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 3));
                        }
                    }
                }
                else if (networkPlayer != 0)
                {
                    not_null_ptr<ItemInstance> weapon = scoreboard->GetFavouriteWeapon(uid);
                    if (!weapon->isEmpty())
                    {
                        item = weapon;
                    }
                    else
                    {
                        item = not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 3));
                    }
                }
            }
        }
        return item;
    }

    const unsigned int columns = getNumberOfColumns();
    if (column == columns - 2)
    {
        item = not_null_ptr<ItemInstance>(new ItemInstance(Items::diamondSword, 1, 0));
    }
    else if (column == columns - 1 && type == EScoreboardType_Scoreboard)
    {
        item = not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 3));
    }
    return item;
}

UIString MG01ScoreboardLayout::getLabel2(const StatsUID&)
{
    return UIString(0x5450e4b5);
}

UIString MG01ScoreboardLayout::getLabel3(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const int rounds = static_cast<int>(Minecraft::GetInstance()->GetMiniGame()->GetRoundCount());
    int total = 0;
    for (int round = 0; round < rounds; ++round)
    {
        total += scoreboard->GetDamageDealt(uid, round);
    }
    std::wstring text(L10N::GetString(IDS_MG_DAMAGE_INFLICTED));
    text = replaceAll(text, L"{*DAMAGE*}", _toString<int>(total));
    return UIString(text);
}

UIString MG01ScoreboardLayout::getLabel4(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const int rounds = static_cast<int>(Minecraft::GetInstance()->GetMiniGame()->GetRoundCount());
    int total = 0;
    for (int round = 0; round < rounds; ++round)
    {
        total += scoreboard->GetDamageReceived(uid, round);
    }
    std::wstring text(L10N::GetString(IDS_MG_DAMAGE_RECEIVED));
    text = replaceAll(text, L"{*DAMAGE*}", _toString<int>(total));
    return UIString(text);
}

UIString MG01ScoreboardLayout::getLabel5(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (!miniGame->HasMultipleRounds() && miniGame->GetLivesPerRound() == 1
        && scoreboard->GetSurvived(uid, -1))
    {
        if (!scoreboard->IsWinner(uid, -1))
        {
            return UIString(IDS_MG_SURVIVOR);
        }
    }
    else
    {
        if (!scoreboard->HasStats(uid))
        {
            return UIString(IDS_MG_SPECTATOR);
        }
        if (!scoreboard->IsOverallWinner(uid))
        {
            return UIString(IDS_MG_NEMESIS);
        }
    }
    return UIString(IDS_MG_WINNER);
}

UIString MG01ScoreboardLayout::getLabel6(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
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
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    if (!scoreboard->HasStats(uid))
    {
        return UIString(L"");
    }
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (!miniGame->HasMultipleRounds() && miniGame->GetLivesPerRound() == 1)
    {
        return UIString(IDS_MG_FINAL_BLOW);
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

std::wstring MG01ScoreboardLayout::GetRowLabel(int column, const StatsUID& uid, const EScoreboardType& type)
{
    if (column == static_cast<int>(getNumberOfColumns()) - 3)
    {
        ClientScoreboard* scoreboard =
            Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
        return _toString<int>(scoreboard->GetRoundsWon(uid));
    }

    if (column != static_cast<int>(getNumberOfColumns()) - 2)
    {
        return std::wstring(L"");
    }

    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const int rounds = static_cast<int>(Minecraft::GetInstance()->GetMiniGame()->GetRoundCount());
    int total = 0;
    if (type == EScoreboardType_Award)
    {
        for (int round = 0; round < rounds; ++round)
        {
            total += scoreboard->GetKills(uid, round);
        }
    }
    else
    {
        total = scoreboard->GetKills(uid, -1);
    }
    return _toString<int>(total);
}

void MG01ScoreboardLayout::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* baseInfo)
{
    MG01PlayerInfo* info = static_cast<MG01PlayerInfo*>(baseInfo);
    IScoreboardLayout::BuildPlayerInfo(player, info);

    boost::shared_ptr<AbstractClientPlayer> localPlayer =
        Minecraft::GetInstance()->GetLocalPlayerByName(player->GetOnlineName());
    const bool localFlag = localPlayer != 0 && localPlayer->isAlive() && !localPlayer->isSpectator();

    const StatsUID uid = StatsUID::NETWORKPLAYER_SUID(player);
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();

    info->localPlayer = localFlag;
    info->roundsWon = scoreboard->GetRoundsWon(uid);

    if (m_type != EScoreboardType_Award)
    {
        info->kills = scoreboard->GetKills(uid, -1);
        info->lifeSpan = scoreboard->GetLifeSpan(uid, -1);
        info->damageDealt = scoreboard->GetDamageDealt(uid, -1);
        info->damageReceived = scoreboard->GetDamageReceived(uid, -1);
        info->survived = scoreboard->GetSurvived(uid, -1) ? 1 : 0;
    }
    else
    {
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        const int rounds = static_cast<int>(miniGame->GetRoundCount());
        int kills = 0;
        int lifeSpan = 0;
        int damageDealt = 0;
        int damageReceived = 0;
        for (int round = 0; round < rounds; ++round)
        {
            kills += scoreboard->GetKills(uid, round);
            lifeSpan = scoreboard->GetLifeSpan(uid, round);
            damageDealt = scoreboard->GetDamageDealt(uid, round);
            damageReceived += scoreboard->GetDamageReceived(uid, round);
        }
        info->kills = kills;
        info->lifeSpan = lifeSpan;
        info->damageDealt = damageDealt;
        info->damageReceived = damageReceived;
        info->survived = scoreboard->GetSurvived(uid, -1) ? 1 : 0;
    }

    MultiPlayerGameMode* gameMode =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLocalGameMode();
    info->teamInvite = static_cast<unsigned char>(gameMode->IsTeamInvite(player->GetOnlineName()));
    info->colour = static_cast<short>(gApp.GetPlayerColour(info->smallId));
}

PlayerInfo* MG01ScoreboardLayout::CreatePlayerInfo()
{
    return new MG01PlayerInfo();
}

void MG01ScoreboardLayout::AddPlayer(unsigned char smallId)
{
    PlayerInfo* info = new MG01PlayerInfo();
    BuildPlayerInfo(CGameNetworkManager::sInstance.GetPlayerBySmallId(smallId), info);
    m_players.push_back(info);
}

void MG01ScoreboardLayout::UpdatePlayer(int index, PlayerInfo*, PlayerInfo* newInfo, bool)
{
    PlayerInfo* info = m_players[index];
    info->smallId = newInfo->smallId;
    info->colour = newInfo->colour;
    info->voiceStatus = newInfo->voiceStatus;
    info->name = newInfo->name;
}

unsigned char MG01ScoreboardLayout::GetEndBarIcon(PlayerInfo* baseInfo)
{
    const MG01PlayerInfo* info = static_cast<const MG01PlayerInfo*>(baseInfo);
    if (info->teamInvite != 0)
    {
        return 2;
    }
    return info->localPlayer ^ 1;
}

bool MG01ScoreboardLayout::showLifeSpanProgress()
{
    return 1;
}
