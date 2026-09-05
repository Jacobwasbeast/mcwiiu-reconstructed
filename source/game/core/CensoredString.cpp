#include "game/core/CensoredString.hpp"

#include <cwchar>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/L10N.hpp"

static const std::wstring& EmptyCensoredString()
{
    static const std::wstring value;
    return value;
}

CensoredString::CensoredString()
    : m_colour(), m_useLocalization(false), m_flags(0), m_pendingId(0), m_censoredId(0),
      m_pending(), m_censored(), m_tickAction(NULL), m_uuid(), m_chatCensored(false), m_uuidLocal(true)
{
    _init(NULL, -1, -1);
}

CensoredString::CensoredString(int firstId, int secondId)
    : m_colour(), m_useLocalization(false), m_flags(0), m_pendingId(0), m_censoredId(0),
      m_pending(), m_censored(), m_tickAction(NULL), m_uuid(), m_chatCensored(false), m_uuidLocal(true)
{
    _init(NULL, firstId, secondId);
}

CensoredString::CensoredString(TickAction* action, int firstId, int secondId)
    : m_colour(), m_useLocalization(false), m_flags(0), m_pendingId(0), m_censoredId(0),
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
    m_censored = L"Custom";
    m_flags &= static_cast<unsigned char>(0xf8);
    if (!m_useLocalization)
    {
        m_pending = L"Custom";
        m_pendingId = -1;
        m_censoredId = -1;
        m_uuid = L"Custom";
        m_chatCensored = false;
        m_uuidLocal = true;
    }
    else
    {
        const wchar_t* pending = L10N::GetString(firstId);
        m_pending.assign(pending, wcslen(pending));
        m_pendingId = firstId;
        m_censoredId = secondId;
        m_uuid = L"Custom";
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
    bool censored = false;
    if (g_pApp != NULL && !g_pApp->IsUUIDLocal((int*)&m_uuid) &&
        (g_pApp->IsConsoleChatCensored() || m_chatCensored))
        censored = true;
    bool blocked = !m_uuidLocal && censored;
    return (!blocked || !censored) ? 1 : 0;
}

std::wstring CensoredString::raw()
{
    if (m_useLocalization && (m_flags & 1) != 0)
        return m_censored;
    return m_pending;
}

void CensoredString::saveToTag(CompoundTag* tag)
{
    static const std::wstring censoredKey;
    static const std::wstring chatKey;
    static const std::wstring uuidKey;
    static const std::wstring customName(L"CustomName");
    tag->putBoolean(censoredKey, m_uuidLocal);
    tag->putBoolean(chatKey, m_chatCensored);
    tag->putString(uuidKey, m_uuid);
    if (!raw().empty())
        tag->putString(customName, raw());
}

void CensoredString::loadFromTag(CompoundTag* tag)
{
    static const std::wstring censoredKey;
    static const std::wstring chatKey;
    static const std::wstring uuidKey;
    static const std::wstring customName(L"CustomName");
    if (tag->contains(censoredKey, 1))
        m_uuidLocal = tag->getBoolean(censoredKey);
    if (tag->contains(chatKey, 1))
        m_chatCensored = tag->getBoolean(chatKey);
    if (tag->contains(uuidKey, 8))
        m_uuid = tag->getString(uuidKey);
    if (tag->contains(customName, 8))
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
