#include "game/world/block/SignBlockEntity.hpp"

#include <boost/shared_ptr.hpp>
#include <cwchar>

#include "game/data/CompoundTag.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/world/Level.hpp"
#include "game/core/CriticalSection.hpp"

class BlockState;
class ServerLevel;

namespace
{
    OSFastMutex gSignUniqueIdMutex;
    bool gSignUniqueIdMutexInitialized = true;
    int gSignUniqueId = 0;
}

struct _STRING_VERIFY_RESPONSE
{
    unsigned short count;
    unsigned short reserved;
    int* results;
};

SignBlockEntity::SignBlockEntity()
    : mCreatedPlayer(), mVerified(true), mCensored(false),
      mCreatedByRestrictedPlayer(false), mCreatedByHostPlayer(true),
      mUnknown(-1), mUniqueId(0), mTextCacheIndex(0), mTextCacheEnabled(true)
{
    setUniqueId();
}

SignBlockEntity::~SignBlockEntity()
{
}

BlockEntity* SignBlockEntity::create()
{
    return new SignBlockEntity();
}

void SignBlockEntity::setUniqueId()
{
    if (gSignUniqueIdMutexInitialized)
    {
        InitializeCriticalSection(&gSignUniqueIdMutex);
        gSignUniqueIdMutexInitialized = false;
    }
    EnterCriticalSection(&gSignUniqueIdMutex);
    mUniqueId = gSignUniqueId++;
    if (gSignUniqueId == -1)
    {
        gSignUniqueId = 0;
    }
    LeaveCriticalSection(&gSignUniqueIdMutex);
}

void SignBlockEntity::SetMessage(int line, std::wstring& message)
{
    mMessages[line] = message;
    setUniqueId();
}

bool SignBlockEntity::IsVerified()
{
    return mVerified;
}

void SignBlockEntity::SetVerified(bool verified)
{
    mVerified = verified;
}

bool SignBlockEntity::IsCensored()
{
    return mCensored;
}

void SignBlockEntity::SetCensored(bool censored)
{
    mCensored = censored;
}

void SignBlockEntity::setCreatedByRestrictedPlayer(bool restricted)
{
    mCreatedByRestrictedPlayer = restricted;
}

bool SignBlockEntity::wasCreatedByRestrictedPlayer() const
{
    return mCreatedByRestrictedPlayer;
}

bool SignBlockEntity::wasCreatedByHostPlayer()
{
    return mCreatedByHostPlayer;
}

void SignBlockEntity::setCreatedByHostPlayer(bool host)
{
    mCreatedByHostPlayer = host;
}

void SignBlockEntity::tick()
{
    BlockEntity::tick();
}

CompoundTag* SignBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putString(L"Text1", mMessages[0]);
    tag->putString(L"Text2", mMessages[1]);
    tag->putString(L"Text3", mMessages[2]);
    tag->putString(L"Text4", mMessages[3]);
    tag->putBoolean(L"Verified", mVerified);
    tag->putBoolean(L"Censored", mCensored);
    tag->putBoolean(L"CreatedByRestrictedPlayer", mCreatedByRestrictedPlayer);
    tag->putInt(L"CreatedByHost", mCreatedByHostPlayer);
    tag->putString(L"uuid", mCreatedPlayer);
    return tag;
}

void SignBlockEntity::setLevelForLoading(Level* level)
{
    BlockEntity::setLevel(level);
}

void SignBlockEntity::load(CompoundTag* tag)
{
    mVerified = false;
    BlockEntity::load(tag);
    for (int line = 0; line < 4; ++line)
    {
        wchar_t key[16];
        swprintf(key, 16, L"Text%d", line + 1);
        mMessages[line] = tag->getString(key);
        if (mMessages[line].size() > 15)
        {
            mMessages[line].resize(15);
        }
    }
    if (tag->contains(L"Verified"))
    {
        mVerified = tag->getBoolean(L"Verified");
    }
    if (tag->contains(L"Censored"))
    {
        mCensored = tag->getBoolean(L"Censored");
    }
    if (tag->contains(L"CreatedByRestrictedPlayer"))
    {
        mCreatedByRestrictedPlayer = tag->getBoolean(L"CreatedByRestrictedPlayer");
    }
    if (tag->contains(L"CreatedByHost"))
    {
        mCreatedByHostPlayer = tag->getBoolean(L"CreatedByHost");
    }
    if (tag->contains(L"uuid"))
    {
        mCreatedPlayer = tag->getString(L"uuid");
    }
    setChanged();
}

boost::shared_ptr<Packet> SignBlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<Packet>(new ClientboundBlockEntityDataPacket(getBlockPos(), 9, 0, getUpdateTag()));
}

CompoundTag* SignBlockEntity::getUpdateTag()
{
    CompoundTag* tag = new CompoundTag();
    save(tag);
    return tag;
}

bool SignBlockEntity::onlyOpCanSetNbt()
{
    return true;
}

void SignBlockEntity::setCreatedPlayer(const std::wstring& player)
{
    mCreatedPlayer = player;
}

std::wstring SignBlockEntity::getCreatedPlayerUUID()
{
    return mCreatedPlayer;
}

unsigned int SignBlockEntity::StringVerifyCallback(void* context, _STRING_VERIFY_RESPONSE* response)
{
    boost::shared_ptr<SignBlockEntity>* sign = static_cast<boost::shared_ptr<SignBlockEntity>*>(context);
    if (sign != NULL && *sign)
    {
        (*sign)->mVerified = true;
        (*sign)->mCensored = false;
        if (response != NULL)
        {
            for (unsigned int i = 0; i < response->count; ++i)
            {
                if (response->results[i] != 0)
                {
                    (*sign)->mCensored = true;
                }
            }
        }
        (*sign)->setUniqueId();
    }
    return 0;
}

void SignBlockEntity::setChanged()
{
    mVerified = true;
    setUniqueId();
}

boost::shared_ptr<BlockEntity> SignBlockEntity::clone()
{
    boost::shared_ptr<BlockEntity> result(new SignBlockEntity());
    BlockEntity::clone(result);
    boost::shared_ptr<SignBlockEntity> sign = boost::static_pointer_cast<SignBlockEntity>(result);
    for (int line = 0; line < 4; ++line)
    {
        sign->mMessages[line] = mMessages[line];
    }
    sign->mVerified = mVerified;
    sign->mCensored = mCensored;
    return result;
}

bool SignBlockEntity::executeClickCommands(boost::shared_ptr<Player> player)
{
    return true;
}

int SignBlockEntity::getUniqueId()
{
    return mUniqueId;
}

void SignBlockEntity::setTextCacheIndex(int index)
{
    mTextCacheIndex = index;
}

int SignBlockEntity::getTextCacheIndex()
{
    return mTextCacheIndex;
}

void SignBlockEntity::disableTextCache()
{
    mTextCacheEnabled = false;
}

bool SignBlockEntity::getTextCacheEnabled()
{
    return mTextCacheEnabled;
}

unsigned long long SignBlockEntity::GetType()
{
    return 0x0000000001000202ULL;
}
