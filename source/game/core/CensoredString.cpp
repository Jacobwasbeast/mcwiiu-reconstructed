#include "game/core/CensoredString.hpp"

#include <cwchar>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/core/L10N.hpp"

static const std::wstring& EmptyCensoredString()
{
    static const std::wstring value;
    return value;
}

CensoredString::CensoredString()
    : boost::enable_shared_from_this<CensoredString>(), m_useLocalization(false), m_flags(0), m_pendingId(0), m_censoredId(0),
      m_pending(), m_censored(), m_tickAction(NULL), m_uuid(), m_chatCensored(false), m_uuidLocal(true)
{
    _init(NULL, -1, -1);
}

CensoredString::CensoredString(int firstId, int secondId)
    : boost::enable_shared_from_this<CensoredString>(), m_useLocalization(false), m_flags(0), m_pendingId(0), m_censoredId(0),
      m_pending(), m_censored(), m_tickAction(NULL), m_uuid(), m_chatCensored(false), m_uuidLocal(true)
{
    _init(NULL, firstId, secondId);
}

CensoredString::CensoredString(TickAction* action, int firstId, int secondId)
    : boost::enable_shared_from_this<CensoredString>(), m_useLocalization(false), m_flags(0), m_pendingId(0), m_censoredId(0),
      m_pending(), m_censored(), m_tickAction(NULL), m_uuid(), m_chatCensored(false), m_uuidLocal(true)
{
    _init(action, firstId, secondId);
}

CensoredString::~CensoredString()
{
}

void CensoredString::_init(TickAction* action, int firstId, int secondId)
{
    m_tickAction = action;
    const std::wstring& empty = EmptyCensoredString();
    m_censored = empty;
    m_flags &= static_cast<unsigned char>(0xf8);
    if (!m_useLocalization)
    {
        m_pending = empty;
        m_pendingId = -1;
        m_censoredId = -1;
        m_uuid = empty;
        m_chatCensored = false;
        m_uuidLocal = true;
    }
    else
    {
        const wchar_t* pending = L10N::GetString(firstId);
        m_pending.assign(pending, wcslen(pending));
        m_pendingId = firstId;
        m_censoredId = secondId;
        m_uuid = empty;
        m_chatCensored = false;
        m_uuidLocal = true;
    }
}

std::wstring CensoredString::getPendingString()
{
    if (m_pendingId == -1)
        return EmptyCensoredString();
    return std::wstring(L10N::GetString(m_pendingId));
}

std::wstring CensoredString::getCensoredString()
{
    if (m_censoredId == -1)
        return EmptyCensoredString();
    return std::wstring(L10N::GetString(m_censoredId));
}

uint64_t CensoredString::_verified(const std::wstring& value, bool censored)
{
    m_censored = L"Custom";
    m_flags = static_cast<unsigned char>((m_flags & 0xfe) | 4);
    m_pending = value;
    if (censored)
        m_flags = static_cast<unsigned char>(m_flags | 2);
    else
        m_flags = static_cast<unsigned char>(m_flags & 0xfd);
    return 0;
}

uint64_t CensoredString::_changed()
{
    if (m_tickAction != NULL && (m_flags & 4) != 0)
        m_tickAction->Execute();
    m_flags = static_cast<unsigned char>(m_flags & 0xfb);
    return 0;
}

void CensoredString::set(const std::wstring& value, const std::wstring& uuid, bool chatCensored, bool uuidLocal)
{
    m_uuid = uuid;
    m_chatCensored = chatCensored;
    m_uuidLocal = uuidLocal;
    if (!m_useLocalization)
    {
        _verified(value, false);
        return;
    }
    if ((m_flags & 1) != 0 && m_censored == value)
        return;
    if (value.empty())
    {
        _verified(L"Custom", false);
        _changed();
        return;
    }
    if (m_pending == value && (m_flags & 1) != 0)
    {
        _verified(m_pending, (m_flags & 2) != 0);
        _changed();
        return;
    }
    _changed();
}

uint32_t CensoredString::checkParentalFilter()
{
    bool censored = !gApp.IsUUIDLocal(m_uuid) && (gApp.IsConsoleChatCensored() || m_chatCensored);
    bool blocked = !m_uuidLocal && censored;
    if (CGameNetworkManager::sInstance.IsHost() && blocked)
        return 0;
    if (!CGameNetworkManager::sInstance.IsHost() && censored)
        return 0;
    return 1;
}

std::wstring CensoredString::raw()
{
    const bool useCensored = m_useLocalization && (m_flags & 1) != 0;
    if (useCensored)
    {
        return std::wstring(m_censored);
    }
    return std::wstring(m_pending);
}

void CensoredString::saveToTag(CompoundTag* tag)
{
    static const std::wstring censoredKey(L"CScreatedOnHost");
    static const std::wstring chatKey(L"CSnamedByRestricted");
    static const std::wstring uuidKey(L"CSownerUUID");
    static const std::wstring customName(L"CustomName");
    tag->putBoolean(censoredKey, m_uuidLocal);
    tag->putBoolean(chatKey, m_chatCensored);
    tag->putString(uuidKey, m_uuid);
    if (!raw().empty())
        tag->putString(customName, raw());
}

void CensoredString::loadFromTag(CompoundTag* tag)
{
    static const std::wstring censoredKey(L"CScreatedOnHost");
    static const std::wstring chatKey(L"CSnamedByRestricted");
    static const std::wstring uuidKey(L"CSownerUUID");
    static const std::wstring customName(L"CustomName");
    if (tag->contains(censoredKey, Tag::TAG_BYTE))
        m_uuidLocal = tag->getBoolean(censoredKey);
    if (tag->contains(chatKey, Tag::TAG_BYTE))
        m_chatCensored = tag->getBoolean(chatKey);
    if (tag->contains(uuidKey, Tag::TAG_STRING))
        m_uuid = tag->getString(uuidKey);
    if (tag->contains(customName, Tag::TAG_STRING))
        set(tag->getString(customName), m_uuid, m_chatCensored, m_uuidLocal);
}

std::wstring CensoredString::get()
{
    if (m_useLocalization && (m_flags & 1) != 0)
        return getPendingString();
    if (m_useLocalization && (m_flags & 2) != 0)
        return getCensoredString();
    return m_pending;
}
