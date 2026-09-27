#include "vendor/_ConnectionProgressParams.hpp"

#include <cstddef>

_ConnectionProgressParams::_ConnectionProgressParams()
{
    m_cancelFunc = NULL;
    m_miniGameId = eMiniGameId_Any;
    m_cancelFuncParam = NULL;
    m_setFailTimer = false;
    m_showTooltips = false;
    m_playerId = 0;
    m_timerTime = 0;
    m_stringId = -1;
}
