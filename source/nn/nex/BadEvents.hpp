#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_BADEVENTS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_BADEVENTS_HPP

namespace nn
{
namespace nex
{

class BadEvents
{
public:
    enum _ID
    {
        SYSTEM_CHECK_FAILURE = 0
    };

    static void Signal(_ID id);
    void SignalEvent(_ID id);

    static BadEvents* s_pInstance;

private:
    int m_eventCounts[SYSTEM_CHECK_FAILURE + 1];
};

}
}

#endif
