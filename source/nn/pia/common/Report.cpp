#include "nn/pia/common/Report.hpp"

namespace nn { namespace pia { namespace common {

Report::Report()
    : m_first(0xffffffffu), m_time(), m_enabled(1)
{
    m_time.SetNow();
}

Report::~Report()
{
}

void Report::Write(unsigned int category, const char* format, ...)
{
    (void)category;
    (void)format;
}

void Report::WriteCaption(unsigned int category)
{
    (void)category;
}

}}}
