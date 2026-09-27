#include "game/core/Scoreboard.hpp"

#include <boost/unordered_set.hpp>

#include "game/core/DamageSource.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionItem.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/Potions.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/GameMedal.hpp"
#include "game/minigame/MiniGameMedals.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/generation/PlayerTeam.hpp"

Scoreboard::Scoreboard()
    : boost::unordered_map<std::wstring, PlayerTeam*>(11),
      mTeamsByPlayer(11),
      mNextTeamIndex(0)
{
}

Scoreboard::~Scoreboard()
{
    removeAllPlayerTeams();
}

PlayerTeam* Scoreboard::getPlayerTeam(const std::wstring& name)
{
    iterator team = find(name);

    if (team != end())
    {
        return team->second;
    }

    return NULL;
}

PlayerTeam* Scoreboard::addPlayerTeam(const std::wstring& name)
{
    PlayerTeam* pTeam = getPlayerTeam(name);

    if (pTeam == NULL)
    {
        pTeam = new PlayerTeam(this, name, size());
        (*this)[name] = pTeam;
        onTeamAdded(pTeam);
    }

    return pTeam;
}

void Scoreboard::removePlayerTeam(PlayerTeam* pTeam)
{
    iterator team = find(pTeam->getName());

    if (team != end())
    {
        erase(team);

        typedef boost::unordered_set<PlayerUID, boost::hash<PlayerUID>, std::equal_to<PlayerUID> >
            PlayerSet;

        for (PlayerSet::const_iterator player = pTeam->getPlayers()->begin();
             player != pTeam->getPlayers()->end();
             ++player)
        {
            mTeamsByPlayer.erase(*player);
        }

        onTeamRemoved(pTeam);
        delete pTeam;
    }
}

void Scoreboard::removeAllPlayerTeams()
{
    while (size() != 0)
    {
        removePlayerTeam(begin()->second);
    }
}

PlayerTeam* Scoreboard::getPlayersTeam(PlayerUID uid)
{
    PlayerTeamByUIDMap::iterator team = mTeamsByPlayer.find(uid);

    if (team != mTeamsByPlayer.end())
    {
        return team->second;
    }

    return NULL;
}

bool Scoreboard::removePlayerFromTeam(PlayerUID uid)
{
    PlayerTeam* pTeam = getPlayersTeam(uid);

    if (pTeam != NULL)
    {
        removePlayerFromTeam(uid, pTeam);
        return true;
    }

    return false;
}

void Scoreboard::addPlayerToTeam(PlayerUID uid, PlayerTeam* pTeam)
{
    if (getPlayersTeam(uid) != NULL)
    {
        removePlayerFromTeam(uid);
    }

    mTeamsByPlayer[uid] = pTeam;
    pTeam->getPlayers()->insert(uid);
}

void Scoreboard::removePlayerFromTeam(PlayerUID uid, PlayerTeam* pTeam)
{
    if (getPlayersTeam(uid) == pTeam)
    {
        mTeamsByPlayer.erase(uid);
        pTeam->getPlayers()->erase(uid);
    }
}

std::vector<PlayerTeam*>* Scoreboard::getPlayerTeams()
{
    std::vector<PlayerTeam*>* pTeams = new std::vector<PlayerTeam*>();
    pTeams->reserve(size());

    for (iterator team = begin(); team != end(); ++team)
    {
        pTeams->push_back(team->second);
    }

    return pTeams;
}

PlayerTeam* Scoreboard::getTeamByIndex(int index) const
{
    for (const_iterator team = begin(); team != end(); ++team)
    {
        PlayerTeam* pTeam = team->second;

        if (pTeam->getTeamIndex() == index)
        {
            return pTeam;
        }
    }

    return NULL;
}

boost::shared_ptr<ServerPlayer> Scoreboard::getPlayer(PlayerUID uid)
{
    return gMinecraftServer->getPlayers()->getPlayer(uid);
}

void Scoreboard::onTeamAdded(PlayerTeam* pTeam)
{
}

void Scoreboard::onTeamChanged(PlayerTeam* pTeam)
{
}

void Scoreboard::onTeamRemoved(PlayerTeam* pTeam)
{
}

void Scoreboard::syncTeamChanges(PlayerTeam* pTeam)
{
}

Scoreboard::EDamageSource Scoreboard::GetDamageSource(const DamageSource* pSource)
{
    if (pSource->isExplosion())
        return DAMAGE_SOURCE_EXPLOSION;
    if (pSource->equals(DamageSource::IN_FIRE) || pSource->equals(DamageSource::ON_FIRE))
        return DAMAGE_SOURCE_FIRE;
    if (pSource->equals(DamageSource::LAVA))
        return DAMAGE_SOURCE_LAVA;
    if (pSource->equals(DamageSource::DROWN))
        return DAMAGE_SOURCE_DROWN;
    if (pSource->equals(DamageSource::STARVE))
        return DAMAGE_SOURCE_STARVE;
    if (pSource->equals(DamageSource::CACTUS))
        return DAMAGE_SOURCE_CACTUS;
    if (pSource->equals(DamageSource::FALL) || pSource->equals(DamageSource::OUT_OF_WORLD)
        || pSource->equals(DamageSource::CREATIVE_PLAYER))
        return DAMAGE_SOURCE_FALL;
    if (pSource->equals(DamageSource::THORNS))
        return DAMAGE_SOURCE_THORNS;
    if (pSource == DamageSource::POISON)
        return DAMAGE_SOURCE_POISON;
    if (pSource == DamageSource::INSTANT_DAMAGE)
        return DAMAGE_SOURCE_HARM;
    if (pSource->equals(DamageSource::MAGIC))
        return DAMAGE_SOURCE_MAGIC;
    if (pSource->equals(DamageSource::ARROW))
        return DAMAGE_SOURCE_ARROW;
    if (pSource->equals(DamageSource::FLY_INTO_WALL))
        return DAMAGE_SOURCE_FLY_INTO_WALL;
    if (pSource->getEntity())
        return DAMAGE_SOURCE_MELEE;
    return DAMAGE_SOURCE_UNKNOWN;
}

Potion* Scoreboard::GetPotionByDamageSource(EDamageSource damageSource)
{
    if (damageSource == DAMAGE_SOURCE_POISON)
    {
        return Potions::POISON;
    }

    if (damageSource == DAMAGE_SOURCE_HARM)
    {
        return Potions::HARMING;
    }

    return NULL;
}

Scoreboard::Weapon::Weapon()
{
    mItemId = -1;
    mPotionId = -1;
}

Scoreboard::Weapon::Weapon(int itemId)
{
    mItemId = itemId;
    mPotionId = -1;
}

Scoreboard::Weapon::Weapon(int itemId, Potion* pPotion)
{
    mItemId = itemId;
    mPotionId = Potion::getId(pPotion);
}

uint32_t Scoreboard::Weapon::GetKey() const
{
    return (mPotionId << 16) | mItemId;
}

bool Scoreboard::Weapon::HasItem() const
{
    return mItemId != -1;
}

void Scoreboard::Weapon::GetItem() const
{
    Item::byId(mItemId);
}

not_null_ptr<ItemInstance> Scoreboard::Weapon::GetItemInstance() const
{
    Item* pItem = Item::byId(mItemId);

    if (pItem == NULL || dynamic_cast<PotionItem*>(pItem) == NULL || mPotionId < 0
        || Potion::byId(mPotionId) == NULL)
    {
        return not_null_ptr<ItemInstance>(new ItemInstance(pItem, 1));
    }

    not_null_ptr<ItemInstance> instance(new ItemInstance(pItem));
    return PotionUtils::setPotion(instance, Potion::byId(mPotionId));
}

Scoreboard::PlayerStats::PlayerStats()
    : PlayerUID(0)
{
    setRecordSUID(StatsUID::EMPTY_SUID());
    mName = L"";
    mMedal = NULL;
    mRoundStats = RoundStatsMap();
}

Scoreboard::PlayerStats::PlayerStats(const PlayerStats& other)
    : PlayerUID(other)
    , mName(other.mName)
    , mMedal(other.mMedal)
    , mFavouriteWeapon(other.mFavouriteWeapon)
    , mWeaknessWeapon(other.mWeaknessWeapon)
    , mRoundStats(other.mRoundStats)
{
}

StatsUID Scoreboard::PlayerStats::getRecordSUID()
{
    return StatsUID(*this);
}

void Scoreboard::PlayerStats::setRecordSUID(const StatsUID& suid)
{
    PlayerUID::operator=(suid);
}

std::wstring Scoreboard::PlayerStats::getRecord(eStringArgIndex index, int round, int life)
{
    _RoundStats& roundStats = mRoundStats[round];

    if (index == eStringArg_PlayerName)
    {
        return mName;
    }

    if (index == eStringArg_KillerName)
    {
        if (!roundStats.mPerLife.empty() && (unsigned int)life < roundStats.mPerLife.size())
        {
            return roundStats.mPerLife[life].mKillerName;
        }

        return L"";
    }

    return L"";
}

bool Scoreboard::PlayerStats::getRecord(eBoolArgIndex index, int round)
{
    _RoundStats& roundStats = mRoundStats[round];

    if (index == eBoolArg_Active)
    {
        return roundStats.mActive;
    }

    if (index == eBoolArg_Finished)
    {
        return roundStats.mFinished;
    }

    if (index == eBoolArg_Winner)
    {
        return roundStats.mWinner;
    }

    return false;
}

float Scoreboard::PlayerStats::getRecord(eFloatArgIndex index, int round)
{
    _RoundStats& roundStats = mRoundStats[round];

    if (index == eFloatArg_DistanceTravelled)
    {
        return (float)roundStats.mDistanceTravelled;
    }

    if (index == eFloatArg_LongestShot)
    {
        return (float)roundStats.mLongestShot;
    }

    if (index < eFloatArg_BestTime)
    {
        return (float)roundStats.mFallDistance;
    }

    if (index == eFloatArg_BestTime)
    {
        return (float)roundStats.mBestTime;
    }

    return 0.0f;
}

int Scoreboard::PlayerStats::getRecord(eIntArgIndex index, int round)
{
    _RoundStats& roundStats = mRoundStats[round];

    switch (index)
    {
    case eIntArg_MedalId:
        if (mMedal != NULL)
        {
            return mMedal->GetId();
        }
        break;

    case eIntArg_FavouriteWeaponItem:
        return mFavouriteWeapon.mItemId;

    case eIntArg_FavouriteWeaponPotion:
        return mFavouriteWeapon.mPotionId;

    case eIntArg_Score:
        return roundStats.mScore;

    case eIntArg_Placement:
        return roundStats.mPlacement;

    case eIntArg_KillerWeaponItem:
        return roundStats.mKillerWeapon.mItemId;

    case eIntArg_KillerWeaponPotion:
        return roundStats.mKillerWeapon.mPotionId;

    case eIntArg_Kills:
        return roundStats.mKills;

    case eIntArg_Deaths:
        return roundStats.mDeaths;

    case eIntArg_Lives:
        return roundStats.mLives;

    case eIntArg_DamageDealt:
        return roundStats.mDamageDealt;

    case eIntArg_DamageTaken:
        return roundStats.mDamageTaken;

    case eIntArg_ArrowsFired:
        return roundStats.mArrowsFired;

    case eIntArg_ArrowsHit:
        return roundStats.mArrowsHit;

    case eIntArg_BlocksPlaced:
        return roundStats.mBlocksPlaced;

    case eIntArg_MobKills:
        return roundStats.mMobKills;

    case eIntArg_ItemsCollected:
        return roundStats.mItemsCollected;

    case eIntArg_BlocksMined:
        return roundStats.mBlocksMined;

    case eIntArg_TimePlayed:
        return roundStats.mTimePlayed;

    case eIntArg_TotalTime:
        return roundStats.mTotalTime;

    case eIntArg_LastCheckpointId:
        return roundStats.mLastCheckpointId;

    case eIntArg_LapsCompleted:
        return roundStats.mLapsCompleted;

    case eIntArg_CheckpointTime:
        if (roundStats.mCheckpointTimes.find(roundStats.mLastCheckpointId)
            != roundStats.mCheckpointTimes.end())
        {
            return roundStats.mCheckpointTimes.at(roundStats.mLastCheckpointId);
        }
        break;

    case eIntArg_LastCheckpointSequenceId:
        return roundStats.mLastCheckpointSequenceId;

    case eIntArg_EndZoneEntered:
        return roundStats.mEndZoneEntered;

    case eIntArg_CheckpointsPassed:
        return roundStats.mCheckpointsPassed;

    case eIntArg_KillsMelee:
        return roundStats.mKillsMelee;

    case eIntArg_KillsRanged:
        return roundStats.mKillsRanged;

    case eIntArg_KillsExplosive:
        return roundStats.mKillsExplosive;

    case eIntArg_KillsOther:
        return roundStats.mKillsOther;

    case eIntArg_DeathsMelee:
        return roundStats.mDeathsMelee;

    case eIntArg_DeathsRanged:
        return roundStats.mDeathsRanged;

    case eIntArg_DeathsExplosive:
        return roundStats.mDeathsExplosive;

    case eIntArg_DeathsOther:
        return roundStats.mDeathsOther;

    case eIntArg_WeaknessWeaponItem:
        return mWeaknessWeapon.mItemId;

    case eIntArg_WeaknessWeaponPotion:
        return mWeaknessWeapon.mPotionId;

    default:
        return 0;
    }

    return -1;
}

long long Scoreboard::PlayerStats::getRecord(eInt64ArgIndex index, int round)
{
    return 0;
}

void Scoreboard::PlayerStats::setRecord(eStringArgIndex index, int round, int life,
                                        std::wstring value)
{
    _RoundStats& roundStats = mRoundStats[round];

    if (index == eStringArg_PlayerName)
    {
        mName = value;
    }
    else if (index == eStringArg_KillerName)
    {
        roundStats.mPerLife[life].mKillerName = value;
    }
}

void Scoreboard::PlayerStats::setRecord(eBoolArgIndex index, int round, bool value)
{
    _RoundStats& roundStats = mRoundStats[round];

    if (index == eBoolArg_Active)
    {
        roundStats.mActive = value;
    }
    else if (index == eBoolArg_Finished)
    {
        roundStats.mFinished = value;
    }
    else if (index == eBoolArg_Winner)
    {
        roundStats.mWinner = value;
    }
}

void Scoreboard::PlayerStats::setRecord(eFloatArgIndex index, int round, float value)
{
    _RoundStats& roundStats = mRoundStats[round];

    if (index == eFloatArg_DistanceTravelled)
    {
        roundStats.mDistanceTravelled = value;
    }
    else if (index == eFloatArg_LongestShot)
    {
        roundStats.mLongestShot = value;
    }
    else if (index < eFloatArg_BestTime)
    {
        roundStats.mFallDistance = value;
    }
    else if (index == eFloatArg_BestTime)
    {
        roundStats.mBestTime = value;
    }
}

void Scoreboard::PlayerStats::setRecord(eIntArgIndex index, int round, int value)
{
    _RoundStats& roundStats = mRoundStats[round];

    switch (index)
    {
    case eIntArg_MedalId:
        mMedal = MiniGameMedals::getMedalById(value);
        break;

    case eIntArg_FavouriteWeaponItem:
        mFavouriteWeapon.mItemId = value;
        break;

    case eIntArg_FavouriteWeaponPotion:
        mFavouriteWeapon.mPotionId = value;
        break;

    case eIntArg_Score:
        roundStats.mScore = value;
        break;

    case eIntArg_Placement:
        roundStats.mPlacement = value;
        break;

    case eIntArg_KillerWeaponItem:
        roundStats.mKillerWeapon.mItemId = value;
        break;

    case eIntArg_KillerWeaponPotion:
        roundStats.mKillerWeapon.mPotionId = value;
        break;

    case eIntArg_Kills:
        roundStats.mKills = value;
        break;

    case eIntArg_Deaths:
        roundStats.mDeaths = value;
        break;

    case eIntArg_Lives:
        roundStats.mLives = value;
        break;

    case eIntArg_DamageDealt:
        roundStats.mDamageDealt = value;
        break;

    case eIntArg_DamageTaken:
        roundStats.mDamageTaken = value;
        break;

    case eIntArg_ArrowsFired:
        roundStats.mArrowsFired = value;
        break;

    case eIntArg_ArrowsHit:
        roundStats.mArrowsHit = value;
        break;

    case eIntArg_BlocksPlaced:
        roundStats.mBlocksPlaced = value;
        break;

    case eIntArg_MobKills:
        roundStats.mMobKills = value;
        break;

    case eIntArg_ItemsCollected:
        roundStats.mItemsCollected = value;
        break;

    case eIntArg_BlocksMined:
        roundStats.mBlocksMined = value;
        break;

    case eIntArg_TimePlayed:
        roundStats.mTimePlayed = value;
        break;

    case eIntArg_TotalTime:
        roundStats.mTotalTime = value;
        break;

    case eIntArg_LastCheckpointId:
        roundStats.mLastCheckpointId = value;
        break;

    case eIntArg_LapsCompleted:
        roundStats.mLapsCompleted = value;
        break;

    case eIntArg_CheckpointTime:
        if (value > 0)
        {
            roundStats.mCheckpointTimes[roundStats.mLastCheckpointId] = value;
        }
        break;

    case eIntArg_LastCheckpointSequenceId:
        roundStats.mLastCheckpointSequenceId = value;
        break;

    case eIntArg_EndZoneEntered:
        roundStats.mEndZoneEntered = value;
        break;

    case eIntArg_CheckpointsPassed:
        roundStats.mCheckpointsPassed = value;
        break;

    case eIntArg_KillsMelee:
        roundStats.mKillsMelee = (unsigned short)value;
        break;

    case eIntArg_KillsRanged:
        roundStats.mKillsRanged = (unsigned short)value;
        break;

    case eIntArg_KillsExplosive:
        roundStats.mKillsExplosive = (unsigned short)value;
        break;

    case eIntArg_KillsOther:
        roundStats.mKillsOther = (unsigned short)value;
        break;

    case eIntArg_DeathsMelee:
        roundStats.mDeathsMelee = (unsigned short)value;
        break;

    case eIntArg_DeathsRanged:
        roundStats.mDeathsRanged = (unsigned short)value;
        break;

    case eIntArg_DeathsExplosive:
        roundStats.mDeathsExplosive = (unsigned short)value;
        break;

    case eIntArg_DeathsOther:
        roundStats.mDeathsOther = (unsigned short)value;
        break;

    case eIntArg_WeaknessWeaponItem:
        mWeaknessWeapon.mItemId = value;
        break;

    case eIntArg_WeaknessWeaponPotion:
        mWeaknessWeapon.mPotionId = value;
        break;

    default:
        break;
    }
}

void Scoreboard::PlayerStats::setRecord(eInt64ArgIndex, int, long long)
{
}

bool Scoreboard::PlayerStats::isRecordSame(PlayerStats* pOther, int record, int round, int life)
{
    if (record == eRecord_SUID)
    {
        return getRecordSUID() == pOther->getRecordSUID();
    }

    int index = record - eRecord_StringFirst;

    if (index < eStringArg_Count)
    {
        return getRecord((eStringArgIndex)index, round, life)
                   .compare(pOther->getRecord((eStringArgIndex)index, round, life))
               == 0;
    }

    index = record - eRecord_BoolFirst;

    if (index < eBoolArg_Count)
    {
        return getRecord((eBoolArgIndex)index, round)
               == pOther->getRecord((eBoolArgIndex)index, round);
    }

    index = record - eRecord_FloatFirst;

    if (index < eFloatArg_Count)
    {
        return getRecord((eFloatArgIndex)index, round)
               == pOther->getRecord((eFloatArgIndex)index, round);
    }

    index = record - eRecord_IntFirst;

    if (index < eIntArg_Count)
    {
        return getRecord((eIntArgIndex)index, round)
               == pOther->getRecord((eIntArgIndex)index, round);
    }

    index = record - eRecord_Int64First;

    if (index < eInt64Arg_Count)
    {
        return pOther->getRecord((eInt64ArgIndex)index, round)
               == getRecord((eInt64ArgIndex)index, round);
    }

    return false;
}

unsigned long long Scoreboard::PlayerStats::getChangedBitfield(PlayerStats* pOther, int round,
                                                               int life)
{
    unsigned long long changed = 0;

    for (int record = 0; record < eRecord_Count; record++)
    {
        if (!isRecordSame(pOther, record, round, life))
        {
            changed |= (unsigned long long)1 << record;
        }
    }

    return changed;
}

void Scoreboard::PlayerStats::updateFromOtherStats(PlayerStats* pOther, int round, int life,
                                                   unsigned long long changed)
{
    for (int record = 0; record < eRecord_Count; record++)
    {
        if (((changed >> record) & 1) == 0)
        {
            continue;
        }

        if (record == eRecord_SUID)
        {
            setRecordSUID(pOther->getRecordSUID());
            continue;
        }

        _RoundStats& roundStats = mRoundStats[round];

        if (roundStats.mPerLife.empty() || roundStats.mPerLife.size() <= (unsigned int)life)
        {
            int missing = (int)mRoundStats[round].mPerLife.size() - life + 1;

            for (int i = 0; i < missing; i++)
            {
                mRoundStats[round].mPerLife.push_back(_PerLife());
            }
        }

        int index = record - eRecord_StringFirst;

        if (index < eStringArg_Count)
        {
            setRecord((eStringArgIndex)index, round, life,
                      pOther->getRecord((eStringArgIndex)index, round, life));
            continue;
        }

        index = record - eRecord_BoolFirst;

        if (index < eBoolArg_Count)
        {
            setRecord((eBoolArgIndex)index, round, pOther->getRecord((eBoolArgIndex)index, round));
            continue;
        }

        index = record - eRecord_FloatFirst;

        if (index < eFloatArg_Count)
        {
            setRecord((eFloatArgIndex)index, round,
                      pOther->getRecord((eFloatArgIndex)index, round));
            continue;
        }

        index = record - eRecord_IntFirst;

        if (index < eIntArg_Count)
        {
            setRecord((eIntArgIndex)index, round, pOther->getRecord((eIntArgIndex)index, round));
        }
    }
}

void Scoreboard::PlayerStats::readRecord(DataInputStream* pInput, int record, int round, int life)
{
    if (record == eRecord_SUID)
    {
        setRecordSUID(StatsUID(pInput->readPlayerUID()));
        return;
    }

    _RoundStats& roundStats = mRoundStats[round];

    if (roundStats.mPerLife.empty() || roundStats.mPerLife.size() <= (unsigned int)life)
    {
        int missing = life - (int)mRoundStats[round].mPerLife.size() + 1;

        for (int i = 0; i < missing; i++)
        {
            mRoundStats[round].mPerLife.push_back(_PerLife());
        }
    }

    int index = record - eRecord_StringFirst;

    if (index < eStringArg_Count)
    {
        setRecord((eStringArgIndex)index, round, life, pInput->readUTF());
        return;
    }

    index = record - eRecord_BoolFirst;

    if (index < eBoolArg_Count)
    {
        setRecord((eBoolArgIndex)index, round, pInput->readBoolean());
        return;
    }

    index = record - eRecord_FloatFirst;

    if (index < eFloatArg_Count)
    {
        setRecord((eFloatArgIndex)index, round, pInput->readFloat());
        return;
    }

    index = record - eRecord_IntFirst;

    if (index < eIntArg_Count)
    {
        setRecord((eIntArgIndex)index, round, pInput->readVarInt());
        return;
    }

    index = record - eRecord_Int64First;

    if (index < eInt64Arg_Count)
    {
        setRecord((eInt64ArgIndex)index, round, pInput->readLong());
    }
}

void Scoreboard::PlayerStats::writeRecord(DataOutputStream* pOutput, int record, int round,
                                          int life)
{
    if (record == eRecord_SUID)
    {
        pOutput->writePlayerUID(getRecordSUID().GetID());
        return;
    }

    int index = record - eRecord_StringFirst;

    if (index < eStringArg_Count)
    {
        pOutput->writeUTF(getRecord((eStringArgIndex)index, round, life));
        return;
    }

    index = record - eRecord_BoolFirst;

    if (index < eBoolArg_Count)
    {
        pOutput->writeBoolean(getRecord((eBoolArgIndex)index, round));
        return;
    }

    index = record - eRecord_FloatFirst;

    if (index < eFloatArg_Count)
    {
        pOutput->writeFloat(getRecord((eFloatArgIndex)index, round));
        return;
    }

    index = record - eRecord_IntFirst;

    if (index < eIntArg_Count)
    {
        pOutput->writeVarInt(getRecord((eIntArgIndex)index, round));
        return;
    }

    index = record - eRecord_Int64First;

    if (index < eInt64Arg_Count)
    {
        pOutput->writeLong(getRecord((eInt64ArgIndex)index, round));
    }
}

Scoreboard::PlayerStats::_RoundStats::_RoundStats()
{
    mQuit = false;
    mWinner = false;
    mActive = false;
    mPlacement = Minecraft::InMiniGame(eMiniGameId_Glide, true) ? -1 : 0;
    mDamageDealt = 0;
    mScore = 0;
    mEndZoneEntered = 0;
    mLongestShot = 0.0;
    mBestTime = 0.0;
    mDistanceTravelled = 0.0;
    mFallDistance = 0.0;
    mDamageTaken = 0;
    mItemsCollected = 0;
    mCheckpointsPassed = 0;
    mBlocksPlaced = 0;
    mLastCheckpointSequenceId = 0;
    mLastCheckpointId = 0;
    mArrowsHit = 0;
    mTimePlayed = 0;
    mMobKills = 0;
    mBlocksMined = 0;
    mTotalTime = 0;
    mDeaths = 0;
    mKills = 0;
    mArrowsFired = 0;
    mLapsCompleted = 0;
    mCheckpointTimes = boost::unordered_map<int, int>();
    mDeathsMelee = 0;
    mDeathsRanged = 0;
    mDeathsExplosive = 0;
    mDeathsOther = 0;
    mKillsMelee = 0;
    mKillsRanged = 0;
    mKillsExplosive = 0;
    mKillsOther = 0;
}

Scoreboard::PlayerStats::_RoundStats::_RoundStats(const _RoundStats& other)
    : mActive(other.mActive)
    , mFinished(other.mFinished)
    , mScore(other.mScore)
    , mPlacement(other.mPlacement)
    , mKills(other.mKills)
    , mDeaths(other.mDeaths)
    , mKillerWeapon(other.mKillerWeapon)
    , mLives(other.mLives)
    , mPerLife(other.mPerLife)
    , mDamageDealt(other.mDamageDealt)
    , mDamageTaken(other.mDamageTaken)
    , mTimeAlive(other.mTimeAlive)
    , mQuit(other.mQuit)
    , mWinner(other.mWinner)
    , mArrowsFired(other.mArrowsFired)
    , mArrowsHit(other.mArrowsHit)
    , mBlocksPlaced(other.mBlocksPlaced)
    , mLastCheckpointId(other.mLastCheckpointId)
    , mLastCheckpointSequenceId(other.mLastCheckpointSequenceId)
    , mLapsCompleted(other.mLapsCompleted)
    , mEndZoneEntered(other.mEndZoneEntered)
    , mDistanceTravelled(other.mDistanceTravelled)
    , mLongestShot(other.mLongestShot)
    , mFallDistance(other.mFallDistance)
    , mCheckpointTimes(other.mCheckpointTimes)
    , mMobKills(other.mMobKills)
    , mCheckpointsPassed(other.mCheckpointsPassed)
    , mItemsCollected(other.mItemsCollected)
    , mBlocksMined(other.mBlocksMined)
    , mTimePlayed(other.mTimePlayed)
    , mTotalTime(other.mTotalTime)
    , mBestTime(other.mBestTime)
    , mDeathsMelee(other.mDeathsMelee)
    , mDeathsRanged(other.mDeathsRanged)
    , mDeathsExplosive(other.mDeathsExplosive)
    , mDeathsOther(other.mDeathsOther)
    , mKillsMelee(other.mKillsMelee)
    , mKillsRanged(other.mKillsRanged)
    , mKillsExplosive(other.mKillsExplosive)
    , mKillsOther(other.mKillsOther)
    , mLastKillerName(other.mLastKillerName)
    , mLastVictimName(other.mLastVictimName)
{
}

Scoreboard::PlayerStats::_RoundStats& Scoreboard::PlayerStats::_RoundStats::operator=(
    const _RoundStats& other)
{
    mActive = other.mActive;
    mFinished = other.mFinished;
    mScore = other.mScore;
    mPlacement = other.mPlacement;
    mKills = other.mKills;
    mDeaths = other.mDeaths;
    mKillerWeapon = other.mKillerWeapon;
    mLives = other.mLives;
    mPerLife = other.mPerLife;
    mDamageDealt = other.mDamageDealt;
    mDamageTaken = other.mDamageTaken;
    mTimeAlive = other.mTimeAlive;
    mQuit = other.mQuit;
    mWinner = other.mWinner;
    mArrowsFired = other.mArrowsFired;
    mArrowsHit = other.mArrowsHit;
    mBlocksPlaced = other.mBlocksPlaced;
    mLastCheckpointId = other.mLastCheckpointId;
    mLastCheckpointSequenceId = other.mLastCheckpointSequenceId;
    mLapsCompleted = other.mLapsCompleted;
    mEndZoneEntered = other.mEndZoneEntered;
    mDistanceTravelled = other.mDistanceTravelled;
    mLongestShot = other.mLongestShot;
    mFallDistance = other.mFallDistance;
    mCheckpointTimes = other.mCheckpointTimes;
    mMobKills = other.mMobKills;
    mCheckpointsPassed = other.mCheckpointsPassed;
    mItemsCollected = other.mItemsCollected;
    mBlocksMined = other.mBlocksMined;
    mTimePlayed = other.mTimePlayed;
    mTotalTime = other.mTotalTime;
    mBestTime = other.mBestTime;
    mDeathsMelee = other.mDeathsMelee;
    mDeathsRanged = other.mDeathsRanged;
    mDeathsExplosive = other.mDeathsExplosive;
    mDeathsOther = other.mDeathsOther;
    mKillsMelee = other.mKillsMelee;
    mKillsRanged = other.mKillsRanged;
    mKillsExplosive = other.mKillsExplosive;
    mKillsOther = other.mKillsOther;
    mLastKillerName = other.mLastKillerName;
    mLastVictimName = other.mLastVictimName;
    return *this;
}
