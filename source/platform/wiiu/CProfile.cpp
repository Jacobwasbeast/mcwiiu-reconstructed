#include "platform/wiiu/CProfile.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cwchar>

#include "game/core/CAchievements.hpp"
#include "platform/wiiu/CInput.hpp"

namespace nn
{
    namespace act
    {
        unsigned int Initialize();
        unsigned int GetUuid(ACTUuid*);
        int GetSlotNoEx(const ACTUuid&);
        bool IsNetworkAccount();
        unsigned int GetAccountId(char*);
        unsigned int GetPrincipalId();
        unsigned char GetSlotNo();
        int GetNumOfAccounts();
    }

    namespace fp
    {
        unsigned int Initialize();
        int ResultToErrorCode(unsigned int);
        unsigned int UpdateGameMode(const GameMode*, const wchar_t*, unsigned int);
        unsigned int UpdateGameModeDescription(const wchar_t*);
    }
}

unsigned int GetTickCount();
void getPlayerUID(PlayerUID*, int, bool);
int getSlotId(int);
int SCIGetParentalEnable(unsigned char*);
int SCIGetParentalAccountEShopPurchase(unsigned char*, unsigned char);
int SCIGetParentalAccountNetCommunicationOnGame(unsigned char*, unsigned char);
int _SCIReadSysConfig(const char*, unsigned int, unsigned int, void*);
int VerifyCountryRange(unsigned int*);
int SCIGetISOResource(char*, unsigned int, const char*);
void OSFastMutex_Init(OSFastMutex*, const char*);
void OSFastMutex_Lock(OSFastMutex*);
void OSFastMutex_Unlock(OSFastMutex*);
unsigned long long OSGetTime();
void OSSleepTicks(unsigned long long);
void OSResumeThread(OSThread*);
unsigned int* OSGetSystemInfo();
void* MEMAllocFromDefaultHeapEx(unsigned int, int);
void MEMFreeToDefaultHeap(void*);
void* allocAligned();
int FSInitCmdBlock(void*);
int FSAddClient(FSClient*, unsigned int);
int FSSetStateChangeNotification(FSClient*, void*);
void stateChangeCallback(FSClient*, FSVolumeState, void*);
int SAVEInitSaveDir(unsigned char);
int SAVEOpenFile(FSClient*, void*, unsigned char, const char*, const char*, int*, unsigned int);
int SAVEFlushQuota(FSClient*, void*, unsigned char, unsigned int);
int FSWriteFile(FSClient*, void*, const void*, unsigned int, unsigned int, int, unsigned int, unsigned int);
int FSReadFile(FSClient*, void*, void*, unsigned int, unsigned int, int, unsigned int, unsigned int);
int FSCloseFile(FSClient*, void*, int, unsigned int);
int FSGetStatFile(FSClient*, void*, int, void*, unsigned int);

CProfile* gProfile;

static char sCountryCodeData[0x300];

void getPlayerUID(PlayerUID* uid, int quadrant, bool includeOffline)
{
    *uid = PlayerUID(0);
    if (gProfile->IsValidPlayer(quadrant))
        gProfile->GetXUID(quadrant, uid, includeOffline);
}

int getSlotId(int quadrant)
{
    PlayerUID uid(0);
    getPlayerUID(&uid, quadrant, false);
    if ((uid.flags & 0x80000000u) != 0)
        return nn::act::GetSlotNoEx(uid.uuid);
    return 0;
}

void* allocAligned()
{
    void* data = MEMAllocFromDefaultHeapEx(0x40, 0x40);
    std::memset(data, 0, 0x40);
    return data;
}

int SCIGetCafeCountry(unsigned int* country)
{
    int result = _SCIReadSysConfig("cafe.cntry_reg", 3, 4, country);
    VerifyCountryRange(country);
    return result;
}

const char* SCIGetCountryCodeA2(unsigned int country)
{
    if (country < 0x100 && SCIGetISOResource(sCountryCodeData, sizeof(sCountryCodeData), "country.txt") == 1)
    {
        const char* end = sCountryCodeData + country * 3 + 2;
        while (*++end != '\0')
        {
        }
        if (end != sCountryCodeData + country * 3)
            return sCountryCodeData + country * 3;
    }
    return NULL;
}

int SCIGetParentalEnable(unsigned char* enabled)
{
    int result = _SCIReadSysConfig("parent.enable", 1, 1, enabled);
    if (*enabled >= 2)
        *enabled = 1;
    else if (static_cast<signed char>(*enabled) < 0)
        *enabled = 0;
    return result;
}

int SCIGetUCParentalAccountSlotNum(unsigned char accountSlot)
{
    return accountSlot;
}

int SCIGetParentalAccountEShopPurchase(unsigned char* blocked, unsigned char accountSlot)
{
    int slot = SCIGetUCParentalAccountSlotNum(accountSlot);
    if (slot == 0)
        return -5;

    char key[0x40];
    std::snprintf(key, sizeof(key), "p.acct.%d.eshop.purchase", slot);
    int result = _SCIReadSysConfig(key, 1, 1, blocked);
    if (*blocked >= 2)
        *blocked = 1;
    else if (static_cast<signed char>(*blocked) < 0)
        *blocked = 0;
    return result;
}

int SCIGetParentalAccountNetCommunicationOnGame(unsigned char* blocked, unsigned char accountSlot)
{
    int slot = SCIGetUCParentalAccountSlotNum(accountSlot);
    if (slot == 0)
        return -5;

    char key[0x40];
    std::snprintf(key, sizeof(key), "p.acct.%d.net.communication.on.game", slot);
    int result = _SCIReadSysConfig(key, 1, 1, blocked);
    if (*blocked >= 2)
        *blocked = 1;
    else if (static_cast<signed char>(*blocked) < 0)
        *blocked = 0;
    return result;
}

static unsigned long long getSystemMilliseconds()
{
    return OSGetTime() * 1000 / (*OSGetSystemInfo() >> 2);
}

CProfile::PROFILE_DETAILS::PROFILE_DETAILS()
    : signedIn(false)
{
}

CProfile::CONTEXTVAL::CONTEXTVAL()
    : boost::unordered_map<int, int>(11)
{
}

CProfile::CProfile()
    : mMiiData(new MiiData()),
      mLoadedMiiData(mMiiData),
      mFatalTrophyErrorId(0),
      mProfileProcessState(eProfileProcessState_Idle),
      mInitialisationData(0),
      mPrimaryProfileReady(true),
      mSignInChange(false),
      mPrimaryPlayerChanged(false),
      mSignInCountdown(0.0f),
      mLastSignInTick(GetTickCount()),
      mSignedInUserCount(0),
      mSignedInUsersMask(0),
      mRequestOnline(false),
      mRequestGuest(false),
      mRequestSubAccount(false),
      mRequestForce(false),
      mRequestTrial(false),
      mRequestFromGame(false),
      mRequestExtra(false),
      mLockedProfile(-1),
      mSignInRequestState(0),
      mSignInRequestResult(0),
      mSignInUIHandler(NULL),
      mSignInUIContext(NULL),
      mSignInChoicesHandler(NULL),
      mRequestedSignInQuadrant(-1),
      mSignInChoicesPending(false),
      mSignInChoicesAccepted(false),
      mSignInChoice(0),
      mSignInChoiceExtra(0),
      mOfflineProfileHandler(NULL),
      mOfflineProfileContext(NULL),
      mOfflineProfileQuadrant(0),
      mGetStringHandler(NULL),
      mPlayerListTitleId(0),
      mServiceId(NULL),
      mMinimumAge(99),
      mAccountReady(false),
      mProfileReady(false),
      mNetworkTicketReady(false),
      mNetworkAvailable(false),
      mNetworkStatus(false),
      mNetworkStatusPending(false),
      mFriendPresenceOnline(false),
      mNetworkStateReserved(false),
      mNetworkAccount(false),
      mPrincipalId(0),
      mFullVersion(false),
      mLicenseState(false),
      mUpsellFromTrial(false),
      mSystemUIShowing(false),
      mLeaderboardDisable(false),
      mDebugFullOverride(false),
      mPrimaryPad(0),
      mLiveConnectionStatus(0),
      mSignInChangeHandler(NULL),
      mSignInChangeContext(NULL),
      mProfileReadErrorHandler(NULL),
      mProfileReadErrorContext(NULL),
      mNotificationsHandler(NULL),
      mNotificationsContext(NULL),
      mUpsellHandler(NULL),
      mUpsellContext(NULL),
      mUpsellType(eUpsellType_None),
      mUpsellQuadrant(0),
      mPresenceContexts(11),
      mPresenceStrings(11),
      mPresenceContextStrings(11),
      mPresenceReserved(0),
      mLastPresenceUpdate(0),
      mLastGameModeUpdate(0),
      mPresenceUpdatePending(false),
      mPresenceUpdateDelay(false),
      mGameModeUpdateReady(false),
      mRichPresenceSettingHandler(NULL),
      mRichPresenceState(0),
      mRichPresenceStatus(0),
      mFriendPresenceHandler(NULL),
      mFriendPresenceContext(NULL),
      mRichPresenceStringHandler(NULL),
      mAwardHandler(NULL),
      mAwardContext(NULL),
      mInitialisedSaveDirectories(0),
      mAwardThreadStack(NULL),
      mAwardThreadStackTop(NULL)
{
    std::memset(mAccountState, 0, sizeof(mAccountState));
    std::memset(mNetworkAccountId, 0, sizeof(mNetworkAccountId));
    std::memset(mParentalControls, 0, sizeof(mParentalControls));
    std::memset(mProfileResult, 0, sizeof(mProfileResult));
    std::memset(mPrimaryState, 0, sizeof(mPrimaryState));
    std::memset(mRichPresenceData, 0, sizeof(mRichPresenceData));
    std::memset(mGameModeData, 0, sizeof(mGameModeData));
    std::memset(mCurrentGameActivity, 0, sizeof(mCurrentGameActivity));
    std::memset(mAchievementsByPad, 0, sizeof(mAchievementsByPad));
    InitializeCriticalSection(&mProfileCriticalSection);
    InitializeCriticalSection(&mInitSaveDirCriticalSection);
    mMiiData = new MiiData();
    mLoadedMiiData->mSysUserMiis = mMiiData->mSysUserMiis;
    mLoadedMiiData->mGuestMiis = mMiiData->mGuestMiis;
    mLoadedMiiData->mMiiMakerMiis = mMiiData->mMiiMakerMiis;
    gProfile = this;
    SystemOnCreate();
    AwardManagerOnCreate();
    RichPresenceOnCreate();
}

void CProfile::SetSignInChange(bool changed)
{
    mSignInChange = changed;
}

void CProfile::Initialise(const void*, const void*, unsigned short, unsigned int, unsigned int, unsigned int*, int, unsigned int*)
{
    nn::act::Initialize();
    nn::act::GetUuid(&mAccountUuid);
    TryToConnectOnline();
    mNetworkAccount = nn::act::IsNetworkAccount();
    if (mNetworkAccount)
    {
        nn::act::GetAccountId(mNetworkAccountId);
        mPrincipalId = nn::act::GetPrincipalId();
    }
    else
    {
        mPrincipalId = 0;
    }
    mProfileReady = true;
}

int CProfile::GetLockedProfile()
{
    return mLockedProfile;
}

void CProfile::SetLockedProfile(int quadrant)
{
    if (mLockedProfile != quadrant)
    {
        if (quadrant >= 0)
            OnSignin(quadrant);
        mLockedProfile = quadrant;
    }
}

bool CProfile::IsSignedIn(int quadrant)
{
    return quadrant >= 0 && quadrant < 4 && mProfiles[quadrant].signedIn;
}

bool CProfile::IsSignedInLive(int quadrant)
{
    return IsSignedIn(quadrant) && mNetworkAvailable && mFriendPresenceOnline;
}

bool CProfile::IsSigningIn(int quadrant)
{
    return mProfiles[quadrant].signingIn;
}

bool CProfile::IsGuest(int quadrant)
{
    return mProfiles[quadrant].mii->mIsGuestMii;
}

int CProfile::RequestSignInUI(bool online, bool guest, bool subAccount, bool force, bool trial, SignInUIHandler handler, void* context, int quadrant)
{
    return RequestSignInUI(online, guest, subAccount, force, trial, false, handler, context, quadrant);
}

int CProfile::RequestSignInUI(bool online, bool guest, bool subAccount, bool force, bool trial, bool fromGame, SignInUIHandler handler, void* context, int quadrant)
{
    mRequestOnline = online;
    mRequestGuest = guest;
    mRequestSubAccount = subAccount;
    mRequestForce = force;
    mRequestTrial = trial;
    mRequestFromGame = fromGame;
    mSignInUIHandler = handler;
    mSignInUIContext = context;
    mRequestedSignInQuadrant = quadrant;
    mSignInChoicesHandler(quadrant);
    return 0;
}

void CProfile::checkParentalControl(int quadrant)
{
    ParentalControlState& state = mParentalControls[quadrant];
    if (!state.loaded)
    {
        unsigned char result = 0;
        unsigned char slot = nn::act::GetSlotNo();
        if (SCIGetParentalEnable(&result) == 1)
            state.enabled = result == 1;
        if (SCIGetParentalAccountEShopPurchase(&result, slot) == 1)
            state.purchasesBlocked = result == 1;
        if (SCIGetParentalAccountNetCommunicationOnGame(&result, slot) == 1)
            state.communicationBlocked = result == 1;
        state.loaded = true;
    }
}

int CProfile::SignInUser(MiiDataEntry* mii, int pad)
{
    if (mii != NULL)
    {
        for (int quadrant = 0; quadrant < 4; ++quadrant)
        {
            if (mProfiles[quadrant].signedIn && std::memcmp(mProfiles[quadrant].mii->mCreatorId, mii->mCreatorId, 10) == 0)
                return -1;
        }
    }
    int quadrant = 0;
    while (quadrant < 4 && mProfiles[quadrant].signedIn)
        ++quadrant;
    if (quadrant == 4)
        return -1;
    PROFILE_DETAILS& profile = mProfiles[quadrant];
    profile.mii = mii;
    profile.signedIn = true;
    if (mii != NULL)
    {
        profile.onlineAccount = mii->mIsAccountMii && mii->mUuid == mAccountUuid;
        if (mProfileReady)
        {
            if (quadrant == 0)
                std::swprintf((wchar_t*)profile.gamertag, 32, L"%s", (const wchar_t*)mii->mName);
            else
                std::swprintf((wchar_t*)profile.gamertag, 32, L"%s (%d)", (const wchar_t*)mii->mName, quadrant + 1);
        }
        checkParentalControl(quadrant);
    }
    OnSignin(pad);
    g_pInput->LockPad(pad, quadrant);
    SetSignInChange(mii != NULL);
    return quadrant;
}

void CProfile::SignOutUser(FFLCreateID& createId)
{
    for (int quadrant = 0; quadrant < 4; ++quadrant)
    {
        if (mProfiles[quadrant].signedIn && std::memcmp(mProfiles[quadrant].mii->mCreatorId, &createId, 10) == 0)
        {
            mSignOutQueue.push_back(quadrant);
            return;
        }
    }
}

void CProfile::SignOutUser(int quadrant)
{
    if (quadrant >= 0 && quadrant < 4 && IsSignedIn(quadrant))
        mSignOutQueue.push_back(quadrant);
}

void CProfile::SetUserSigningIn(int quadrant, bool signingIn)
{
    mProfiles[quadrant].signingIn = signingIn;
}

int CProfile::DisplayOfflineProfile(OfflineProfileHandler handler, void* context, int quadrant)
{
    mOfflineProfileHandler = handler;
    mOfflineProfileContext = context;
    mOfflineProfileQuadrant = quadrant;
    mProfileProcessState = eProfileProcessState_OfflineProfile;
    return 0;
}

void CProfile::RequestConvertOfflineToGuestUI(OfflineProfileHandler, SignInUIHandler handler, void* context, int quadrant)
{
    RequestSignInUI(false, true, false, false, false, true, handler, context, quadrant);
}

void CProfile::SetPrimaryPlayerChanged(bool changed)
{
    mPrimaryPlayerChanged = changed;
}

int CProfile::QuerySigninStatus()
{
    mSignedInUserCount = 0;
    mSignedInUsersMask = 0;
    for (int quadrant = 0; quadrant < 4; ++quadrant)
    {
        if (mProfiles[quadrant].signedIn)
        {
            ++mSignedInUserCount;
            mSignedInUsersMask |= 1 << quadrant;
        }
    }
    return 0;
}

PlayerUID CProfile::QuadrantOfflineXUID(int quadrant)
{
    MiiDataEntry* mii = mProfiles[quadrant].mii;
    if (mii == NULL)
        return PlayerUID(0);
    if (mii->mIsAccountMii)
        return PlayerUID(mii->mUuid, mProfiles[quadrant].onlineAccount, g_pInput->IsPadDRCController(quadrant));
    FFLCreateID* createId = (FFLCreateID*)mii->mCreatorId;
    return PlayerUID(*createId, mPrincipalId, mii->mIsGuestMii, g_pInput->IsPadDRCController(quadrant));
}

PlayerUID CProfile::QuadrantOnlineXUID(int quadrant)
{
    if (mFriendPresenceOnline)
        return QuadrantOfflineXUID(quadrant);
    return PlayerUID(0);
}

void CProfile::GetXUID(int quadrant, PlayerUID* uid, bool online)
{
    *uid = online ? QuadrantOnlineXUID(quadrant) : QuadrantOfflineXUID(quadrant);
}

bool CProfile::AreXUIDSEqual(PlayerUID first, PlayerUID second)
{
    return first == second;
}

bool CProfile::AllowedToPlayMultiplayer(int quadrant)
{
    return IsSignedInLive(quadrant);
}

void CProfile::StartTrialGame()
{
}

void CProfile::AllowedPlayerCreatedContent(int quadrant, bool singlePlayer, int* allowed, int* communicationAllowed)
{
    bool result = true;
    int first = singlePlayer ? quadrant : 0;
    int last = singlePlayer ? quadrant + 1 : 4;
    for (int player = first; player < last && result; ++player)
    {
        if (IsSignedIn(player) && !IsSignedInLive(player))
        {
            checkParentalControl(player);
            result = !mParentalControls[player].communicationBlocked;
        }
    }
    *allowed = result;
    *communicationAllowed = result;
}

bool CProfile::CanViewPlayerCreatedContent(int quadrant, bool singlePlayer, PlayerUID*, unsigned int)
{
    int first = singlePlayer ? quadrant : 0;
    int last = singlePlayer ? quadrant + 1 : 4;
    for (int player = first; player < last; ++player)
    {
        bool chat = true;
        bool content = true;
        GetChatAndContentRestrictions(player, true, &chat, &content, NULL);
        if (!chat && !content)
            return false;
    }
    return true;
}

bool CProfile::GetProfileAvatar(int quadrant, AvatarHandler handler, void* context)
{
    AvatarRequest request = { quadrant, handler, context };
    mAvatarRequests.push_back(request);
    return true;
}

void CProfile::CancelProfileAvatarRequest()
{
}

void CProfile::SetGetStringFunc(GetStringHandler handler)
{
    mGetStringHandler = handler;
}

void CProfile::SetPlayerListTitleID(int titleId)
{
    mPlayerListTitleId = titleId;
}

void CProfile::SetFatalTrophyErrorID(int stringId)
{
    mFatalTrophyErrorId = stringId;
}

const wchar_t* CProfile::GetMiiName(int quadrant)
{
    static const wchar_t empty[] = L"";
    return IsSignedIn(quadrant) ? (const wchar_t*)mProfiles[quadrant].mii->mName : empty;
}

const wchar_t* CProfile::GetGamertag(int quadrant)
{
    if (!IsSignedInLive(quadrant))
        return GetMiiName(quadrant);
    return (const wchar_t*)mProfiles[quadrant].gamertag;
}

std::wstring CProfile::GetDisplayName(int quadrant)
{
    return std::wstring(GetGamertag(quadrant));
}

char* CProfile::GetNetworkAccountID()
{
    return mNetworkAccountId;
}

void CProfile::SetSignInChoicesComplete(bool accepted, int choice, int extra)
{
    mSignInChoicesAccepted = accepted;
    mSignInChoice = choice;
    mSignInChoiceExtra = extra;
    mSignInChoicesPending = true;
}

void CProfile::ResetProfileProcessState()
{
    mProfileProcessState = eProfileProcessState_Idle;
}

bool CProfile::GetSignInChange()
{
    return mSignInChange;
}

void CProfile::CheckForSignInChange()
{
    if (mSignInChange)
    {
        SetSignInChange(false);
        if (!IsFullVersion())
            CallSignInChangeCallback(mPrimaryPlayerChanged);
        else
        {
            mPrimaryPlayerChanged = true;
            CallSignInChangeCallback(true);
        }
    }
    if (mSignInChoicesPending)
    {
        mSignInUIHandler(mSignInUIContext, mSignInChoicesAccepted, mSignInChoice, mSignInChoiceExtra);
        mSignInChoicesPending = false;
    }
}

bool CProfile::PrimaryPlayerChanged()
{
    return mPrimaryPlayerChanged;
}

bool CProfile::PrimaryPlayerHasSignedOutDialogBox(bool)
{
    return false;
}

void CProfile::SetProfileProcessState(EProfileProcessState state)
{
    mProfileProcessState = state;
}

void CProfile::Tick()
{
    TickSystem();
    CheckForSystemNotifications();
    TickRichPresence();
    CheckForSignInChange();
    AwardManagerTick();
    while (!mAvatarRequests.empty())
    {
        AvatarRequest request = mAvatarRequests.back();
        if (request.handler != NULL)
        {
            MiiDataEntry* mii = mProfiles[request.quadrant].mii;
            request.handler(request.context, mii == NULL ? NULL : (unsigned char*)mii->mIconMemory, 0);
        }
        mAvatarRequests.pop_back();
    }
    if (!mSignOutQueue.empty())
    {
        int quadrant = mSignOutQueue.front();
        if (IsSignedIn(quadrant))
        {
            OnSignout(quadrant);
            mProfiles[quadrant] = PROFILE_DETAILS();
            SetSignInChange(true);
        }
        mSignOutQueue.pop_front();
    }
    if (mProfileProcessState == eProfileProcessState_Idle)
    {
        SetLeaderboardDisable(false);
        if (mPrimaryPlayerChanged && GetLockedProfile() != -1)
            SetLockedProfile(-1);
        ShowSigninUI();
    }
}

void CProfile::SetFriendPresenceOnline(bool online)
{
    mFriendPresenceOnline = online;
}

void CProfile::SetNetworkStatus(bool online)
{
    mFriendPresenceOnline = online;
    mNetworkAvailable = online;
    if (mNetworkStatus != online)
    {
        mNetworkStatus = online;
        SetSignInChange(true);
    }
}

void CProfile::SetNetworkStatusWithoutSigninChange(bool online)
{
    mFriendPresenceOnline = online;
    mNetworkAvailable = online;
    mNetworkStatus = online;
}

void CProfile::TryToConnectOnline()
{
}

unsigned int CProfile::GetSignedInUsersMask()
{
    return mSignedInUsersMask;
}

void CProfile::RefreshChatAndContentRestrictions(ProfileReadErrorHandler, void*)
{
}

bool CProfile::GetChatAndContentRestrictions(int, bool* chatRestricted, bool* contentRestricted, int* age)
{
    if (chatRestricted != NULL)
        *chatRestricted = false;
    if (contentRestricted != NULL)
        *contentRestricted = false;
    if (age != NULL)
        *age = 100;
    return true;
}

bool CProfile::GetChatAndContentRestrictions(int, bool, bool* chatRestricted, bool* contentRestricted, int* age)
{
    if (chatRestricted != NULL)
        *chatRestricted = false;
    if (contentRestricted != NULL)
        *contentRestricted = false;
    if (age != NULL)
        *age = 100;
    return true;
}

void CProfile::SetServiceID(char* serviceId)
{
    mServiceId = serviceId;
}

void CProfile::HandleNetworkTicket(int, void*)
{
}

void CProfile::SetMinimumAge(int age)
{
    mMinimumAge = age;
}

unsigned int CProfile::getUserID(int)
{
    return 0;
}

int CProfile::getQuadrant(ACTUuid&)
{
    return -1;
}

void CProfile::SetSignInChoicesCallback(SignInChoicesHandler handler)
{
    mSignInChoicesHandler = handler;
}

std::vector<CProfile::ProfileSignInEntry>* CProfile::GetSignInEntries()
{
    return &mSignInEntries;
}

void CProfile::LoadAllMiiData()
{
    mLoadedMiiData->LoadAllData();
    MiiDataEntry* accountMii = NULL;
    for (std::vector<MiiDataEntry>::iterator it = mLoadedMiiData->mSysUserMiis.begin(); it != mLoadedMiiData->mSysUserMiis.end(); ++it)
    {
        if (it->mUuid == mAccountUuid)
            accountMii = &*it;
    }
    std::memcpy(mAccountState, accountMii->mCreatorId, sizeof(accountMii->mCreatorId));
    SignInUser(accountMii, -1);
}

void CProfile::FreeAllMiiData()
{
    mLoadedMiiData->FreeAllData();
}

int CProfile::GetNumSysUserMiis()
{
    return (int)mLoadedMiiData->mSysUserMiis.size();
}

int CProfile::GetNumFavouriteMiis()
{
    return (int)mLoadedMiiData->mMiiMakerMiis.size();
}

int CProfile::GetNumGuestMiis()
{
    return (int)mLoadedMiiData->mGuestMiis.size();
}

MiiDataEntry* CProfile::GetSysUserMiiData(int index)
{
    return &mLoadedMiiData->mSysUserMiis[index];
}

MiiDataEntry* CProfile::GetFavouriteMiiData(int index)
{
    return &mLoadedMiiData->mMiiMakerMiis[index];
}

MiiDataEntry* CProfile::GetGuestMiiData(int index)
{
    return &mLoadedMiiData->mGuestMiis[index];
}

bool CProfile::IsHasAchievementsFile(int quadrant)
{
    CAchievements* achievements = getAchievements(quadrant);
    return achievements != NULL && achievements->mHasAchievementsFile;
}

void CProfile::ShowSigninUI()
{
    if (GetNumSysUserMiis() == 0)
    {
        SetSignInChange(false);
        SetSysUIShowing(true);
        SetProfileProcessState(eProfileProcessState_ProcessSignIn);
    }
}

int CProfile::GetNumSignedInUsers()
{
    return mSignedInUserCount;
}

bool CProfile::ProcessSignIn()
{
    if (GetNumSignedInUsers() == 0)
        return false;
    if (GetPrimaryPad() != -1 && IsValidPlayer(GetPrimaryPad()) && IsSignedIn(GetPrimaryPad()))
        return true;
    SetLockedProfile(-1);
    return false;
}

bool CProfile::IsValidPlayer(int quadrant)
{
    return (mSignedInUsersMask & (1 << quadrant)) != 0;
}

void CProfile::GetLocalPlayerInfo(nn::pia::transport::Station::PlayerInfo*, int)
{
}

void CProfile::Terminate()
{
}

void CProfile::RichPresenceRegisterPresenceString(int id, int stringId)
{
    mPresenceStrings[id] = stringId;
}

void CProfile::RichPresenceRegisterContext(int id, int contextId)
{
    mPresenceContexts[id] = contextId;
    for (int quadrant = 0; quadrant < 4; ++quadrant)
        mContextValues[quadrant][id] = -1;
}

void CProfile::RichPresenceRegisterContextString(int context, int value, int stringId)
{
    mPresenceContextStrings[context][value] = stringId;
}

void CProfile::RichPresenceInit(int, int titleId)
{
    mRichPresenceSettingHandler = (RichPresenceSettingHandler)titleId;
    unsigned int result = nn::fp::Initialize();
    unsigned int level = (result >> 27) & 3;
    unsigned int moduleMask = level == 3 ? 0x07f00000 : 0x1ff00000;
    unsigned int descriptionMask = level == 3 ? 0x3ff : 0xfffff;
    unsigned int description = result & descriptionMask;
    if ((result & moduleMask) == 0x00c00000 && description > 0x31ff && description < 0x1f400)
    {
        unsigned int errorCode = nn::fp::ResultToErrorCode(result);
        DebugPrintf("\"%s @%d\" failed. errorCode = %03u-%04u\n",
                    "RichPresenceInit", 44, errorCode / 10000, errorCode % 10000);
    }
    mLastPresenceUpdate = getSystemMilliseconds() - 600000;
    mLastGameModeUpdate = 0;
    mCurrentPresenceString.clear();
    mPresenceUpdatePending = false;
    mPresenceUpdateDelay = false;
    mGameModeUpdateReady = false;
}

const wchar_t* CProfile::getStringFromID(int stringId)
{
    return mGetStringHandler(stringId);
}

void CProfile::CreatePresenceString(int quadrant, std::wstring& text)
{
    text.assign(getStringFromID(mPresenceStrings[mCurrentGameActivity[quadrant]]));
    for (boost::unordered_map<int, int>::const_iterator context = mPresenceContexts.begin(); context != mPresenceContexts.end(); ++context)
    {
        const wchar_t* formatToken = getStringFromID(context->second);
        std::wstring::size_type position = text.find(formatToken);
        if (position != std::wstring::npos)
        {
            int setting = mContextValues[quadrant][context->first];
            const wchar_t* replacement = getStringFromID(mPresenceContextStrings[context->first][setting]);
            text.replace(position, std::wcslen(formatToken), replacement);
        }
    }
}

void CProfile::SetPresenceString(int presence)
{
    if (presence != GetPrimaryPad())
        return;
    std::wstring text;
    CreatePresenceString(presence, text);
    if (text == mCurrentPresenceString)
    {
        mPresenceUpdatePending = false;
        return;
    }

    unsigned long long now = getSystemMilliseconds();
    unsigned int gameModeKind;
    std::memcpy(&gameModeKind, mGameModeData, sizeof(gameModeKind));
    unsigned int interval = gameModeKind == 1 ? 120000 : 600000;
    if (now - mLastPresenceUpdate <= interval && !mGameModeUpdateReady)
    {
        if (!mPresenceUpdateDelay && !mPresenceUpdatePending)
            mPresenceUpdatePending = true;
        return;
    }

    unsigned int result;
    if (mGameModeUpdateReady || mPresenceUpdateDelay)
        result = nn::fp::UpdateGameMode((nn::fp::GameMode*)mGameModeData, text.c_str(), 0);
    else
        result = nn::fp::UpdateGameModeDescription(text.c_str());
    if ((result & 0x80000000U) == 0)
    {
        mLastPresenceUpdate = now;
        mPresenceUpdatePending = false;
        mGameModeUpdateReady = false;
        mPresenceUpdateDelay = false;
        mCurrentPresenceString = text;
    }
}

void CProfile::SetRichPresenceContextValue(int context, int value, int setting)
{
    if (context == 0 && mContextValues[0][value] != setting)
    {
        mContextValues[0][value] = setting;
        SetPresenceString(0);
    }
}

void CProfile::SetCurrentGameActivity(int activity, int presence, bool force)
{
    if (activity == 0)
    {
        if (mCurrentGameActivity[0] != presence)
        {
            mCurrentGameActivity[0] = presence;
            SetPresenceString(0);
        }
        if (force)
        {
            for (int quadrant = 1; quadrant < 4; ++quadrant)
            {
                if (mCurrentGameActivity[quadrant] != 0)
                {
                    mCurrentGameActivity[quadrant] = 0;
                    SetPresenceString(quadrant);
                }
            }
        }
    }
}

void CProfile::SetRichPresenceSettingFn(RichPresenceSettingHandler handler)
{
    mRichPresenceSettingHandler = handler;
}

void CProfile::SetRichPresenceGameModeData(nn::fp::GameMode& gameMode)
{
    if (std::memcmp(mGameModeData, &gameMode, sizeof(mGameModeData)) == 0)
        return;
    std::memcpy(mGameModeData, &gameMode, sizeof(mGameModeData));
    mPresenceUpdateDelay = true;
    mLastGameModeUpdate = getSystemMilliseconds();
}

void CProfile::SetFriendPresenceChangeCallback(FriendPresenceHandler handler, void* context)
{
    mFriendPresenceHandler = handler;
    mFriendPresenceContext = context;
}

void CProfile::SetRichPresenceGetStringCallback(RichPresenceStringHandler handler)
{
    mRichPresenceStringHandler = handler;
}

void CProfile::RichPresenceOnCreate()
{
    mRichPresenceSettingHandler = NULL;
    mRichPresenceStatus = 0;
    mFriendPresenceHandler = NULL;
}

void CProfile::TickRichPresence()
{
    if (mPresenceUpdateDelay && getSystemMilliseconds() - mLastGameModeUpdate > 2000)
    {
        mPresenceUpdateDelay = false;
        mGameModeUpdateReady = true;
    }
    if (mPresenceUpdatePending || mGameModeUpdateReady)
        SetPresenceString(GetPrimaryPad());
}

int CProfile::GetPrimaryPad()
{
    return mPrimaryPad;
}

void CProfile::SetPrimaryPad(int pad)
{
    g_pInput->LockPad(pad, 0);
    mPrimaryPad = 0;
}

void CProfile::ClearGameUsers()
{
    g_pInput->UnlockPad(0);
    for (int quadrant = 1; quadrant < 4; ++quadrant)
    {
        if (IsSignedIn(quadrant))
            SignOutUser(quadrant);
    }
}

bool CProfile::IsFullVersion()
{
    return true;
}

void CProfile::SetFullVersion(bool fullVersion)
{
    mFullVersion = fullVersion;
}

void CProfile::SetSignInChangeCallback(SignInChangeHandler handler, void* context)
{
    mSignInChangeHandler = handler;
    mSignInChangeContext = context;
}

void CProfile::SetNotificationsCallback(NotificationsHandler handler, void* context)
{
    mNotificationsHandler = handler;
    mNotificationsContext = context;
}

bool CProfile::RegionIsNorthAmerica()
{
    return false;
}

bool CProfile::LocaleIsUSorCanada()
{
    return false;
}

int CProfile::GetLiveConnectionStatus()
{
    return mLiveConnectionStatus;
}

bool CProfile::IsSystemUIDisplayed()
{
    return mSystemUIShowing;
}

void CProfile::SetProfileReadErrorCallback(ProfileReadErrorHandler handler, void* context)
{
    mProfileReadErrorHandler = handler;
    mProfileReadErrorContext = context;
}

void CProfile::SetUpsellType(eUpsellType type, int quadrant)
{
    mUpsellType = type;
    mUpsellQuadrant = quadrant;
}

void CProfile::DisplayFullVersionPurchase(bool fromTrial, int quadrant, int)
{
    SetUpsellType(eUpsellType_None, quadrant);
    if (fromTrial)
        mUpsellFromTrial = true;
    SetProfileProcessState(eProfileProcessState_Upsell);
}

void CProfile::SetUpsellCallback(UpsellHandler handler, void* context)
{
    mUpsellHandler = handler;
    mUpsellContext = context;
}

void CProfile::SetDebugFullOverride(bool enabled)
{
    mDebugFullOverride = enabled;
}

void CProfile::DebugPrintf(const char*, ...)
{
}

void CProfile::SystemOnCreate()
{
    mDebugFullOverride = true;
    mSystemUIShowing = false;
}

void CProfile::SetSysUIShowing(bool showing)
{
    mSystemUIShowing = showing;
}

void CProfile::CheckForSystemNotifications()
{
}

void CProfile::SetLeaderboardDisable(bool disabled)
{
    mLeaderboardDisable = disabled;
}

void CProfile::CallSignInChangeCallback(bool primaryChanged)
{
    if (mSignInChangeHandler != NULL)
        mSignInChangeHandler(mSignInChangeContext, primaryChanged, mSignedInUsersMask);
}

void CProfile::TickSystem()
{
}

bool CProfile::AddAwardManagerTask(CAchievements* achievements)
{
    OSFastMutex_Lock(&mAwardMutex);
    mAwardQueue.push_back(achievements);
    OSFastMutex_Unlock(&mAwardMutex);
    return true;
}

int CProfile::GetRegistrationId(int achievementId) const
{
    return mAwardRegistrations[achievementId].registrationId;
}

void CProfile::RegisterAward(int achievementId, int registrationId, eAwardType, int callbackValue, int callbackContext)
{
    AwardRegistration& registration = mAwardRegistrations[achievementId];
    registration.registrationId = registrationId;
    registration.callbackValue = callbackValue;
    registration.callbackContext = callbackContext;
}

int CProfile::GetAwardId(int achievementId)
{
    return achievementId;
}

eAwardType CProfile::GetAwardType(int)
{
    return eAwardType_None;
}

bool CProfile::CanBeAwarded(int, int)
{
    return true;
}

void CProfile::OnSignout(int quadrant)
{
    if (quadrant >= 0 && quadrant < 4 && IsValidPlayer(quadrant))
    {
        mAchievements.insert(mAchievementsByPad[quadrant]);
        mAchievementsByPad[quadrant] = NULL;
    }
}

void CProfile::OnSignin(int quadrant)
{
    if (quadrant < 0 || quadrant >= 4 || !IsValidPlayer(quadrant))
        return;

    PlayerUID uid(0);
    getPlayerUID(&uid, quadrant, false);
    CAchievements*& achievements = mAchievementsByPad[quadrant];
    if (achievements == NULL)
    {
        achievements = new CAchievements(quadrant);
    }
    else if (achievements->mPlayerUID != uid)
    {
        OnSignout(quadrant);
        OnSignin(quadrant);
    }
}

CAchievements* CProfile::getAchievements(int quadrant)
{
    if (quadrant < 0 || quadrant >= 4)
        return NULL;
    OnSignin(quadrant);
    return mAchievementsByPad[quadrant];
}

CAchievements* CProfile::getAchievementsByPad(int quadrant) const
{
    if (quadrant < 0 || quadrant >= 4)
        return NULL;
    return mAchievementsByPad[quadrant];
}

bool CProfile::callAwardManagerCallback(int quadrant, int achievementId) const
{
    CAchievements* achievements = getAchievementsByPad(quadrant);
    if (achievements != NULL && achievements->mIsSystemPlayer)
    {
        if (mAwardHandler != NULL)
        {
            const AwardRegistration& registration = mAwardRegistrations[achievementId];
            mAwardHandler(mAwardContext, quadrant, achievementId, registration.callbackValue, registration.callbackContext);
            return true;
        }
        return false;
    }
    return false;
}

void CProfile::Award(int quadrant, int achievementId, bool)
{
    PlayerUID uid(0);
    getPlayerUID(&uid, quadrant, false);
    CAchievements* achievements = getAchievements(quadrant);
    if (achievementId < 0 || achievementId >= 128 || achievements == NULL)
        return;
    if (!achievements->IsIdle())
    {
        achievements->addPendingAward(achievementId);
        return;
    }
    if (!achievements->SetAwardedBit(achievementId, true))
        callAwardManagerCallback(quadrant, achievementId);
}

bool CProfile::IsAwardsFlagSet(int quadrant, int achievementId)
{
    PlayerUID uid(0);
    getPlayerUID(&uid, quadrant, false);
    CAchievements* achievements = getAchievements(quadrant);
    return achievements != NULL && achievementId >= 0 && achievementId < 128 && achievements->GetAwardedBit(achievementId);
}

void CProfile::SetOnAwardHandler(AwardHandler handler, void* context)
{
    mAwardHandler = handler;
    mAwardContext = context;
}

CAchievements* CProfile::nextAwardManagerTask()
{
    CAchievements* achievements = NULL;
    OSFastMutex_Lock(&mAwardMutex);
    if (!mAwardQueue.empty())
    {
        achievements = mAwardQueue.front();
        mAwardQueue.pop_front();
    }
    OSFastMutex_Unlock(&mAwardMutex);
    return achievements;
}

int CProfile::openAwardManagerFile(int& file, const char* mode, unsigned char slot, const char* filename)
{
    return SAVEOpenFile(&mFileSystemClient, mFileSystemCommand, slot, filename, mode, &file, 0xffffffffU);
}

void CProfile::EnterInitSaveDirCS()
{
    EnterCriticalSection(&mInitSaveDirCriticalSection);
}

void CProfile::LeaveInitSaveDirCS()
{
    LeaveCriticalSection(&mInitSaveDirCriticalSection);
}

int CProfile::initSaveDirectory(unsigned char slot)
{
    nn::act::GetNumOfAccounts();
    unsigned int slotMask = 1U << slot;
    if ((mInitialisedSaveDirectories & slotMask) == 0)
    {
        mInitialisedSaveDirectories |= slotMask;
        if (slot != nn::act::GetSlotNo())
        {
            EnterInitSaveDirCS();
            int result = SAVEInitSaveDir(slot);
            LeaveInitSaveDirCS();
            if (result != 0)
                return false;
        }
    }
    return true;
}

void CProfile::closeAwardManagerFile(int& file)
{
    if (file != -1)
        FSCloseFile(&mFileSystemClient, mFileSystemCommand, file, 0xffffffffU);
}

void CProfile::flushSaveDirectory(unsigned char slot)
{
    nn::act::GetNumOfAccounts();
    SAVEFlushQuota(&mFileSystemClient, mFileSystemCommand, slot, 0xffffffffU);
}

void CProfile::AwardManagerThreadProc(int, void* argument)
{
    CProfile* profile = (CProfile*)argument;
    for (;;)
    {
        C4JThreadImpl::Sleep(70);
        CAchievements* achievements = profile->nextAwardManagerTask();
        if (achievements == NULL)
            continue;

        const char* filename = achievements->GetFilename();
        unsigned int state = achievements->GetState();
        profile->initSaveDirectory(achievements->mSlotId);
        int file = -1;
        if (state == 2)
        {
            if (profile->openAwardManagerFile(file, "w+", achievements->mSlotId, filename) == 0)
            {
                void* data = allocAligned();
                achievements->mHasAchievementsFile = true;
                achievements->WriteTo(data, 16);
                FSWriteFile(&profile->mFileSystemClient, profile->mFileSystemCommand, data, 1, 16, file, 0, 0xffffffffU);
                profile->closeAwardManagerFile(file);
                MEMFreeToDefaultHeap(data);
                if (achievements->mSlotId != nn::act::GetSlotNo())
                    profile->flushSaveDirectory(achievements->mSlotId);
            }
            else
            {
                achievements->WriteTo(NULL, 0);
                profile->closeAwardManagerFile(file);
            }
        }
        else if (state == 1)
        {
            if (profile->openAwardManagerFile(file, "r", achievements->mSlotId, filename) == 0)
            {
                unsigned char stat[104];
                if (FSGetStatFile(&profile->mFileSystemClient, profile->mFileSystemCommand, file, stat, 0xffffffffU) == 0)
                {
                    void* data = allocAligned();
                    int bytesRead = FSReadFile(&profile->mFileSystemClient, profile->mFileSystemCommand, data, 1, 16, file, 0, 0xffffffffU);
                    achievements->mHasAchievementsFile = true;
                    achievements->ReadFrom(data, bytesRead);
                    profile->closeAwardManagerFile(file);
                    MEMFreeToDefaultHeap(data);
                    continue;
                }
            }
            achievements->ReadFrom(NULL, 0);
            profile->closeAwardManagerFile(file);
        }
    }
}

void CProfile::AwardManagerOnCreate()
{
    mAwardHandler = NULL;
    mAwardContext = NULL;
    OSFastMutex_Init(&mAwardMutex, "AwardManager");
    FSInitCmdBlock(mFileSystemCommand);
    FSAddClient(&mFileSystemClient, 0xffffffffU);
    FSStateChangeParams notification = { stateChangeCallback, NULL, 0, 0 };
    FSSetStateChangeNotification(&mFileSystemClient, &notification);
    mAwardThreadStack = MEMAllocFromDefaultHeapEx(60000, 16);
    mAwardThreadStackTop = (unsigned char*)mAwardThreadStack + 60000;
    if (C4JThreadImpl::OSCreateThread4J(&mAwardThread, (int (*)(int, void*))AwardManagerThreadProc, 0, this, mAwardThreadStackTop, 60000, 16, 7) != 0)
        OSResumeThread(&mAwardThread);
    std::memset(mAwardRegistrations, 0xff, sizeof(mAwardRegistrations));
}

void CProfile::AwardManagerTick()
{
    std::set<CAchievements*>::iterator deleting = mAchievements.begin();
    while (deleting != mAchievements.end())
    {
        CAchievements* achievements = *deleting;
        if (!achievements->IsIdle())
        {
            ++deleting;
        }
        else if (achievements->hasPendingAwards())
        {
            std::set<int> awardsNotRegistered;
            achievements->awardPending(awardsNotRegistered);
            ++deleting;
        }
        else if (achievements->IsDirty())
        {
            achievements->StartWrite();
            ++deleting;
        }
        else
        {
            mAchievements.erase(deleting++);
        }
    }

    for (int quadrant = 0; quadrant < 4; ++quadrant)
    {
        CAchievements* achievements = getAchievements(quadrant);
        if (achievements == NULL)
            continue;
        if (achievements->hasPendingAwards())
        {
            std::set<int> awardsNotRegistered;
            achievements->awardPending(awardsNotRegistered);
            for (std::set<int>::const_iterator award = awardsNotRegistered.begin(); award != awardsNotRegistered.end(); ++award)
                callAwardManagerCallback(quadrant, *award);
        }
        else if (achievements->IsIdle())
        {
            achievements->StartWrite();
        }
    }
}

bool CProfile::initSaveDirectoryForQuadrant(int quadrant, unsigned char* slot)
{
    *slot = (unsigned char)getSlotId(quadrant);
    return initSaveDirectory(*slot);
}
