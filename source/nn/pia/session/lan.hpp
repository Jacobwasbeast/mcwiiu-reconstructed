#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSIONINFOLIST_LAN_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSIONINFOLIST_LAN_HPP

#include <cstddef>

#include "nn/pia/session/ISessionInfoList.hpp"

namespace nn { namespace pia { namespace lan { class LanSessionInfoPrivate; } namespace session {

class ISessionInfo;
template <typename T>
class SessionInfoList;

template <>
class SessionInfoList<lan::LanSessionInfoPrivate> : public ISessionInfoList
{
public:
    virtual ~SessionInfoList();
    virtual ISessionInfo* const* Begin() const;
    virtual ISessionInfo** Begin();
    virtual ISessionInfo* const* End() const;
    virtual ISessionInfo** End();
    virtual unsigned short GetSize() const;
    virtual unsigned short GetMaxSize() const;
    virtual void Clear();

    unsigned short m_size;
    unsigned short m_maxSize;
    ISessionInfo** m_items;
};

}}}

#endif
