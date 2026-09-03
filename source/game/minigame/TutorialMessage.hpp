#ifndef GAME_MINIGAME_TUTORIALMESSAGE_HPP
#define GAME_MINIGAME_TUTORIALMESSAGE_HPP

class TutorialMessage
{
public:
    TutorialMessage(int messageId, bool repeatMessage, unsigned char maxDisplayCount);

    bool canDisplay();
    const wchar_t* getMessageForDisplay();

private:
    int m_messageId;
    bool m_repeatMessage;
    unsigned char m_maxDisplayCount;
    unsigned char m_displayCount;
};

#endif
