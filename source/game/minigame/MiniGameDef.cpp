#include "game/minigame/MiniGameDef.hpp"

#include <algorithm>
#include <cwchar>
#include <deque>
#include <sstream>
#include "game/core/L10N.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/File.hpp"
#include "game/core/FileInputStream.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringFormat.hpp"
#include "game/core/StringUtils.hpp"
#include "game/core/WeighedTreasure.hpp"
#include "game/core/XmlConstruct.hpp"
#include "game/core/XmlObject.hpp"
#include "game/core/XmlParser.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemPotionFix.hpp"
#include "game/item/Item.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/network/ClientboundMGPlayerSettingsUpdatePacket.hpp"
#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"
#include "game/util/GameModeUtilities.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/Blocks.hpp"

bool equalsIgnoreCase(const std::wstring& first, const std::wstring& second);

class Block;

static const unsigned int sHostOptionCategoryStringIds[4] =
{
    0xfb298fe6u, 0xe1a168a0u, 0x92734c7eu, 0x710952f7u
};

static MiniGamePlayerSettings sPlayerSettings;
static OSFastMutex sPlayerSettingsMutex;
static MiniGameDef* sMiniGameDefs[eMiniGameId_Count];
static MiniGameDef* sDefaultMiniGameDef;
static unsigned int sCrcTable[256];

static Block* sBlackList[] = { NULL };
static Block* sWhiteList[] = { NULL };
static Block* sPlacementRestrictedList[18] = { NULL };

static bool sBlackListInitialised;
static bool sWhiteListInitialised;
static bool sPlacementRestrictedListInitialised;

static void setPlayerSettingsHostOptions(const GameModeOptions& setting)
{
    EnterCriticalSection(&sPlayerSettingsMutex);
    static_cast<GameModeOptions&>(sPlayerSettings) = setting;
    LeaveCriticalSection(&sPlayerSettingsMutex);
}

static std::pair<bool, int> parseNamedValue(const std::wstring& value,
                                             const wchar_t* const* names,
                                             const int* values,
                                             int count)
{
    for (int i = 0; i < count; ++i)
    {
        if (value == names[i])
        {
            return std::make_pair(true, values[i]);
        }
    }
    return std::make_pair(false, 0);
}

std::pair<bool, int> parseDifficulty(std::wstring value)
{
    static const wchar_t* const names[] = { L"Peaceful", L"Easy", L"Normal", L"Hard" };
    static const int values[] = { 0, 1, 2, 3 };
    return parseNamedValue(value, names, values, 4);
}

std::pair<bool, int> parseGameType(std::wstring value)
{
    static const wchar_t* const names[] =
        { L"NotSet", L"Survival", L"Creative", L"Adventure", L"Spectator", L"Lobby" };
    static const int values[] = { -1, 0, 1, 2, 3, 4 };
    return parseNamedValue(value, names, values, 6);
}

std::pair<bool, int> parseSpectateMode(std::wstring value)
{
    static const wchar_t* const names[] = { L"$SpectateMode", L"Invisible", L"Bat", L"Vex", L"Parrot" };
    static const int values[] = { -1, 0, 1, 2, 3 };
    return parseNamedValue(value, names, values, 5);
}

std::pair<bool, int> parseBuildMode(std::wstring value)
{
    static const wchar_t* const names[] = { L"NoRestrictions", L"WhiteList", L"BlackList" };
    static const int values[] = { 0, 1, 2 };
    return parseNamedValue(value, names, values, 3);
}

void getVersionNumber(const XmlObject& object, unsigned short* major, unsigned short* minor)
{
    const std::wstring version = object[L"version"].XmlConstruct::asString(L"1.0");
    const std::wstring::size_type separator = version.find(L'.');
    *major = _fromString<unsigned short>(version.substr(0, separator));
    *minor = _fromString<unsigned short>(version.substr(separator + 1));
}

Block** getBlackList()
{
    if (!sBlackListInitialised)
    {
        sBlackListInitialised = true;
    }
    return sBlackList;
}

Block** getWhiteList()
{
    if (!sWhiteListInitialised)
    {
        sWhiteListInitialised = true;
    }
    return sWhiteList;
}

Block** getPlacementRestrictedList()
{
    if (!sPlacementRestrictedListInitialised)
    {
        sPlacementRestrictedList[0] = Blocks::torch;
        sPlacementRestrictedList[1] = Blocks::redstone_torch;
        sPlacementRestrictedList[2] = Blocks::wooden_pressure_plate;
        sPlacementRestrictedList[3] = Blocks::acacia_pressure_plate;
        sPlacementRestrictedList[4] = Blocks::birch_pressure_plate;
        sPlacementRestrictedList[5] = Blocks::dark_oak_pressure_plate;
        sPlacementRestrictedList[6] = Blocks::jungle_pressure_plate;
        sPlacementRestrictedList[7] = Blocks::spruce_pressure_plate;
        sPlacementRestrictedList[8] = Blocks::stone_pressure_plate;
        sPlacementRestrictedList[9] = Blocks::heavy_weighted_pressure_plate;
        sPlacementRestrictedList[10] = Blocks::light_weighted_pressure_plate;
        sPlacementRestrictedList[11] = Blocks::rail;
        sPlacementRestrictedList[12] = Blocks::golden_rail;
        sPlacementRestrictedList[13] = Blocks::detector_rail;
        sPlacementRestrictedList[14] = Blocks::activator_rail;
        sPlacementRestrictedList[15] = Blocks::snow_layer;
        sPlacementRestrictedList[16] = Blocks::carpet;
        sPlacementRestrictedListInitialised = true;
    }
    return sPlacementRestrictedList;
}

template <typename T>
static void setGameSettingsHelper(int profile, unsigned int setting, unsigned char value,
                                  T* settings, unsigned int mask, unsigned int shift)
{
    const T shifted = static_cast<T>(static_cast<unsigned int>(value) << (shift & 0x3f));
    const T oldValue = *settings;
    if ((static_cast<unsigned int>(oldValue) ^ static_cast<unsigned int>(shifted)) & mask)
    {
        *settings = static_cast<T>((oldValue & static_cast<T>(~mask)) | shifted);
        if (setting > 3)
        {
            GameSettings::ActionGameSettings(profile, static_cast<eGameSetting>(setting));
        }
        GameSettings::SetSettingsChanged(profile);
    }
}

void setGameSettingsHelper__tm__3_Uc__Fi12eGameSettingUcPZ1ZZ1ZUi_v(
    int profile, unsigned int setting, unsigned char value, unsigned char* settings,
    unsigned int mask, unsigned int shift)
{
    setGameSettingsHelper(profile, setting, value, settings, mask, shift);
}

void setGameSettingsHelper__tm__3_Us__Fi12eGameSettingUcPZ1ZZ1ZUi_v(
    int profile, unsigned int setting, unsigned char value, unsigned short* settings,
    unsigned int mask, unsigned int shift)
{
    setGameSettingsHelper(profile, setting, value, settings, mask, shift);
}

void setGameSettingsHelper__tm__3_Ui__Fi12eGameSettingUcPZ1ZZ1ZUi_v(
    int profile, unsigned int setting, unsigned char value, unsigned int* settings,
    unsigned int mask, unsigned int shift)
{
    setGameSettingsHelper(profile, setting, value, settings, mask, shift);
}

MiniGameDef::BlockData::BlockData()
    : mPlaceMode(EBuildMode_Default), mMayPlace(), mDestroyMode(EBuildMode_Default), mMayDestroy()
{
}

MiniGameDef::ChestItemsInfo::ChestItemsInfo(const ChestItemsInfo& other)
    : mItem(other.mItem), mMinCount(other.mMinCount), mMaxCount(other.mMaxCount), mWeight(other.mWeight)
    , mMinPlayers(other.mMinPlayers), mMaxPlayers(other.mMaxPlayers), mChance(other.mChance), mKpScore(other.mKpScore)
{
}

MiniGameDef::ChestItemsInfo& MiniGameDef::ChestItemsInfo::operator=(const ChestItemsInfo& other)
{
    if (this != &other)
    {
        mItem = other.mItem;
        mMinCount = other.mMinCount;
        mMaxCount = other.mMaxCount;
        mWeight = other.mWeight;
        mMinPlayers = other.mMinPlayers;
        mMaxPlayers = other.mMaxPlayers;
        mChance = other.mChance;
        mKpScore = other.mKpScore;
    }
    return *this;
}

MiniGameDef::ChestRefill::ChestRefill()
    : mEnabled(false), mCentralChance(0), mOuterEnabled(false), mOuterChance(0), mHvEnabled(false), mHvChance(0)
    , mNumberEnabled(false), mNumber(0), mMinDistanceEnabled(false), mMinDistance(0), mMaxDistanceEnabled(false), mMaxDistance(0)
    , mCentralLootGroup(), mOuterLootGroup(), mHighValueLootGroup()
{
}

MiniGameDef::FoodBalance::FoodBalance()
    : mMaxFoodEnabled(false), mMaxFood(0), mMaxSaturationEnabled(false), mMaxSaturation(0.0f)
    , mStartSaturationEnabled(false), mStartSaturation(0.0f), mSaturationFloorEnabled(false), mSaturationFloor(0.0f)
    , mExhaustionDropEnabled(false), mExhaustionDrop(0.0f)
    , mHealthTickEnabled(false), mHealthTickCount(0), mHealthTickSaturatedEnabled(false), mHealthTickCountSaturated(0)
    , mHealLevelEnabled(false), mHealLevel(0), mStarveLevelEnabled(false), mStarveLevel(0)
    , mSprintLevelEnabled(false), mSprintLevel(0), mExhaustionHealEnabled(false), mExhaustionHeal(0.0f)
    , mExhaustionJumpEnabled(false), mExhaustionJump(0.0f), mExhaustionSprintJumpEnabled(false), mExhaustionSprintJump(0.0f)
    , mExhaustionMineEnabled(false), mExhaustionMine(0.0f), mExhaustionAttackEnabled(false), mExhaustionAttack(0.0f)
    , mExhaustionDamageEnabled(false), mExhaustionDamage(0.0f)
    , mExhaustionSneakEnabled(false), mExhaustionSneak(0.0f), mExhaustionWalkEnabled(false), mExhaustionWalk(0.0f)
    , mExhaustionSprintEnabled(false), mExhaustionSprint(0.0f), mExhaustionSwimEnabled(false), mExhaustionSwim(0.0f)
    , mExhaustionIdleEnabled(false), mExhaustionIdle(0.0f)
{
}

MiniGameDef::FoodBalance::FoodBalance(const FoodBalance& other)
    : mMaxFoodEnabled(other.mMaxFoodEnabled), mMaxFood(other.mMaxFood)
    , mMaxSaturationEnabled(other.mMaxSaturationEnabled), mMaxSaturation(other.mMaxSaturation)
    , mStartSaturationEnabled(other.mStartSaturationEnabled), mStartSaturation(other.mStartSaturation)
    , mSaturationFloorEnabled(other.mSaturationFloorEnabled), mSaturationFloor(other.mSaturationFloor)
    , mExhaustionDropEnabled(other.mExhaustionDropEnabled), mExhaustionDrop(other.mExhaustionDrop)
    , mHealthTickEnabled(other.mHealthTickEnabled), mHealthTickCount(other.mHealthTickCount)
    , mHealthTickSaturatedEnabled(other.mHealthTickSaturatedEnabled), mHealthTickCountSaturated(other.mHealthTickCountSaturated)
    , mHealLevelEnabled(other.mHealLevelEnabled), mHealLevel(other.mHealLevel)
    , mStarveLevelEnabled(other.mStarveLevelEnabled), mStarveLevel(other.mStarveLevel)
    , mSprintLevelEnabled(other.mSprintLevelEnabled), mSprintLevel(other.mSprintLevel)
    , mExhaustionHealEnabled(other.mExhaustionHealEnabled), mExhaustionHeal(other.mExhaustionHeal)
    , mExhaustionJumpEnabled(other.mExhaustionJumpEnabled), mExhaustionJump(other.mExhaustionJump)
    , mExhaustionSprintJumpEnabled(other.mExhaustionSprintJumpEnabled), mExhaustionSprintJump(other.mExhaustionSprintJump)
    , mExhaustionMineEnabled(other.mExhaustionMineEnabled), mExhaustionMine(other.mExhaustionMine)
    , mExhaustionAttackEnabled(other.mExhaustionAttackEnabled), mExhaustionAttack(other.mExhaustionAttack)
    , mExhaustionDamageEnabled(other.mExhaustionDamageEnabled), mExhaustionDamage(other.mExhaustionDamage)
    , mExhaustionSneakEnabled(other.mExhaustionSneakEnabled), mExhaustionSneak(other.mExhaustionSneak)
    , mExhaustionWalkEnabled(other.mExhaustionWalkEnabled), mExhaustionWalk(other.mExhaustionWalk)
    , mExhaustionSprintEnabled(other.mExhaustionSprintEnabled), mExhaustionSprint(other.mExhaustionSprint)
    , mExhaustionSwimEnabled(other.mExhaustionSwimEnabled), mExhaustionSwim(other.mExhaustionSwim)
    , mExhaustionIdleEnabled(other.mExhaustionIdleEnabled), mExhaustionIdle(other.mExhaustionIdle)
{
}

MiniGameDef::SoundSpec::SoundSpec() : mEnabled(false), mSound(0), mForLast(false), mVolume(0.0f) {}

MiniGameDef::SoundSpec::SoundSpec(const XmlObject& object) : mEnabled(false), mSound(0), mForLast(false), mVolume(0.0f)
{
    maybe<int> sound = object[L"forLast"].maybeInt();
    if (sound.isSet()) { mEnabled = true; mSound = sound.get(); }
    maybe<float> volume = object[L"volume"].maybeFloat();
    if (volume.isSet()) { mForLast = true; mVolume = volume.get(); }
}

unsigned int MiniGameDef::SoundSpec::ForLast() const { return mEnabled ? (unsigned int)mSound : 0x7fffffffU; }
double MiniGameDef::SoundSpec::Volume() const { return mForLast ? mVolume : 1.0; }

MiniGameDef::TimerSpec::TimerSpec()
    : mEnabled(false), mDuration(0), mAnnounce(false), mAnnounceEvery(0), mChangeAnnounce(false)
    , mChangeAnnounceAfter(0), mEndAnnounce(false), mEndAnnounceAfter(0), mFinalAnnounce(false), mFinalAnnounceAfter(0)
{
}

MiniGameDef::TimerSpec::TimerSpec(const XmlObject& object)
    : mEnabled(false), mDuration(0), mAnnounce(false), mAnnounceEvery(0), mChangeAnnounce(false)
    , mChangeAnnounceAfter(0), mEndAnnounce(false), mEndAnnounceAfter(0), mFinalAnnounce(false), mFinalAnnounceAfter(0)
{
    maybe<int> value = object[L"duration"].maybeInt();
    if (value.isSet()) { mEnabled = true; mDuration = value.get(); }
    value = object[L"announceEvery"].maybeInt();
    if (value.isSet()) { mAnnounce = true; mAnnounceEvery = value.get(); }
    value = object[L"changeAnnounceAfter"].maybeInt();
    if (value.isSet()) { mChangeAnnounce = true; mChangeAnnounceAfter = value.get(); }
    value = object[L"changeAnnounceWhenRemaining"].maybeInt();
    if (value.isSet()) { mEndAnnounce = true; mEndAnnounceAfter = value.get(); }
    value = object[L"changedAnnounce"].maybeInt();
    if (value.isSet()) { mFinalAnnounce = true; mFinalAnnounceAfter = value.get(); }
}

void MiniGameDef::TimerSpec::SetDurationS(int value) { mEnabled = true; mDuration = value; }
unsigned int MiniGameDef::TimerSpec::TotalDurationS() const { return mEnabled ? (unsigned int)mDuration : 0xffffffffU; }
int MiniGameDef::TimerSpec::TotalDurationMs() const
{
    int seconds = (int)TotalDurationS();
    return seconds < 0 ? -1 : seconds * 1000;
}
unsigned int MiniGameDef::TimerSpec::AnnounceEveryS(bool isFinal) const
{
    if (isFinal)
        return mFinalAnnounce ? (unsigned int)mFinalAnnounceAfter : 0xffffffffU;
    return mAnnounce ? (unsigned int)mAnnounceEvery : 0xffffffffU;
}
int MiniGameDef::TimerSpec::AnnounceEveryMs(bool isFinal) const
{
    int seconds = (int)AnnounceEveryS(isFinal);
    return seconds < 0 ? -1 : seconds * 1000;
}
int MiniGameDef::TimerSpec::ChangeAnnounceAfterS() const
{
    if (mChangeAnnounce)
        return mChangeAnnounceAfter;
    return mEndAnnounce ? (int)TotalDurationS() - mEndAnnounceAfter : -1;
}
int MiniGameDef::TimerSpec::ChangeAnnounceAfterMs() const
{
    int seconds = ChangeAnnounceAfterS();
    return seconds < 0 ? -1 : seconds * 1000;
}

MiniGameDef::LootSetDef::LootSetDef() : mAttributes(), mGroups() {}
MiniGameDef::LootSetDef::~LootSetDef() {}

const MiniGameDef::ModeVariationLootGroup* MiniGameDef::LootSetDef::getLootGroup(const std::wstring& name) const
{
    boost::unordered_map<std::wstring, ModeVariationLootGroup>::const_iterator it = mGroups.find(name);
    return it == mGroups.end() ? NULL : &it->second;
}

std::wstring MiniGameDef::LootSetDef::getAttribute(const std::wstring& group, std::wstring defaultValue) const
{
    boost::unordered_map<std::wstring, std::wstring>::const_iterator it = mAttributes.find(group);
    return it == mAttributes.end() ? defaultValue : it->second;
}

MiniGameDef::ModeVariationLootGroup::ModeVariationLootGroup()
    : std::vector<ChestItemsInfo>(), mKits(), mPlayerEquipment(), mChestKpScore(0), mDistributionMethod(0)
{
}

MiniGameDef::ModeVariationLootGroup::ModeVariationLootGroup(const ModeVariationLootGroup& other)
    : std::vector<ChestItemsInfo>(other), mKits(other.mKits), mPlayerEquipment(other.mPlayerEquipment)
    , mChestKpScore(other.mChestKpScore), mDistributionMethod(other.mDistributionMethod)
{
}

MiniGameDef::ModeVariationLootGroup::~ModeVariationLootGroup() {}

std::vector<not_null_ptr<ItemInstance> >* MiniGameDef::ModeVariationLootGroup::GetChestItems() const
{
    std::vector<not_null_ptr<ItemInstance> >* result = new std::vector<not_null_ptr<ItemInstance> >();
    Random random;
    for (const_iterator it = begin(); it != end(); ++it)
    {
        for (int i = 0; i < it->mWeight; ++i)
        {
            not_null_ptr<ItemInstance> item = it->mItem->copy();
            int count = it->mMinCount;
            if (it->mMaxCount > it->mMinCount) count += random.nextInt(it->mMaxCount - it->mMinCount);
            int maxStack = item->getMaxStackSize();
            if (count > maxStack) count = maxStack;
            item->setCount(count);
            result->push_back(item);
        }
    }
    return result;
}

const std::vector<MiniGameDef::ChestItemsInfo>& MiniGameDef::ModeVariationLootGroup::GetAllChestItems() const { return *this; }
const std::vector<MiniGameDef::ChestKitInfo>& MiniGameDef::ModeVariationLootGroup::GetAllKits() const { return mKits; }
unsigned int MiniGameDef::ModeVariationLootGroup::GetChestKPScore() const { return mChestKpScore; }

arrayWithLength<WeighedTreasure*> MiniGameDef::ModeVariationLootGroup::GetWeightedChestItems() const
{
    arrayWithLength<WeighedTreasure*> result(size(), true);
    for (unsigned int i = 0; i < size(); ++i)
        result.data[i] = new WeighedTreasure((*this)[i].mItem->copy(), (*this)[i].mMinCount, (*this)[i].mMaxCount, (*this)[i].mChance);
    return result;
}

const MiniGameDef::PlayerEquipmentInfo& MiniGameDef::ModeVariationLootGroup::GetPlayerEquipment() const { return mPlayerEquipment; }

MiniGameDef::MiniGameDef(const XmlObject& object)
    : mHostOptionVersion(0), mBaseDamage(), mNutrition(), mPotionDurations(), mOptionLists(), mProjectilePower(), mMatchmakingFilters()
    , mMatchmakingFilterIds(), mOptionDefaults(), mPlayerSettings(), mSaturationModifiers(), mLingeringDurations()
    , mItemOverrides(), mTimers(), mSounds(), mId((EMiniGameId)object[L"id"].asInt(0)), mStartConditions(0)
    , mEndConditions(0), mPostGameConditions(0), mShowdownConditions(0), mVersionMajor(0), mVersionMinor(0), mCrc(0)
    , mCanAutoStart(false), mLoadOnlyActiveAreaChunks(false), mDisableBuilding(false), mDifficultySet(false), mDifficulty(0)
    , mPlayerGameTypeSet(false), mPlayerGameType(0), mSpectateMode(-1), mSpectateEntity(0), mSpectateEntitySet(false)
    , mPlayerAbilitiesSet(false), mPlayerAbilities(), mShowInvulnerableEffect(false), mLockedInventory(false)
    , mShowdownAnnouncements(false), mAllowPvP(false), mTutorialsEnabled(false), mDamageIndicatorDelay(0.0f)
    , mShouldTickBlocks(true), mShouldTickBlocksSet(false), mCanLavaConvertWater(false), mCanLavaConvertWaterSet(false), mPlaceMode(EBuildMode_Default), mDestroyMode(EBuildMode_Default)
    , mUseMode(EBuildMode_Default), mUseBlockMode(EBuildMode_Default), mMayPlace(), mMayDestroy(), mMayUse(), mMayUseBlock()
    , mCanTntDestroyBlocks(false), mMaxPlayersForSmallMaps(0), mFoodBalances(), mChestRefill(), mBlockData(), mLootSets()
    , mTextureName(), mCanCraft(true), mAllowsUGC(true), mHostOptions()
{
    mVersionMajor = object[L"versionMajor"].asUInt(0);
    mVersionMinor = object[L"versionMinor"].asUInt(0);
    mCanAutoStart = object[L"canAutoStart"].asBool(false);
    mLoadOnlyActiveAreaChunks = object[L"loadOnlyActiveAreaChunks"].asBool(false);
    mShowInvulnerableEffect = object[L"showInvulnerableEffect"].asBool(false);
    mLockedInventory = object[L"lockedInventory"].asBool(false);
    mShowdownAnnouncements = object[L"showdownAnnouncements"].asBool(false);
    mAllowPvP = object[L"allowPvP"].asBool(false);
    mTutorialsEnabled = object[L"tutorialsEnabled"].asBool(false);
    mDamageIndicatorDelay = object[L"damageIndicatorDelay"].asFloat(0.0f);
    mChestRefill.mCentralLootGroup = object[L"centralLootGroup"].asString();
    mChestRefill.mOuterLootGroup = object[L"outerLootGroup"].asString();
    mChestRefill.mHighValueLootGroup = object[L"highValueLootGroup"].asString();
    mTextureName = object[L"texture"].asString();
    mCanCraft = object[L"canCraft"].asBool(false);
    mAllowsUGC = object[L"AllowsUGC"].asBool(false);
    const XmlObject* blockTicking = object.child(L"BlockTicking");
    if (blockTicking != NULL)
    {
        maybe<bool> overrideValue = (*blockTicking)[L"override"].maybeBool();
        if (overrideValue.isSet())
        {
            mShouldTickBlocksSet = true;
            mShouldTickBlocks = overrideValue.get();
        }
    }
    maybe<bool> lavaConversion = object[L"CanLavaConvertWater"].maybeBool();
    if (lavaConversion.isSet())
    {
        mCanLavaConvertWaterSet = true;
        mCanLavaConvertWater = lavaConversion.get();
    }
    parsePermissions(object);
    parseChestRefill(object);
    parseHostOptionSettings(object);
    parseMatchmakingFilter(object);
}

MiniGameDef::MiniGameDef(EMiniGameId id)
    : mHostOptionVersion(0), mBaseDamage(), mNutrition(), mPotionDurations(), mOptionLists(), mProjectilePower(), mMatchmakingFilters()
    , mMatchmakingFilterIds(), mOptionDefaults(), mPlayerSettings(), mSaturationModifiers(), mLingeringDurations()
    , mItemOverrides(), mTimers(), mSounds(), mId(id), mStartConditions(0), mEndConditions(0), mPostGameConditions(0)
    , mShowdownConditions(0), mVersionMajor(0), mVersionMinor(0), mCrc(0), mCanAutoStart(false)
    , mLoadOnlyActiveAreaChunks(false), mDisableBuilding(false), mDifficultySet(false), mDifficulty(0)
    , mPlayerGameTypeSet(false), mPlayerGameType(0), mSpectateMode(-1), mSpectateEntity(0), mSpectateEntitySet(false)
    , mPlayerAbilitiesSet(false), mPlayerAbilities(), mShowInvulnerableEffect(false), mLockedInventory(false)
    , mShowdownAnnouncements(false), mAllowPvP(false), mTutorialsEnabled(false), mDamageIndicatorDelay(0.0f)
    , mShouldTickBlocks(true), mShouldTickBlocksSet(false), mCanLavaConvertWater(false), mCanLavaConvertWaterSet(false), mPlaceMode(EBuildMode_Default), mDestroyMode(EBuildMode_Default)
    , mUseMode(EBuildMode_Default), mUseBlockMode(EBuildMode_Default), mMayPlace(), mMayDestroy(), mMayUse(), mMayUseBlock()
    , mCanTntDestroyBlocks(false), mMaxPlayersForSmallMaps(0), mFoodBalances(), mChestRefill(), mBlockData(), mLootSets()
    , mTextureName(), mCanCraft(true), mAllowsUGC(true), mHostOptions()
{
}

MiniGameDef::~MiniGameDef()
{
    cleanUp();
}

std::pair<bool, int> MiniGameDef::parseTimer(std::wstring name)
{
    static const wchar_t* names[] = { L"GameInitialise", L"RoundPreparing", L"RoundStartDelay", L"GracePeriod",
        L"RoundEndDelay", L"LobbyInterval", L"RoundLimit", L"Showdown", L"PostGame", L"TimeToReachEnd" };
    static const int values[] = { 0, 1, 2, 3, 4, 6, 7, 8, 9, 11 };
    for (unsigned int i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
        if (name == names[i]) return std::make_pair(true, values[i]);
    return std::make_pair(false, 0);
}

std::pair<bool, int> MiniGameDef::parseSound(std::wstring name)
{
    static const wchar_t* names[] = { L"OnShowdown", L"OnRoundStart", L"OnRoundPlay", L"OnRoundEnding", L"OnChestRefill",
        L"OnDeath", L"LobbyTimer", L"RoundStartTimer", L"GraceTimer", L"RoundLimitTimer", L"OnFalling" };
    for (unsigned int i = 0; i < sizeof(names) / sizeof(names[0]); ++i)
        if (name == names[i]) return std::make_pair(true, (int)i);
    return std::make_pair(false, 0);
}

std::pair<bool, int> MiniGameDef::parseDistributionMethod(std::wstring name)
{
    static const wchar_t* names[] = { L"Spread", L"KitKPRandom", L"KitKPGreedy", L"Fixed", L"WeightedRandom" };
    for (unsigned int i = 0; i < sizeof(names) / sizeof(names[0]); ++i)
        if (name == names[i]) return std::make_pair(true, (int)i);
    return std::make_pair(false, 0);
}

void MiniGameDef::parseFoodBalance(const XmlObject& object, FoodBalance* balance)
{
    maybe<int> intValue = object[L"maxFood"].maybeInt();
    if (intValue.isSet()) { balance->mMaxFoodEnabled = true; balance->mMaxFood = intValue.get(); }
    maybe<float> floatValue = object[L"maxSaturation"].maybeFloat();
    if (floatValue.isSet()) { balance->mMaxSaturationEnabled = true; balance->mMaxSaturation = floatValue.get(); }
    floatValue = object[L"startSaturation"].maybeFloat();
    if (floatValue.isSet()) { balance->mStartSaturationEnabled = true; balance->mStartSaturation = floatValue.get(); }
    floatValue = object[L"saturationFloor"].maybeFloat();
    if (floatValue.isSet()) { balance->mSaturationFloorEnabled = true; balance->mSaturationFloor = floatValue.get(); }
    floatValue = object[L"exhaustionDrop"].maybeFloat();
    if (floatValue.isSet()) { balance->mExhaustionDropEnabled = true; balance->mExhaustionDrop = floatValue.get(); }
    intValue = object[L"healthTickCount"].maybeInt();
    if (intValue.isSet()) { balance->mHealthTickEnabled = true; balance->mHealthTickCount = intValue.get(); }
    intValue = object[L"healthTickCountSaturated"].maybeInt();
    if (intValue.isSet()) { balance->mHealthTickSaturatedEnabled = true; balance->mHealthTickCountSaturated = intValue.get(); }
    intValue = object[L"healLevel"].maybeInt();
    if (intValue.isSet()) { balance->mHealLevelEnabled = true; balance->mHealLevel = intValue.get(); }
    intValue = object[L"starveLevel"].maybeInt();
    if (intValue.isSet()) { balance->mStarveLevelEnabled = true; balance->mStarveLevel = intValue.get(); }
    intValue = object[L"sprintLevel"].maybeInt();
    if (intValue.isSet()) { balance->mSprintLevelEnabled = true; balance->mSprintLevel = intValue.get(); }
    floatValue = object[L"exhaustionHeal"].maybeFloat();
    if (floatValue.isSet()) { balance->mExhaustionHealEnabled = true; balance->mExhaustionHeal = floatValue.get(); }
    floatValue = object[L"exhaustionJump"].maybeFloat();
    if (floatValue.isSet()) { balance->mExhaustionJumpEnabled = true; balance->mExhaustionJump = floatValue.get(); }
    floatValue = object[L"exhaustionSprintJump"].maybeFloat();
    if (floatValue.isSet()) { balance->mExhaustionSprintJumpEnabled = true; balance->mExhaustionSprintJump = floatValue.get(); }
    floatValue = object[L"exhaustionMine"].maybeFloat();
    if (floatValue.isSet()) { balance->mExhaustionMineEnabled = true; balance->mExhaustionMine = floatValue.get(); }
    floatValue = object[L"exhaustionAttack"].maybeFloat();
    if (floatValue.isSet()) { balance->mExhaustionAttackEnabled = true; balance->mExhaustionAttack = floatValue.get(); }
    floatValue = object[L"exhaustionDamage"].maybeFloat();
    if (floatValue.isSet()) { balance->mExhaustionDamageEnabled = true; balance->mExhaustionDamage = floatValue.get(); }
    floatValue = object[L"exhaustionSneak"].maybeFloat();
    if (floatValue.isSet()) { balance->mExhaustionSneakEnabled = true; balance->mExhaustionSneak = floatValue.get(); }
    floatValue = object[L"exhaustionWalk"].maybeFloat();
    if (floatValue.isSet()) { balance->mExhaustionWalkEnabled = true; balance->mExhaustionWalk = floatValue.get(); }
    floatValue = object[L"exhaustionSprint"].maybeFloat();
    if (floatValue.isSet()) { balance->mExhaustionSprintEnabled = true; balance->mExhaustionSprint = floatValue.get(); }
    floatValue = object[L"exhaustionSwim"].maybeFloat();
    if (floatValue.isSet()) { balance->mExhaustionSwimEnabled = true; balance->mExhaustionSwim = floatValue.get(); }
    floatValue = object[L"exhaustionIdle"].maybeFloat();
    if (floatValue.isSet()) { balance->mExhaustionIdleEnabled = true; balance->mExhaustionIdle = floatValue.get(); }
}

void MiniGameDef::parsePermissions(const XmlObject& object)
{
    if (equalsIgnoreCase(object.mName, L""))
        return;

    EBuildMode* mode;
    std::vector<int>* exceptions;

    if (equalsIgnoreCase(object.mName, L"DestroyPermissions"))
    {
        mode = &mDestroyMode;
        exceptions = &mMayDestroy;
    }
    else if (equalsIgnoreCase(object.mName, L"PlacePermissions"))
    {
        mode = &mPlaceMode;
        exceptions = &mMayPlace;
    }
    else if (equalsIgnoreCase(object.mName, L"UsePermissions"))
    {
        mode = &mUseMode;
        exceptions = &mMayUse;
    }
    else if (equalsIgnoreCase(object.mName, L"BlockUsePermissions"))
    {
        mode = &mUseBlockMode;
        exceptions = &mMayUseBlock;
    }
    else
    {
        return;
    }

    maybe<std::wstring> modeText = object[L"mode"].maybeString();
    int modeValue = 0;
    if (modeText.isSet())
    {
        std::pair<bool, int> parsed = parseBuildMode(modeText.get());
        modeValue = parsed.first ? parsed.second : 0;
    }
    *mode = (EBuildMode)modeValue;

    if (modeValue != 0)
    {
        const std::vector<XmlChild>& children = object.getChildrenRaw();
        for (std::vector<XmlChild>::const_iterator child = children.begin(); child != children.end(); ++child)
        {
            if (child->m_type == 1 && equalsIgnoreCase(child->xml()->mName, L"Exception"))
            {
                exceptions->push_back((*child->xml())[L"id"].asInt());
            }
        }
    }
}

void MiniGameDef::parseChestRefill(const XmlObject& object)
{
    maybe<int> value = object[L"centralRefillChance"].maybeInt();
    if (value.isSet()) { mChestRefill.mEnabled = true; mChestRefill.mCentralChance = value.get(); }
    value = object[L"outerRefillChance"].maybeInt();
    if (value.isSet()) { mChestRefill.mOuterEnabled = true; mChestRefill.mOuterChance = value.get(); }
    value = object[L"highValueRefillChance"].maybeInt();
    if (value.isSet()) { mChestRefill.mHvEnabled = true; mChestRefill.mHvChance = value.get(); }
    value = object[L"numberOfChestsToRefill"].maybeInt();
    if (value.isSet()) { mChestRefill.mNumberEnabled = true; mChestRefill.mNumber = value.get(); }
    value = object[L"minPlayerDistance"].maybeInt();
    if (value.isSet()) { mChestRefill.mMinDistanceEnabled = true; mChestRefill.mMinDistance = value.get(); }
    value = object[L"maxPlayerDistance"].maybeInt();
    if (value.isSet()) { mChestRefill.mMaxDistanceEnabled = true; mChestRefill.mMaxDistance = value.get(); }
}

std::deque<not_null_ptr<ItemInstance> > MiniGameDef::parseItemInstance(const XmlObject& object, int* weight)
{
    int id = object[L"id"].asInt(0);
    int count = object[L"count"].asInt(1);
    int aux = object[L"aux"].asInt(0);
    if (weight != NULL)
    {
        *weight = object[L"slot"].asInt(-1);
    }

    not_null_ptr<ItemInstance> item(new ItemInstance(Item::byId(id), count, aux));

    std::vector<MobEffectInstance*> effects;
    std::wstring lastEffectName;

    const std::vector<XmlChild>& children = object.getChildrenRaw();
    for (std::vector<XmlChild>::const_iterator child = children.begin(); child != children.end(); ++child)
    {
        if (child->m_type != 1)
            continue;

        if (equalsIgnoreCase(child->xml()->mName, L"Enchantment"))
        {
            bool overrideEnchantable = (*child->xml())[L"override"].asBool(false);
            if (item->isEnchantable() || overrideEnchantable)
            {
                int enchantId = (*child->xml())[L"id"].asInt();
                int enchantLevel = (*child->xml())[L"level"].asInt();
                Enchantment* enchantment = Enchantment::byId(enchantId);
                if (enchantment != NULL && overrideEnchantable)
                {
                    item->enchant(enchantment, enchantLevel);
                }
                else
                {
                    bool canApply = enchantment != NULL && enchantment->canEnchant(item);
                    if (canApply)
                    {
                        int maxLevel = enchantment->getMaxLevel();
                        if (maxLevel < enchantLevel) enchantLevel = maxLevel;
                        item->enchant(enchantment, enchantLevel);
                    }
                }
            }
        }
        else if (equalsIgnoreCase(child->xml()->mName, L"Dye"))
        {
            ArmorItem* armor = (item->getItem() != NULL) ? dynamic_cast<ArmorItem*>(item->getItem()) : NULL;
            int rgb = _fromHEXString<int>((*child->xml())[L"rgb"].maybeString().get());
            if (armor != NULL)
            {
                armor->setColor(item, rgb);
            }
        }
        else if (equalsIgnoreCase(child->xml()->mName, L"CustomMobEffect"))
        {
            int effectId = (*child->xml())[L"Id"].asInt(0);
            int amplifier = (*child->xml())[L"Amplifier"].asInt(1);
            int duration = (*child->xml())[L"Duration"].asInt(200);
            bool ambient = (*child->xml())[L"Ambient"].asBool(false);
            lastEffectName = (*child->xml())[L"Name"].XmlConstruct::asString(L"");
            MobEffect* effect = MobEffect::byId(effectId);
            if (amplifier < 0) amplifier = 0;
            effects.push_back(new MobEffectInstance(effect, duration, amplifier, ambient, true));
        }
    }

    ItemPotionFix::fix(item);
    if (!effects.empty())
    {
        PotionUtils::setPotionName(item, lastEffectName);
        PotionUtils::setCustomEffects(item, &effects);
        for (std::vector<MobEffectInstance*>::iterator it = effects.begin(); it != effects.end(); ++it)
        {
            delete *it;
        }
        effects.clear();
    }

    std::deque<not_null_ptr<ItemInstance> > result;
    if (!item->isStackable() && item->getCount() > 1)
    {
        int splitCount = item->getCount();
        for (int i = 0; i < splitCount; ++i)
        {
            not_null_ptr<ItemInstance> single = item->copy();
            single->setCount(1);
            result.push_back(single);
        }
    }
    else
    {
        result.push_back(item);
    }
    return result;
}

void MiniGameDef::parseKit(std::vector<ChestKitInfo>* kits, const XmlObject& object)
{
    ChestKitInfo kit;
    kit.mWeight = object[L"weight"].asInt(1);
    kit.mMinPlayers = object[L"minPlayers"].asInt(0);
    kit.mMaxPlayers = object[L"maxPlayers"].asInt(8);
    int ignoredWeight = 0;
    kit.mItems.push_back(parseItemInstance(object, &ignoredWeight).front());
    kits->push_back(kit);
}

void MiniGameDef::parseEquipment(PlayerEquipmentInfo& equipment, const XmlObject& object)
{
    for (int i = 0; i < 6; ++i)
    {
        const XmlObject* slot = object.child(EquipmentSlot::SLOTS[i]->getName());
        if (slot != NULL)
        {
            int ignoredWeight = 0;
            equipment.mSlots[i] = maybe<not_null_ptr<ItemInstance> >(parseItemInstance(*slot, &ignoredWeight).front());
        }
    }
}

void MiniGameDef::parseChestItem(std::vector<ChestItemsInfo>* items, const XmlObject& object)
{
    maybe<int> minCount = object[L"minCount"].maybeInt();
    maybe<int> maxCount = object[L"maxCount"].maybeInt();
    maybe<unsigned int> playersForMin = object[L"playersForMin"].maybeUInt();
    maybe<unsigned int> kpScore = object[L"kpScore"].maybeUInt();
    maybe<unsigned int> weight = object[L"weight"].maybeUInt();

    const std::vector<XmlChild>& children = object.getChildrenRaw();
    for (std::vector<XmlChild>::const_iterator child = children.begin(); child != children.end(); ++child)
    {
        if (child->m_type != 1)
            continue;

        std::deque<not_null_ptr<ItemInstance> > parsedItems = parseItemInstance(*child->xml(), NULL);
        while (!parsedItems.empty())
        {
            ChestItemsInfo info;
            info.mItem = parsedItems.front();
            info.mMinCount = minCount.isSet() ? minCount.get() : 1;
            info.mMaxCount = maxCount.isSet() ? maxCount.get() : 1;
            info.mWeight = weight.isSet() ? (int)weight.get() : 1;
            info.mMinPlayers = playersForMin.isSet() ? (int)playersForMin.get() : 0;
            info.mMaxPlayers = 1;
            info.mChance = 1;
            info.mKpScore = kpScore.isSet() ? (int)kpScore.get() : 1;
            items->push_back(info);
            parsedItems.pop_front();
        }
    }
}

void MiniGameDef::parseItemOverrideMap(int itemId, const XmlObject& object)
{
    mItemOverrides[itemId] = object[L"replacement"].asInt(itemId);
    mBaseDamage[itemId] = object[L"baseDamage"].asDouble(0.0);
    mNutrition[itemId] = object[L"nutrition"].asInt(0);
    mSaturationModifiers[itemId] = object[L"saturationModifier"].asFloat(0.0f);
    mPotionDurations[itemId] = object[L"potionDuration"].asInt(0);
    mLingeringDurations[itemId] = object[L"lingeringDuration"].asInt(0);
    mProjectilePower[itemId] = object[L"throwPower"].asDouble(0.0);
}

void MiniGameDef::parseMatchmakingFilter(const XmlObject& object)
{
    int filterId = object[L"filterId"].asInt(-1);
    if (filterId >= 0)
    {
        boost::unordered_map<int, boost::unordered_map<int, int> > filter;
        boost::unordered_map<int, int> values;
        values[object[L"value"].asInt(0)] = object[L"match"].asInt(0);
        filter[object[L"option"].asInt(0)] = values;
        mMatchmakingFilterIds.push_back(filterId);
        mMatchmakingFilters.push_back(filter);
    }
}

void MiniGameDef::parseHostOptionSettings(const XmlObject& object)
{
    for (int option = eCustomGameModeHostOption_NoSmallSkins; option < eCustomGameModeHostOption_Nothing; ++option)
    {
        std::wstringstream key;
        key << L"option" << option;
        maybe<unsigned int> value = object[key.str()].maybeUInt();
        if (value.isSet())
        {
            mOptionDefaults[option] = value.get();
            mHostOptions.Set((eCustomGameModeHostOption)option, value.get());
        }
    }
}

unsigned int MiniGameDef::GetOptionType(eCustomGameModeHostOption option)
{
    static const unsigned char types[] = { 0, 1, 2, 1 };
    if (option > eCustomGameModeHostOption_NoSmallSkins && option < eCustomGameModeHostOption_CentreSpawn)
        return types[option];
    if (option > eCustomGameModeHostOption_All && option < eCustomGameModeHostOption_TimeTrial)
        return 1;
    return 0;
}

unsigned int MiniGameDef::GetOptionsDefault(eCustomGameModeHostOption option) const
{
    if (option == eCustomGameModeHostOption_All)
    {
        GameModeOptions options;
        for (int i = 0; i < eCustomGameModeHostOption_Nothing; ++i)
            options.Set((eCustomGameModeHostOption)i, GetOptionsDefault((eCustomGameModeHostOption)i));
        return options.Get(eCustomGameModeHostOption_All);
    }
    boost::unordered_map<int, unsigned int>::const_iterator it = mOptionDefaults.find(option);
    return it == mOptionDefaults.end() ? 0 : it->second;
}

MiniGameDef* MiniGameDef::parseMiniGameDef(XmlObject* object)
{
    if (object == NULL) return NULL;
    return new MiniGameDef(*object);
}

void MiniGameDef::MakeCRCTable()
{
    for (unsigned int i = 0; i < 256; ++i)
    {
        unsigned int value = i;
        for (int bit = 0; bit < 8; ++bit)
            value = (value & 1) != 0 ? (value >> 1) ^ 0xedb88320U : value >> 1;
        sCrcTable[i] = value;
    }
}

unsigned int MiniGameDef::UpdateCRC(unsigned int crc, unsigned char* data, int length)
{
    for (int i = 0; i < length; ++i) crc = sCrcTable[(crc ^ data[i]) & 0xff] ^ (crc >> 8);
    return crc;
}

unsigned int MiniGameDef::CRC(unsigned char* data, int length)
{
    return UpdateCRC(0xffffffffU, data, length) ^ 0xffffffffU;
}

void MiniGameDef::setCRC(unsigned int crc) { mCrc = crc; }

MiniGameDef* MiniGameDef::parseMiniGameDef(arrayWithLength<unsigned char> data)
{
    MiniGameDef* result = parseMiniGameDef(XmlParser::readXml(data.data, data.length));
    result->setCRC(CRC(data.data, data.length));
    return result;
}

MiniGameDef* MiniGameDef::parseMiniGameDef(std::wstring fileName)
{
    fileName = std::wstring(L"Common/res/TitleUpdate/GameRules") + fileName;
    File file(fileName);
    FileInputStream stream(file);
    unsigned int size = stream.filesize();
    arrayWithLength<unsigned char> data(size, true);
    bool valid = file.ext() == L".xml" && file.exists();
    if (valid)
    {
        stream.read(data);
        return parseMiniGameDef(data);
    }
    return new MiniGameDef((EMiniGameId)0);
}

void MiniGameDef::SetPlayerSettings(const MiniGamePlayerSettings& settings) { sPlayerSettings = settings; }
void MiniGameDef::GetPlayerSettings(MiniGamePlayerSettings& settings) { settings = sPlayerSettings; }

MiniGameDef* MiniGameDef::GetCustomGameModeReferenceById(EMiniGameId id)
{
    return id >= eMiniGameId_Lobby && id < eMiniGameId_Count ? sMiniGameDefs[id] : sDefaultMiniGameDef;
}

unsigned int MiniGameDef::GetId() const { return mId; }

void MiniGameDef::cleanUp()
{
    for (boost::unordered_map<int, FoodBalance*>::iterator it = mFoodBalances.begin(); it != mFoodBalances.end(); ++it)
        delete it->second;
    mFoodBalances.clear();
}

void MiniGameDef::StaticCtor()
{
    MakeCRCTable();
    sDefaultMiniGameDef = new MiniGameDef(eMiniGameId_Any);
    sMiniGameDefs[eMiniGameId_Lobby] = parseMiniGameDef(L"lobby.xml");
    sMiniGameDefs[eMiniGameId_Battle] = parseMiniGameDef(L"MG01.xml");
    sMiniGameDefs[eMiniGameId_Tumble] = parseMiniGameDef(L"MG02.xml");
    sMiniGameDefs[eMiniGameId_Glide] = parseMiniGameDef(L"MG03.xml");
    sMiniGameDefs[eMiniGameId_Build] = NULL;
    InitializeCriticalSection(&sPlayerSettingsMutex);
}

MiniGameDef* MiniGameDef::GetCustomGameModeById(EMiniGameId id, bool) { return GetCustomGameModeReferenceById(id); }

unsigned int MiniGameDef::GetHostOption(eCustomGameModeHostOption option) const
{
    MiniGamePlayerSettings settings;
    GetPlayerSettings(settings);
    return settings.Get(option);
}
unsigned int MiniGameDef::GetRoundCount() const { return GetHostOption(eCustomGameModeHostOption_TimeTrial) == 0 ? mPlayerSettings.mRoundCount : 1; }
int MiniGameDef::GetFinalRoundIndex() const { return GetRoundCount() - 1; }
unsigned int MiniGameDef::GetGameVariant() const
{
    MiniGamePlayerSettings settings;
    GetPlayerSettings(settings);
    return settings.Get(eCustomGameModeHostOption_GameVariant);
}
bool MiniGameDef::HostOption_Bool(eCustomGameModeHostOption, unsigned int index) const { return index != 0; }
bool MiniGameDef::HostOption_Bool(eCustomGameModeHostOption option) const { return HostOption_Bool(option, GetHostOption(option)); }
bool MiniGameDef::ArePointsActive() const { return HostOption_Bool(eCustomGameModeHostOption_PointsActive); }
bool MiniGameDef::ArePowerupsActive() const { return HostOption_Bool(eCustomGameModeHostOption_PowerupsActive); }
bool MiniGameDef::HasFeatureSupportForInfiniteLives() const { return mId != eMiniGameId_Tumble; }

const MiniGameDef::LootSetDef& MiniGameDef::getCurrentLootSet() const
{
    unsigned int index = mPlayerSettings.mLootSetId;
    if (index < mLootSets.size()) return mLootSets[index];
    static LootSetDef emptyLootSet;
    return emptyLootSet;
}

bool MiniGameDef::HasLootGroup(const std::wstring& group) const { return getCurrentLootSet().getLootGroup(group) != NULL; }

unsigned int MiniGameDef::PopulateContainers(const std::vector<boost::shared_ptr<Container> >* containers, const std::wstring& group) const
{
    const ModeVariationLootGroup* lootGroup = getCurrentLootSet().getLootGroup(group);
    switch (lootGroup->mDistributionMethod)
    {
    case 0:
    {
        std::vector<not_null_ptr<ItemInstance> >* items = lootGroup->GetChestItems();
        unsigned int result = GameModeUtilities::DistributeItemsSpread(containers, items);
        delete items;
        return result;
    }
    case 1:
        return GameModeUtilities::DistributeItemsKitsAndKP(containers, &lootGroup->GetAllChestItems(),
            &lootGroup->GetAllKits(), lootGroup->GetChestKPScore(), false);
    case 2:
        return GameModeUtilities::DistributeItemsKitsAndKP(containers, &lootGroup->GetAllChestItems(),
            &lootGroup->GetAllKits(), lootGroup->GetChestKPScore(), true);
    case 3:
        return GameModeUtilities::DistributeItemsKitsFixed(containers, &lootGroup->GetAllKits());
    case 4:
        return GameModeUtilities::DistributeItemsWeightedRandom(containers, lootGroup->GetWeightedChestItems(),
            lootGroup->GetChestKPScore());
    }
    return 0;
}

unsigned int MiniGameDef::EquipPlayers(std::vector<boost::shared_ptr<ServerPlayer> >* players) const
{
    const ModeVariationLootGroup* group = getCurrentLootSet().getLootGroup(L"default");
    if (group == NULL || players == NULL) return 0;
    return std::min((unsigned int)players->size(), (unsigned int)group->GetAllKits().size());
}

bool MiniGameDef::IsCustomGame(EMiniGameId id, const MiniGamePlayerSettings& settings)
{
    unsigned int variant = settings.Get(eCustomGameModeHostOption_GameVariant);
    return id == eMiniGameId_Battle ? variant == 2 : id == eMiniGameId_Tumble && variant == 3;
}

bool MiniGameDef::IsCustomGame() const { return IsCustomGame(mId, mPlayerSettings); }

int MiniGameDef::GetOptionsCount(eCustomGameModeHostOption option) const
{
    if (option < eCustomGameModeHostOption_All)
    {
        boost::unordered_map<int, std::vector<int> >::const_iterator it = mOptionLists.find(option);
        return it == mOptionLists.end() ? 0 : it->second.size();
    }
    unsigned int index = option - eCustomGameModeHostOption_MapVariant;
    return index < mMatchmakingFilters.size() ? mMatchmakingFilters[index].size() : 0;
}

unsigned int MiniGameDef::GetGameVariantCount() const { return mMatchmakingFilters.empty() ? 0 : mMatchmakingFilters[0].size(); }
int MiniGameDef::GetLootSetCount() const { return mLootSets.size(); }
int MiniGameDef::GetLootSetId() const { return mPlayerSettings.mLootSetId; }
void MiniGameDef::SetLootSet(int id) const { const_cast<MiniGamePlayerSettings&>(mPlayerSettings).mLootSetId = id; }

std::wstring MiniGameDef::GetLootSetText(int id) const
{
    unsigned int mode = GetId();
    if (mode == 1)
    {
        if (id < 3)
        {
            if (id == 0) return L10N::GetString(0x9d428428);
            if (id == 1) return L10N::GetString(0xc344fe91);
            return L10N::GetString(0xe94778fa);
        }
        if (id < 4) return L10N::GetString(0xf49f363);
        if (id == 4) return L10N::GetString(0x354c6dcc);
        if (id == 5) return L10N::GetString(0x6482ae73);
        return std::wstring();
    }
    if (mode == 2)
    {
        if (id == 0) return L10N::GetString(0x6ea6266a);
        if (id == 1) return L10N::GetString(0x48a3ac01);
        if (id < 3) return L10N::GetString(0x304c4139);
        if (id == 3) return L10N::GetString(0x93162dc9);
        return std::wstring();
    }
    return std::wstring();
}

std::wstring MiniGameDef::GetLootSetText() const { return GetLootSetText(GetLootSetId()); }

std::wstring MiniGameDef::GetLootSetDescription(int id) const
{
    unsigned int mode = GetId();
    if (mode == 1)
    {
        if (id < 3)
        {
            if (id == 0) return L10N::GetString(0xe3d04b5e);
            if (id == 1) return L10N::GetString(0xfdf2d6af);
            return L10N::GetString(0x172a369c);
        }
        if (id < 4) return L10N::GetString(0x931ab4ad);
        if (id == 4) return L10N::GetString(0x32400c9a);
        if (id == 5) return L10N::GetString(0x597a317d);
        return std::wstring();
    }
    if (mode == 2)
    {
        if (id == 0) return L10N::GetString(0xdf7a97cc);
        if (id == 1) return L10N::GetString(0x5486471f);
        if (id < 3) return L10N::GetString(0xfcc2df67);
        if (id == 3) return L10N::GetString(0xbb83a837);
        return std::wstring();
    }
    return std::wstring();
}

MiniGamePlayerSettings MiniGameDef::GetDefaultPlayerSettings() const { return mPlayerSettings; }

boost::shared_ptr<ClientboundMGPlayerSettingsUpdatePacket> MiniGameDef::GetPlayerSettingsPacket() const
{
    return boost::shared_ptr<ClientboundMGPlayerSettingsUpdatePacket>(new ClientboundMGPlayerSettingsUpdatePacket(mPlayerSettings));
}

std::wstring MiniGameDef::getVersionString() const
{
    std::wstringstream version;
    version << mVersionMajor << L"." << mVersionMinor;
    return version.str();
}

bool MiniGameDef::IsCRCValid() const
{
    static const unsigned int validCrc[] = { 0xe34a6dd7U, 0x0940817cU, 0x62401305U, 0xf44e4784U };
    return mId >= eMiniGameId_Lobby && mId <= eMiniGameId_Glide && mCrc == validCrc[mId];
}

bool MiniGameDef::HasPermission(EBuildMode mode, const std::vector<int>& permissions, int value) const
{
    if (mode == (EBuildMode)0 || mode >= (EBuildMode)3) return true;
    bool found = std::find(permissions.begin(), permissions.end(), value) != permissions.end();
    return (mode == (EBuildMode)1) == found;
}

unsigned int MiniGameDef::MayPlace(int id) const { return !mDisableBuilding && HasPermission(mPlaceMode, mMayPlace, id); }
unsigned int MiniGameDef::MayDestroy(int id) const { return !mDisableBuilding && HasPermission(mDestroyMode, mMayDestroy, id); }
unsigned int MiniGameDef::MayUse(int id) const { return !mDisableBuilding && HasPermission(mUseMode, mMayUse, id); }
unsigned int MiniGameDef::MayUseBlock(int id) const { return !mDisableBuilding && HasPermission(mUseBlockMode, mMayUseBlock, id); }

bool MiniGameDef::CanAutoStart() const { return mCanAutoStart; }
bool MiniGameDef::ShowInvulnerableEffect() const { return mShowInvulnerableEffect; }
bool MiniGameDef::LockedInventory() const { return mLockedInventory; }
bool MiniGameDef::ShowdownAnnouncements() const { return mShowdownAnnouncements; }
bool MiniGameDef::AllowPvP() const { return mAllowPvP; }
bool MiniGameDef::GetTutorialsEnabled() const { return mTutorialsEnabled; }
bool MiniGameDef::GetShowNametags() const { return mPlayerSettings.mShowNametags; }
double MiniGameDef::GetDamageIndicatorDelay() const { return mDamageIndicatorDelay; }
bool MiniGameDef::LoadOnlyActiveAreaChunks() const { return mLoadOnlyActiveAreaChunks; }
unsigned int MiniGameDef::GetLivesPerRound() const { return mPlayerSettings.mLivesPerRound; }
unsigned int MiniGameDef::GetTeamCount() const { return mPlayerSettings.mTeamCount; }
bool MiniGameDef::IsTeamBasedSpawn() const { return mPlayerSettings.mTeamBasedSpawn; }
bool MiniGameDef::IsCheckpointBasedSpawn() const { return mPlayerSettings.mCheckpointBasedSpawn; }
bool MiniGameDef::HasSpectatorParticipation() const { return mId == eMiniGameId_Tumble && mPlayerSettings.mSpectatorParticipation; }
bool MiniGameDef::IsSmallInventoryEnabled() const { return mPlayerSettings.mSmallInventoryEnabled; }
unsigned int MiniGameDef::GetMaxPlayersForSmallMaps() const { return mMaxPlayersForSmallMaps; }
bool MiniGameDef::IsTakeEverythingEnabled() const { return mPlayerSettings.mTakeEverythingEnabled; }
bool MiniGameDef::IsChestRefillEnabled() const { return mPlayerSettings.mChestRefillEnabled; }
bool MiniGameDef::ForceSpectateModeFollow() const { return mId == eMiniGameId_Glide; }
bool MiniGameDef::SupressGameModeMessages() const { return true; }

static const MiniGameDef::FoodBalance gDefaultFoodBalance;

const MiniGameDef::FoodBalance* MiniGameDef::GetFoodBalance() const
{
    if (mFoodBalances.find(mPlayerSettings.mFoodBalance) != mFoodBalances.end())
    {
        return mFoodBalances.at(mPlayerSettings.mFoodBalance);
    }
    return &gDefaultFoodBalance;
}

unsigned int MiniGameDef::getMaxFood(int value) const { const FoodBalance* f = GetFoodBalance(); return f->mMaxFoodEnabled ? f->mMaxFood : value; }
double MiniGameDef::getMaxSaturation(float value) const { const FoodBalance* f = GetFoodBalance(); return f->mMaxSaturationEnabled ? f->mMaxSaturation : value; }
double MiniGameDef::getStartSaturation(float value) const { const FoodBalance* f = GetFoodBalance(); return f->mStartSaturationEnabled ? f->mStartSaturation : value; }
double MiniGameDef::getSaturationFloor(float value) const { const FoodBalance* f = GetFoodBalance(); return f->mSaturationFloorEnabled ? f->mSaturationFloor : value; }
double MiniGameDef::getExhaustionDrop(float value) const { const FoodBalance* f = GetFoodBalance(); return f->mExhaustionDropEnabled ? f->mExhaustionDrop : value; }
unsigned int MiniGameDef::getHealthTickCount(int value) const { const FoodBalance* f = GetFoodBalance(); return f->mHealthTickEnabled ? f->mHealthTickCount : value; }
unsigned int MiniGameDef::getHealthTickCountSaturated(int value) const { const FoodBalance* f = GetFoodBalance(); return f->mHealthTickSaturatedEnabled ? f->mHealthTickCountSaturated : value; }
unsigned int MiniGameDef::getHealLevel(int value) const { const FoodBalance* f = GetFoodBalance(); return f->mHealLevelEnabled ? f->mHealLevel : value; }
unsigned int MiniGameDef::getStarveLevel(int value) const { const FoodBalance* f = GetFoodBalance(); return f->mStarveLevelEnabled ? f->mStarveLevel : value; }
unsigned int MiniGameDef::getSprintLevel(int value) const { const FoodBalance* f = GetFoodBalance(); return f->mSprintLevelEnabled ? f->mSprintLevel : value; }
double MiniGameDef::getExhaustionHeal(float value) const { const FoodBalance* f = GetFoodBalance(); return f->mExhaustionHealEnabled ? f->mExhaustionHeal : value; }
double MiniGameDef::getExhaustionJump(float value) const { const FoodBalance* f = GetFoodBalance(); return f->mExhaustionJumpEnabled ? f->mExhaustionJump : value; }
double MiniGameDef::getExhaustionSprintJump(float value) const { const FoodBalance* f = GetFoodBalance(); return f->mExhaustionSprintJumpEnabled ? f->mExhaustionSprintJump : value; }
double MiniGameDef::getExhaustionMine(float value) const { const FoodBalance* f = GetFoodBalance(); return f->mExhaustionMineEnabled ? f->mExhaustionMine : value; }
double MiniGameDef::getExhaustionAttack(float value) const { const FoodBalance* f = GetFoodBalance(); return f->mExhaustionAttackEnabled ? f->mExhaustionAttack : value; }
double MiniGameDef::getExhaustionDamage(float value) const { const FoodBalance* f = GetFoodBalance(); return f->mExhaustionDamageEnabled ? f->mExhaustionDamage : value; }
double MiniGameDef::getExhaustionWalk(float value) const { const FoodBalance* f = GetFoodBalance(); return f->mExhaustionWalkEnabled ? f->mExhaustionWalk : value; }
double MiniGameDef::getExhaustionSprint(float value) const { const FoodBalance* f = GetFoodBalance(); return f->mExhaustionSprintEnabled ? f->mExhaustionSprint : value; }
double MiniGameDef::getExhaustionSneak(float value) const { const FoodBalance* f = GetFoodBalance(); return f->mExhaustionSneakEnabled ? f->mExhaustionSneak : value; }
double MiniGameDef::getExhaustionSwim(float value) const { const FoodBalance* f = GetFoodBalance(); return f->mExhaustionSwimEnabled ? f->mExhaustionSwim : value; }
double MiniGameDef::getExhaustionIdle(float value) const { const FoodBalance* f = GetFoodBalance(); return f->mExhaustionIdleEnabled ? f->mExhaustionIdle : value; }

unsigned int MiniGameDef::getCentralRefillChance() const { return mChestRefill.mEnabled ? mChestRefill.mCentralChance : 0; }
unsigned int MiniGameDef::getOuterRefillChance() const { return mChestRefill.mOuterEnabled ? mChestRefill.mOuterChance : 0; }
unsigned int MiniGameDef::getHVRefillChance() const { return mChestRefill.mHvEnabled ? mChestRefill.mHvChance : 0; }
unsigned int MiniGameDef::getNumberOfChestsToRefill() const { return mChestRefill.mNumberEnabled ? mChestRefill.mNumber : 0; }
unsigned int MiniGameDef::getMinPlayerDistanceToEffectChance() const { return mChestRefill.mMinDistanceEnabled ? mChestRefill.mMinDistance : 0; }
unsigned int MiniGameDef::getMaxPlayerDistanceToEffectChance() const { return mChestRefill.mMaxDistanceEnabled ? mChestRefill.mMaxDistance : 0; }
std::wstring MiniGameDef::getCentralLootGroup() const { return mChestRefill.mCentralLootGroup.empty() ? L"central" : mChestRefill.mCentralLootGroup; }
std::wstring MiniGameDef::getOuterLootGroup() const { return mChestRefill.mOuterLootGroup.empty() ? L"outer" : mChestRefill.mOuterLootGroup; }
std::wstring MiniGameDef::getHVLootGroup() const { return mChestRefill.mHighValueLootGroup.empty() ? L"highValue" : mChestRefill.mHighValueLootGroup; }

not_null_ptr<ItemInstance> MiniGameDef::getEquipmentForSlot(EquipmentSlot::EIndex slot) const
{
    const ModeVariationLootGroup* group = getCurrentLootSet().getLootGroup(L"default");
    if (group != NULL && slot >= EquipmentSlot::eIndex_MainHand && slot <= EquipmentSlot::eIndex_Head &&
        group->GetPlayerEquipment().mSlots[slot].isSet())
        return group->GetPlayerEquipment().mSlots[slot].get();
    return ItemInstance::EMPTY;
}

unsigned int MiniGameDef::GetStartConditions() const { return mStartConditions; }
unsigned int MiniGameDef::GetEndConditions() const { return mEndConditions; }
unsigned int MiniGameDef::GetPostGameConditions() const { return mPostGameConditions; }
unsigned int MiniGameDef::GetShowdownConditions() const { return mShowdownConditions; }

const std::vector<int>& MiniGameDef::GetOptionsList(eCustomGameModeHostOption option) const
{
    static const std::vector<int> empty;
    boost::unordered_map<int, std::vector<int> >::const_iterator it = mOptionLists.find(option);
    return it == mOptionLists.end() ? empty : it->second;
}

unsigned int MiniGameDef::HostOption_Category(eCustomGameModeHostOption option, unsigned int index) const
{
    const std::vector<int>& values = GetOptionsList(option);
    return index < values.size() ? values[index] : 0;
}

unsigned int MiniGameDef::HostOption_Category(eCustomGameModeHostOption option) const { return HostOption_Category(option, GetHostOption(option)); }
int MiniGameDef::GetSpectateMode() const { return mSpectateMode != -1 ? mSpectateMode : HostOption_Category(eCustomGameModeHostOption_SpectateMode); }
unsigned long long MiniGameDef::GetSpectateEntity() const
{
    unsigned int spectateMode = GetSpectateMode();
    if (spectateMode == 1)
        return 0x1D000101ULL;
    if (spectateMode == 2)
        return 0x2D6C0003ULL;
    if (spectateMode == 3)
        return 0x5D540048ULL;
    return 0x000000000C800000ULL;
}
maybe<int> MiniGameDef::getDifficulty() const { return mDifficultySet ? maybe<int>(mDifficulty) : maybe<int>(); }
maybe<int> MiniGameDef::getPlayerGameType() const { return mPlayerGameTypeSet ? maybe<int>(mPlayerGameType) : maybe<int>(); }
Abilities MiniGameDef::getPlayerAbilities() const { return mPlayerAbilitiesSet ? mPlayerAbilities : mPlayerSettings.mPlayerAbilities; }
MiniGameDef::TimerSpec MiniGameDef::getTimerSpec(int timer) const { return atMaybe(mTimers, timer, TimerSpec()); }
MiniGameDef::SoundSpec MiniGameDef::getSoundSpec(EGameModeSound sound) const { return atMaybe(mSounds, sound, SoundSpec()); }
double MiniGameDef::getBaseDamage(int itemId, double value) const { return atMaybe(mBaseDamage, itemId, value); }
int MiniGameDef::getNutrition(int itemId, int value) const { return atMaybe(mNutrition, itemId, value); }
float MiniGameDef::getSaturationModifier(int itemId, float value) const { return atMaybe(mSaturationModifiers, itemId, value); }
int MiniGameDef::getPotionDuration(int effectId, int value) const { return atMaybe(mPotionDurations, effectId, value); }
int MiniGameDef::getLingeringDuration(int effectId, int value) const { return atMaybe(mLingeringDurations, effectId, value); }
double MiniGameDef::getThrowPower(unsigned long long itemId, double value) const { return atMaybe(mProjectilePower, (int)itemId, value); }
double MiniGameDef::getThrowAngle(unsigned long long itemId, double value) const { return atMaybe(mBaseDamage, (int)itemId, value); }
bool MiniGameDef::shouldTickBlocks() const { return mShouldTickBlocksSet ? mShouldTickBlocks : true; }
bool MiniGameDef::shouldTickBlock(int blockId) const { return atMaybe(mBlockData.mMayPlace, blockId, mShouldTickBlocks); }
bool MiniGameDef::canLavaConvertWater() const { return mCanLavaConvertWaterSet ? mCanLavaConvertWater : false; }
bool MiniGameDef::canTntDestroyBlocks() const { return mPlayerSettings.mCanTntDestroyBlocks.isSet() ? mPlayerSettings.mCanTntDestroyBlocks.get() : false; }
unsigned char MiniGameDef::CurrentHostOptionVersion() const { return mHostOptionVersion; }

unsigned int MiniGameDef::HostOption_Time(eCustomGameModeHostOption option, unsigned int index) const
{
    const std::vector<int>& values = GetOptionsList(option);
    return index < values.size() ? values[index] : 0;
}

unsigned int MiniGameDef::HostOption_Time(eCustomGameModeHostOption option) const { return HostOption_Time(option, GetHostOption(option)); }

std::wstring MiniGameDef::GetValueText(eCustomGameModeHostOption option, int value) const
{
    unsigned int type = GetOptionType(option);

    if (type == 0)
    {
        if (option == 0)
        {
            value = (value == 0);
        }
        return L10N::GetString(value != 0 ? 0xc8e108fd : 0xf74dab3b);
    }

    if (type == 1)
    {
        if (option == 1)
        {
            unsigned int category = HostOption_Category(option, (unsigned int)value);
            return L10N::GetString((int)sHostOptionCategoryStringIds[category]);
        }

        if (option > 10)
        {
            if (option == 11)
            {
                if (value == 0) return L10N::GetString(0x68039067);
                if (value == 1) return L10N::GetString(0x47958fd3);
                if (value < 3) return L10N::GetString(0xa7e3639c);
                if (value != 3) return std::wstring();
                return L10N::GetString(0xa7369313);
            }
            if (option < 13)
            {
                unsigned int id = GetId();
                if (id != 0)
                {
                    if (id == 1)
                    {
                        if (value == 0) return L10N::GetString(0x189a3627);
                        if (value == 1) return L10N::GetString(0x9038dc1);
                        if (value < 3) return L10N::GetString(0x6829d25f);
                        if (value != 3) return std::wstring();
                        return L10N::GetString(0xa0f8061a);
                    }
                    if (id < 3)
                    {
                        if (value < 2)
                        {
                            if (value == 0) return L10N::GetString(0x189a3627);
                            if (value == 1) return L10N::GetString(0x6ea6266a);
                        }
                        if (value < 3) return L10N::GetString(0x48a3ac01);
                        if (value != 3)
                        {
                            if (value == 4) return L10N::GetString(0xa0f8061a);
                            return std::wstring();
                        }
                        return L10N::GetString(0x34f502c2);
                    }
                    if (id == 3)
                    {
                        if (value == 0) return L10N::GetString(0x189a3627);
                        if (value == 1) return L10N::GetString(0xc45287b7);
                        if (value == 2) return L10N::GetString(0x1c7d5232);
                        if (value != 4) return std::wstring();
                        return L10N::GetString(0xa0f8061a);
                    }
                }
                return std::wstring();
            }
            if (option == 13)
            {
                unsigned int id = GetId();
                if (id != 0)
                {
                    if (id == 1)
                    {
                        if (value == 0) return L10N::GetString(0x9038dc1);
                        if (value == 1) return L10N::GetString(0x6829d25f);
                        if (value == 2) return L10N::GetString(0xa0f8061a);
                        return std::wstring();
                    }
                    if (id < 3)
                    {
                        if (value == 0) return L10N::GetString(0x6ea6266a);
                        if (value == 1) return L10N::GetString(0x48a3ac01);
                        if (value < 3) return L10N::GetString(0x34f502c2);
                        if (value != 3) return std::wstring();
                        return L10N::GetString(0xa0f8061a);
                    }
                    if (id == 3)
                    {
                        if (value == 0) return L10N::GetString(0xc45287b7);
                        if (value != 1) return std::wstring();
                        return L10N::GetString(0x1c7d5232);
                    }
                }
                return std::wstring();
            }
        }
        return std::wstring();
    }

    if (type == 2)
    {
        if (option == 2)
        {
            if (value == 0) return L10N::GetString(0x47958fd3);
            if (value == 1) return L10N::GetString(0xa7e3639c);
            if (value == 2) return L10N::GetString(0xa7369313);
            return std::wstring();
        }

        int seconds = (int)HostOption_Time(option, (unsigned int)value);
        if (seconds == -2) return L10N::GetString(0xb10b55cf);
        if (seconds == -1) return L10N::GetString(0xd2691ec2);
        if (seconds > 0x3b)
        {
            std::wstring templateText = L10N::GetString(seconds == 0x3c ? 0xb80e547f : 0x82557e0c);
            std::wstring minutesText = _toString(seconds / 0x3c);
            return replaceAll(templateText, L"{MINUTES}", minutesText);
        }
        if (seconds >= 0)
        {
            std::wstring templateText = L10N::GetString(seconds == 1 ? 0xb58fee78 : 0x4ddf2da3);
            std::wstring secondsText = _toString(seconds);
            return replaceAll(templateText, L"{SECONDS}", secondsText);
        }
        return std::wstring();
    }

    return std::wstring();
}

std::wstring MiniGameDef::getComparisonImageForGameMode(EMiniGameId id)
{
    switch (id)
    {
    case eMiniGameId_Battle:
        return std::wstring(L"Graphics\\GameModeGraphics\\BattleModeScreen.png");
    case eMiniGameId_Tumble:
        return std::wstring(L"Graphics\\GameModeGraphics\\MG02Screen.png");
    case eMiniGameId_Glide:
        return std::wstring(L"Graphics\\GameModeGraphics\\MG03Screen.png");
    default:
        return std::wstring(L"Graphics\\MinecraftBrokenIcon.png");
    }
}

std::wstring MiniGameDef::GetVariantIconPath(EMiniGameId id, unsigned int variant)
{
    if (id == eMiniGameId_Tumble)
    {
        if (variant > 2)
            variant = 2;
        return formatwstr(L"Graphics\\GameModeGraphics\\MG02_M_%d", variant);
    }
    return std::wstring(L"default");
}

void MiniGameDef::SetHostOption(eCustomGameModeHostOption option, unsigned int value) const
{
    GameModeOptions options = static_cast<const GameModeOptions&>(sPlayerSettings);
    options.Set(option, value);
    setPlayerSettingsHostOptions(options);
}

int MiniGameDef::GetHostOptionIndexFrom_Time(eCustomGameModeHostOption option, int value) const
{
    const std::vector<int>& values = GetOptionsList(option);
    std::vector<int>::const_iterator it = std::find(values.begin(), values.end(), value);
    return it == values.end() ? -1 : it - values.begin();
}

int MiniGameDef::GetHostOptionIndexFrom_Category(eCustomGameModeHostOption option, int value) const
{
    return GetHostOptionIndexFrom_Time(option, value);
}

unsigned int MiniGameDef::GetHostOptionIndexFrom_Bool(eCustomGameModeHostOption, bool value) const { return value ? 1 : 0; }

unsigned int MiniGameDef::GetHostOptionIndexFromValue(eCustomGameModeHostOption option, int value) const
{
    switch (GetOptionType(option))
    {
    case 0: return GetHostOptionIndexFrom_Bool(option, value != 0);
    case 1: return GetHostOptionIndexFrom_Category(option, value);
    case 2: return GetHostOptionIndexFrom_Time(option, value);
    default: return 0;
    }
}

unsigned int MiniGameDef::GetOptionMin(eCustomGameModeHostOption option, bool includeRandom) const
{
    const std::vector<int>& values = GetOptionsList(option);
    if (values.empty()) return 0;
    unsigned int minimum = includeRandom ? 0 : 1;
    return minimum < values.size() ? minimum : 0;
}

int MiniGameDef::GetOptionMax(eCustomGameModeHostOption option) const
{
    int count = GetOptionsCount(option);
    return count == 0 ? 0 : count - 1;
}

unsigned int MiniGameDef::AllowSmallSkins() const { return !HostOption_Bool(eCustomGameModeHostOption_NoSmallSkins); }

std::wstring MiniGameDef::GetFoodBalanceIndexText(int index) const
{
    switch (index)
    {
    case 0: return L10N::GetString(0x1ea80257);
    case 1: return L10N::GetString(0xf8a587ee);
    case 2: return L10N::GetString(0xd2a30d85);
    case 3: return L10N::GetString(0xaca0931c);
    case 4: return L10N::GetString(0x869e18b3);
    case 5: return L10N::GetString(0x609b9e4a);
    case 6: return L10N::GetString(0x3a9923e1);
    case 7: return L10N::GetString(0x1496a978);
    default: return std::wstring();
    }
}

std::wstring MiniGameDef::GetFoodBalanceIndexDescription(int index) const
{
    switch (index)
    {
    case 0: return L10N::GetString(0x9a084979);
    case 1: return L10N::GetString(0x3ce24768);
    case 2: return L10N::GetString(0x175deebb);
    case 3: return L10N::GetString(0xa7ba696a);
    case 4: return L10N::GetString(0x4b98883d);
    case 5: return L10N::GetString(0xabd0762c);
    case 6: return L10N::GetString(0x6c2ba0ff);
    case 7: return L10N::GetString(0xfc881bae);
    default: return std::wstring();
    }
}

void MiniGameDef::SetDisabledEditActions(bool disabled) const { const_cast<MiniGameDef*>(this)->mDisableBuilding = disabled; }
bool MiniGameDef::EditActionsDisabled() const { return mDisableBuilding; }
unsigned int MiniGameDef::GetItemRespawnTime() const { return 30; }
unsigned int MiniGameDef::IsShowdownEnabled() const { return 1; }
bool MiniGameDef::CanCraft() const { return mCanCraft; }
bool MiniGameDef::HasMultipleRounds() const { return GetRoundCount() > 1; }

void MiniGameDef::addWeightedChestItem(std::vector<ChestItemsInfo>* items, not_null_ptr<ItemInstance> item,
                                       unsigned int weight, unsigned int minCount, unsigned int maxCount)
{
    ChestItemsInfo info;
    info.mItem = item;
    info.mWeight = weight;
    info.mMinCount = minCount;
    info.mMaxCount = maxCount;
    info.mMinPlayers = 0;
    info.mMaxPlayers = 8;
    info.mChance = weight;
    info.mKpScore = 0;
    items->push_back(info);
}

void MiniGameDef::addChestItem(std::vector<ChestItemsInfo>* items, not_null_ptr<ItemInstance> item, unsigned int weight,
                               unsigned int minCount, unsigned int maxCount, unsigned int slot, unsigned int chance)
{
    ChestItemsInfo info;
    info.mItem = item;
    info.mWeight = weight;
    info.mMinCount = minCount;
    info.mMaxCount = maxCount;
    info.mMinPlayers = slot;
    info.mMaxPlayers = 8;
    info.mChance = chance;
    info.mKpScore = 0;
    items->push_back(info);
}

unsigned int MiniGameDef::GetName() const
{
    static const unsigned int names[] = { 0, 0x42e0ac30U, 0xb4e81b6bU, 0x8ee5a102U };
    return mId > eMiniGameId_Lobby && mId <= eMiniGameId_Glide ? names[mId] : 0xffffffffU;
}

unsigned int MiniGameDef::GetHostOptionStringId(eCustomGameModeHostOption option)
{
    static const unsigned int names[] = { 0xa329d6e9U, 0x901c1fe8U, 0x908811f4U };
    if (option < eCustomGameModeHostOption_MaxPlayers) return names[option];
    if (option == eCustomGameModeHostOption_CentreSpawn || option == eCustomGameModeHostOption_IsTeams) return 0xef2a2f6aU;
    if (option == eCustomGameModeHostOption_GameVariant) return 0x5bd476fbU;
    return 0xffffffffU;
}

void MiniGameDef::AddLabelForOption(std::vector<HtmlString>& labels, eCustomGameModeHostOption option, unsigned int value) const
{
    std::wstring text = L10N::GetString(GetHostOptionStringId(option));
    text += L": ";
    text += GetValueText(option, value);
    labels.push_back(HtmlString(text, eMinecraftColour_HTMLColor_White, false, false, 0));
}

void MiniGameDef::GetGameSettingsDescription(std::vector<HtmlString>& labels, unsigned int flags) const
{
    for (int option = 0; option < eCustomGameModeHostOption_Nothing; ++option)
        if ((flags & (1U << option)) != 0) AddLabelForOption(labels, (eCustomGameModeHostOption)option, GetHostOption((eCustomGameModeHostOption)option));
}

void MiniGameDef::SetDefaultMatchmakingSearchData(bool joining, LaunchMoreOptionsMenuInitData* data) const
{
    if (data->mValue5C < 1 || data->mValue5C > 4) return;
    if (joining)
    {
        data->mValue60 = 0;
        data->mValue64 = 0;
    }
    else if (!mMatchmakingFilters.empty())
    {
        data->mValue60 = mMatchmakingFilters[0].empty() ? 0 : mMatchmakingFilters[0].begin()->first + 1;
        data->mValue64 = mMatchmakingFilters.size() < 2 || mMatchmakingFilters[1].empty() ? 0 : mMatchmakingFilters[1].begin()->first + 1;
    }
}

bool MiniGameDef::HasFeatureSeasonalMaps() const { return mId == eMiniGameId_Tumble; }
bool MiniGameDef::HasFeatureKeepInventory() const { return mId != eMiniGameId_Battle; }

void MiniGameDef::SetGameSettingsFromMatchmakingFilter(LaunchMoreOptionsMenuInitData* data) const
{
    const int selected[] = { data->mValue60, data->mValue64 };
    for (unsigned int set = 0; set < 2 && set < mMatchmakingFilters.size(); ++set)
    {
        if (selected[set] == 0) continue;
        boost::unordered_map<int, boost::unordered_map<int, int> >::const_iterator filter = mMatchmakingFilters[set].find(selected[set] - 1);
        if (filter == mMatchmakingFilters[set].end()) continue;
        for (boost::unordered_map<int, int>::const_iterator option = filter->second.begin(); option != filter->second.end(); ++option)
            data->mPlayerSettings.Set((eCustomGameModeHostOption)option->first, GetHostOptionIndexFromValue((eCustomGameModeHostOption)option->first, option->second));
    }
    if (data->mValue64 != 0) data->mPlayerSettings.Set(eCustomGameModeHostOption_GameVariant, data->mValue64 - 1);
    data->mPlayerSettings.Set(eCustomGameModeHostOption_EnableVoting,
        data->mValue5C != eMiniGameId_Tumble || data->mValue64 != 3);
    if (data->mFlag56 && data->mValue5C == eMiniGameId_Tumble)
        data->mPlayerSettings.Set(eCustomGameModeHostOption_NoSmallSkins, data->mFlag58);
    if (data->mFlag56 && data->mValue5C == eMiniGameId_Glide)
    {
        data->mPlayerSettings.Set(eCustomGameModeHostOption_NoSmallSkins, 0);
        data->mPlayerSettings.Set(eCustomGameModeHostOption_TimeTrial, data->mFlag68);
    }
    SetPlayerSettings(data->mPlayerSettings);
}

unsigned int MiniGameDef::CalculateMatchmakingData(bool joining, LaunchMoreOptionsMenuInitData* data,
                                                    unsigned int** values, unsigned int** ids) const
{
    if (data->mValue5C < 1 || data->mValue5C > 4) return 0;
    if (!joining)
    {
        *values = new unsigned int[3];
        (*values)[0] = data->mValue60;
        (*values)[1] = data->mValue64;
        return 3;
    }
    unsigned int count = (data->mValue60 != 0) + (data->mValue64 != 0);
    *values = new unsigned int[count];
    *ids = new unsigned int[count];
    unsigned int index = 0;
    if (data->mValue60 != 0) { (*values)[index] = data->mValue60; (*ids)[index++] = 0; }
    if (data->mValue64 != 0) { (*values)[index] = data->mValue64; (*ids)[index] = 1; }
    return count;
}

unsigned int MiniGameDef::CalculateJoiningMatchmakingData(LaunchMoreOptionsMenuInitData* data, unsigned int** values, unsigned int** ids) const
{
    return CalculateMatchmakingData(true, data, values, ids);
}

unsigned int MiniGameDef::CalculateHostingMatchmakingData(LaunchMoreOptionsMenuInitData* data, unsigned int** values) const
{
    if (!data->mFlag58 && data->mFlag68) return CalculateMatchmakingData(false, data, values, NULL);
    return 0;
}

bool MiniGameDef::AllowsUGC() const { return mAllowsUGC; }

bool MiniGameDef::HasMultipleMaps() const
{
    if (mId == eMiniGameId_Tumble) return GetOptionsCount(eCustomGameModeHostOption_MapVariant) > 1;
    return mId == eMiniGameId_Battle || mId == eMiniGameId_Glide;
}

bool MiniGameDef::HasMultipleVariants() const { return GetGameVariantCount() > 1; }

unsigned int MiniGameDef::VotableType() const
{
    if (mId == eMiniGameId_Tumble) return 1;
    if (mId != eMiniGameId_Build)
    {
        if (HasMultipleMaps()) return 0;
        if (HasMultipleVariants()) return 1;
    }
    return 2;
}

bool MiniGameDef::HasRoundLengthOption() const { return GetOptionsCount(eCustomGameModeHostOption_RoundLength) > 1; }
bool MiniGameDef::HasGameVariants() const { return GetGameVariantCount() != 0; }
bool MiniGameDef::HasCentralSpawnOption() const { return mId == eMiniGameId_Battle; }
bool MiniGameDef::HasTeamGameOption() const { return GetOptionsCount(eCustomGameModeHostOption_IsTeams) > 1; }
bool MiniGameDef::IsHUDDisabled() const { return mId == eMiniGameId_Tumble || mId == eMiniGameId_Glide; }
bool MiniGameDef::ArePortalsDisabled() const { return true; }
bool MiniGameDef::IsBatSqueakDisabled() const { return mId == eMiniGameId_Glide; }
unsigned int MiniGameDef::GetLayerCount() const { return mPlayerSettings.mLayerCount.isSet() ? mPlayerSettings.mLayerCount.get() : 0; }
double MiniGameDef::GetLayerScale(float value) const { return mPlayerSettings.mArenaSizeMultiplier.isSet() ? mPlayerSettings.mArenaSizeMultiplier.get() : value; }
unsigned int MiniGameDef::GetMapSizeOverride() const { return mPlayerSettings.mMapSizeOverride.isSet() ? mPlayerSettings.mMapSizeOverride.get() : 0; }

std::wstring MiniGameDef::GetMapSizeOverrideText(int value) const
{
    std::wstringstream text;
    text << L"mapSize." << value;
    return text.str();
}

std::wstring MiniGameDef::getTextureName() const { return mTextureName; }
std::wstring MiniGameDef::getIcon() const
{
    return std::wstring(L"Graphics\\") + getTextureName() + L".png";
}

bool MiniGameDef::HasFeatureLookBack() const { return mId == eMiniGameId_Glide; }
bool MiniGameDef::HasFeatureStartCameraAnimation() const { return mId == eMiniGameId_Tumble; }
bool MiniGameDef::HasFeatureFinishLineSoundEffect() const { return mId == eMiniGameId_Glide; }
bool MiniGameDef::HasFeatureCheckpoints() const { return mId == eMiniGameId_Glide; }
bool MiniGameDef::HasFeatureClouds() const { return mId != eMiniGameId_Glide; }
bool MiniGameDef::HasFeatureCustomBeacons() const { return mId == eMiniGameId_Glide; }
bool MiniGameDef::HasFeatureDirectionalFireworks() const { return mId == eMiniGameId_Tumble; }
bool MiniGameDef::HasFeatureFairExplosions() const { return mId == eMiniGameId_Tumble; }
bool MiniGameDef::HasFeatureReducedKnockback() const { return mId == eMiniGameId_Tumble && getCurrentLootSet().getAttribute(L"reducedKnockback", L"false") == L"true"; }
bool MiniGameDef::HasFeatureArrowCanKnockback() const { return mId == eMiniGameId_Tumble && getCurrentLootSet().getAttribute(L"arrowCanKnockback", L"false") == L"true"; }
bool MiniGameDef::HasFeatureSplashLevitatePotions() const { return mId == eMiniGameId_Tumble && getCurrentLootSet().getAttribute(L"splashLevitatePotions", L"false") == L"true"; }
bool MiniGameDef::HasFeatureShorterHitbox() const { return mId == eMiniGameId_Tumble; }
bool MiniGameDef::HasFeatureGhostThroughProjectiles() const
{
    std::wstring value = getCurrentLootSet().getAttribute(L"ghostThroughProjectiles", L"false");
    return mId == eMiniGameId_Tumble && (value == L"true" || value == L"always");
}
bool MiniGameDef::HasFeatureProjectilesBreakBlocks() const { return mId == eMiniGameId_Tumble; }
bool MiniGameDef::HasFeatureThrownProjectilesAreFireImmune() const { return mId == eMiniGameId_Tumble; }
bool MiniGameDef::HasFeatureAutoElytraFly() const { return mId == eMiniGameId_Tumble && getCurrentLootSet().getAttribute(L"autoElytraFly", L"false") == L"true"; }
bool MiniGameDef::HasFeatureInvertedExplodingGrenades() const { return mId == eMiniGameId_Tumble && getCurrentLootSet().getAttribute(L"invertedExplodingGrenades", L"false") == L"true"; }
bool MiniGameDef::HasFeatureLessExplosionParticles() const { return mId == eMiniGameId_Tumble; }
bool MiniGameDef::HasFeatureElytraFlyingOnSpawn() const { return mId == eMiniGameId_Glide; }
bool MiniGameDef::HasFeatureOverrideHandSwing() const { return mId != eMiniGameId_Any; }
bool MiniGameDef::HasFeatureReducedParticles() const { return mId == eMiniGameId_Tumble; }
bool MiniGameDef::HasFeatureSnowballAwards() const { return mId == eMiniGameId_Tumble && getCurrentLootSet().getAttribute(L"snowballAwards", L"false") == L"true"; }
