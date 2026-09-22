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
    ~SessionInfoList() override;

    ISessionInfo* const* Begin() const;
    ISessionInfo** Begin();
    ISessionInfo* const* End() const;
    ISessionInfo** End();
    unsigned short GetSize() const;
    unsigned short GetMaxSize() const;
    void Clear();

private:
    unsigned short m_size;
    unsigned short m_maxSize;
    ISessionInfo** m_items;
};

}}}

#endif
