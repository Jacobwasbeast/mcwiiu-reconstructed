#include "nn/pia/session/lan.hpp"

#include "nn/pia/session/SessionInfo.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace session {

SessionInfoList<lan::LanSessionInfoPrivate>::~SessionInfoList()
{
    if (m_items != 0)
    {
        for (unsigned short i = 0; i < m_maxSize; ++i)
        {
            if (m_items[i] != 0)
            {
                m_items[i]->~ISessionInfo();
                pead::freeToSeadHeap(m_items[i]);
            }
        }
        pead::getArraySize(m_items);
        pead::freeToSeadHeap(m_items);
    }
}

ISessionInfo* const* SessionInfoList<lan::LanSessionInfoPrivate>::Begin() const
{
    return m_items;
}

ISessionInfo** SessionInfoList<lan::LanSessionInfoPrivate>::Begin()
{
    return m_items;
}

ISessionInfo* const* SessionInfoList<lan::LanSessionInfoPrivate>::End() const
{
    return m_items + m_size;
}

ISessionInfo** SessionInfoList<lan::LanSessionInfoPrivate>::End()
{
    return m_items + m_size;
}

unsigned short SessionInfoList<lan::LanSessionInfoPrivate>::GetSize() const
{
    return m_size;
}

unsigned short SessionInfoList<lan::LanSessionInfoPrivate>::GetMaxSize() const
{
    return m_maxSize;
}

void SessionInfoList<lan::LanSessionInfoPrivate>::Clear()
{
    for (unsigned short i = 0; i < m_maxSize; ++i)
    {
        if (m_items != 0 && m_items[i] != 0)
        {
            m_items[i]->Clear();
        }
    }
    m_size = 0;
}

}}}
