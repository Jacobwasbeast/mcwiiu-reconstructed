#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STREAMBUNDLING_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STREAMBUNDLING_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class StreamBundling : public RootObject
{
public:
    StreamBundling();
    virtual ~StreamBundling();

    void Enable(int iMaxFlushDelay);
    void Flush();
    void SetMaxFlushDelay(int iMaxFlushDelay);
    void UpdateFlushTime();
    Time GetLastFlushTime();

    bool IsEnabled() const
    {
        return m_bEnabled;
    }

private:
    bool m_bEnabled;
    int m_iMaxFlushDelay;
    Time m_lastFlushTime;
    unsigned short m_unknown10;
    unsigned short m_unknown12;
    CriticalSection m_criticalSection;
};

}
}

#endif
