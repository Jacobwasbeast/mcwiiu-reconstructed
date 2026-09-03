#include "game/minigame/TutorialMessage.hpp"
#include "game/core/L10N.hpp"

TutorialMessage::TutorialMessage(int messageId, bool repeatMessage, unsigned char maxDisplayCount)
{
    m_messageId = messageId;
    m_maxDisplayCount = maxDisplayCount;
    m_displayCount = 0;
    m_repeatMessage = repeatMessage;
}

bool TutorialMessage::canDisplay()
{
    if (m_repeatMessage && m_maxDisplayCount <= m_displayCount)
        return false;
    return true;
}

const wchar_t* TutorialMessage::getMessageForDisplay()
{
    if (!canDisplay())
        return L"";
    if (m_repeatMessage)
        ++m_displayCount;
    return L10N::GetString(m_messageId);
}
