#include "game/world/generation/PlayerUID.hpp"

#include <cstring>
#include <cwchar>

struct FFLCreateID
{
    unsigned char data[10];
};

template<typename T> T _fromHEXString(const std::wstring& text);
template <> int _fromHEXString<int>(const std::wstring& text);

PlayerUID::PlayerUID(int uid)
{
    memset(this, 0, sizeof(PlayerUID));
}

PlayerUID::PlayerUID(ACTUuid uuid, bool isOnlineAccount, bool isDRCController)
{
    this->uuid = uuid;

    this->isUuid = true;
    this->isOnlineAccount = isOnlineAccount;
    this->hasPrincipalId = false;
    this->isDRCController = isDRCController;
}

PlayerUID::PlayerUID(FFLCreateID createId, unsigned int principalId, bool hasPrincipalId, bool isDRCController)
{
    memcpy(mii.createId, &createId, sizeof(FFLCreateID));
    mii.principalId = hasPrincipalId ? principalId : 0;

    this->isUuid = false;
    this->isOnlineAccount = false;
    this->hasPrincipalId = hasPrincipalId;
    this->isDRCController = isDRCController;
}

PlayerUID::PlayerUID(std::wstring uidString)
{
    if (uidString.length() > 28)
    {
        for (int i = 0; i < 16; ++i)
        {
            uuid.data[i] = (unsigned char)_fromHEXString<int>(uidString.substr(i * 2, 2));
        }

        isUuid = true;
        hasPrincipalId = false;
    }
    else if (uidString.length() > 20)
    {
        for (int i = 0; i < 10; ++i)
        {
            mii.createId[i] = (unsigned char)_fromHEXString<int>(uidString.substr(i * 2, 2));
        }

        for (int i = 0; i < 4; ++i)
        {
            mii.principalId = 0;
            mii.principalIdBytes[i] = (unsigned char)_fromHEXString<int>(uidString.substr(20 + i * 2, 2));
        }

        isUuid = false;
        hasPrincipalId = true;
    }
    else
    {
        for (int i = 0; i < 10; ++i)
        {
            mii.createId[i] = (unsigned char)_fromHEXString<int>(uidString.substr(i * 2, 2));
        }

        mii.principalId = 0;

        isUuid = false;
        hasPrincipalId = false;
    }
}

std::wstring PlayerUID::UuidString() const
{
    wchar_t text[40];

    swprintf(text, 40, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
             uuid.data[0], uuid.data[1], uuid.data[2], uuid.data[3],
             uuid.data[4], uuid.data[5], uuid.data[6], uuid.data[7],
             uuid.data[8], uuid.data[9], uuid.data[10], uuid.data[11],
             uuid.data[12], uuid.data[13], uuid.data[14], uuid.data[15]);

    return std::wstring(text);
}

std::wstring PlayerUID::toString() const
{
    if (isUuid)
    {
        return UuidString();
    }

    wchar_t text[40];

    if (hasPrincipalId)
    {
        swprintf(text, 40, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                 mii.createId[0], mii.createId[1], mii.createId[2], mii.createId[3],
                 mii.createId[4], mii.createId[5], mii.createId[6], mii.createId[7],
                 mii.createId[8], mii.createId[9],
                 mii.principalIdBytes[0], mii.principalIdBytes[1],
                 mii.principalIdBytes[2], mii.principalIdBytes[3]);
    }
    else
    {
        swprintf(text, 40, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                 mii.createId[0], mii.createId[1], mii.createId[2], mii.createId[3],
                 mii.createId[4], mii.createId[5], mii.createId[6], mii.createId[7],
                 mii.createId[8], mii.createId[9]);
    }

    return std::wstring(text);
}

bool PlayerUID::operator==(const PlayerUID& other) const
{
    if (isUuid != other.isUuid)
    {
        return false;
    }

    if (isUuid)
    {
        return uuid == other.uuid;
    }

    return mii.principalId == other.mii.principalId &&
           memcmp(mii.createId, other.mii.createId, sizeof(mii.createId)) == 0;
}

bool PlayerUID::operator!=(const PlayerUID& other) const
{
    return !(*this == other);
}
