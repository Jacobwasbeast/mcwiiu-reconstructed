#include "nn/pia/transport/ReliableSlidingWindow.hpp"
#include <string.h>

namespace nn { namespace pia { namespace transport {

unsigned short g_reliableSlidingWindowGeneration = 0;

void ReliableSlidingWindow::Cleanup()
{
    if (m_field00 == g_reliableSlidingWindowGeneration)
        return;
    m_field00 = g_reliableSlidingWindowGeneration;
    m_field40 = 0;
    m_stationId = 0;
    memset(m_field08, 0, sizeof(m_field08));
}

bool ReliableSlidingWindow::IsInCommunication() const
{
    return m_field00 != g_reliableSlidingWindowGeneration;
}

}}}
