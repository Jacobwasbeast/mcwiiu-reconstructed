#include "vendor/_ConnectionProgressParams.hpp"

_ConnectionProgressParams::_ConnectionProgressParams()
{
    m_connectionId = 0;
    m_userIndex = -1;
    m_isLocal = false;
    m_isJoining = false;
    m_progress = 0;
    m_gameType = 0;
    m_miniGameType = 0;
    m_timeout = 15;
}
