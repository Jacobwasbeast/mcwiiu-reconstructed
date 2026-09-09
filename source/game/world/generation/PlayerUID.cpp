#include "game/world/generation/PlayerUID.hpp"

#include <cstring>
#include <cwchar>

#include "platform/wiiu/FFLiTypes.hpp"

template<typename T> T _fromHEXString(const std::wstring& text);
template<> int _fromHEXString<int>(const std::wstring& text);

PlayerUID::PlayerUID(int uid)
{
    (void)uid;
    std::memset(this, 0, sizeof(PlayerUID));
}

PlayerUID::PlayerUID(ACTUuid uuid, bool onlineAccount, bool drcController)
{
    this->uuid = uuid;
    flags = (0x80000000u | (onlineAccount ? 0x40000000u : 0) |
             (drcController ? 0x10000000u : 0)) | (flags & 0x0fffffffu);
}

PlayerUID::PlayerUID(FFLCreateID createId, unsigned int principalId, bool hasPrincipalId, bool drcController)
{
    std::memcpy(mii.createId, createId.data, sizeof(createId.data));
    mii.principalId = hasPrincipalId ? principalId : 0;
    flags = ((((hasPrincipalId ? 1u : 0u) << 1) |
              (drcController ? 1u : 0u)) << 28) | (flags & 0x0fffffffu);
}

PlayerUID::PlayerUID(std::wstring uidString)
{
    if (uidString.length() < 0x1d)
    {
        if (uidString.length() < 0x15)
        {
            for (int i = 0; i < 10; ++i)
                mii.createId[i] = (unsigned char)_fromHEXString<int>(uidString.substr(i * 2, 2));
            mii.principalId = 0;
            flags &= 0x5fffffffu;
        }
        else
        {
            for (int i = 0; i < 10; ++i)
                mii.createId[i] = (unsigned char)_fromHEXString<int>(uidString.substr(i * 2, 2));
            for (int i = 0; i < 4; ++i)
                mii.principalIdBytes[i] = (unsigned char)_fromHEXString<int>(uidString.substr(20 + i * 2, 2));
            flags = (flags & 0x5fffffffu) | 0x20000000u;
        }
    }
    else
    {
        for (int i = 0; i < 16; ++i)
            uuid.data[i] = (unsigned char)_fromHEXString<int>(uidString.substr(i * 2, 2));
        flags = (flags & 0xdfffffffu) | 0x80000000u;
    }
}

std::wstring PlayerUID::UuidString() const
{
    wchar_t text[40];
    swprintf(text, 40, L"%02x%02x%02x%02x%02x%02x%02x%02x",
             uuid.data[0], uuid.data[1], uuid.data[2], uuid.data[3],
             uuid.data[4], uuid.data[5], uuid.data[6], uuid.data[7]);
    return std::wstring(text);
}

std::wstring PlayerUID::toString() const
{
    wchar_t text[40];
    if ((flags & 0x80000000u) != 0)
        return UuidString();
    if ((flags & 0x20000000u) != 0)
    {
        swprintf(text, 40, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                 mii.createId[0], mii.createId[1], mii.createId[2], mii.createId[3],
                 mii.createId[4], mii.createId[5], mii.createId[6], mii.createId[7],
                 mii.createId[8], mii.createId[9], mii.principalIdBytes[0],
                 mii.principalIdBytes[1], mii.principalIdBytes[2], mii.principalIdBytes[3]);
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
    unsigned int uuidType = flags >> 31;
    if (uuidType != (other.flags >> 31))
        return false;
    if (uuidType != 0)
        return uuid == other.uuid;
    return mii.principalId == other.mii.principalId &&
           std::memcmp(this, &other, 10) == 0;
}

bool PlayerUID::operator!=(const PlayerUID& other) const
{
    return !(*this == other);
}

unsigned int hash_value(const PlayerUID& uid)
{
    unsigned int seed = 0;
    if ((uid.flags & 0x80000000u) == 0)
    {
        for (int i = 0; i < 10; ++i)
        {
            seed ^= static_cast<unsigned int>(uid.mii.createId[i]) +
                    seed * 0x40u + (seed >> 2) + 0x9e3779b9u;
        }
        seed ^= uid.mii.principalId +
                seed * 0x40u + (seed >> 2) + 0x9e3779b9u;
    }
    else
    {
        for (int i = 0; i < 16; ++i)
        {
            seed ^= static_cast<unsigned int>(uid.uuid.data[i]) +
                    seed * 0x40u + (seed >> 2) + 0x9e3779b9u;
        }
    }
    return seed;
}
