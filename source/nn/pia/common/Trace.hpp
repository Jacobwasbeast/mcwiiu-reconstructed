#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_TRACE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_TRACE_HPP

#include "nn/pia/common/Time.hpp"

namespace nn { namespace pia { namespace common {

class Trace
{
public:
    Trace();
    static void CreateInstance();
    static void DestroyInstance();
    static void Write(const char* format, ...);
    static void WriteCaption();
    static void WriteCaption(const char* left, const char* right);

private:
    static Trace* s_instance;
    static void* s_storage;
    int m_level;
    int m_enabled;
    Time m_start;
    int m_flags;
    bool m_ready;
};

}}}

#endif
