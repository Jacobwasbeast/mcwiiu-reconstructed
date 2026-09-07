#ifndef VENDOR__CONNECTIONPROGRESSPARAMS_HPP
#define VENDOR__CONNECTIONPROGRESSPARAMS_HPP

#include <stdint.h>

class _ConnectionProgressParams
{
public:
    _ConnectionProgressParams();

private:
    friend class Minecraft;

    int32_t m_connectionId;
    int32_t m_userIndex;
    bool m_isLocal;
    bool m_isJoining;
    int32_t m_progress;
    int32_t m_gameType;
    int32_t m_miniGameType;
    int32_t m_timeout;
};

#endif
