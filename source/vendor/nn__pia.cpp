#include "nn__pia.hpp"

extern void SetModuleId(const char*);

namespace nn {
namespace pia {

namespace {
unsigned int g_nexNetworkErrorCode = 0;
unsigned int g_nexNetworkErrorCodeSecondary = 0;
}

void Assert(const char*, const char*, unsigned int, const char*, ...) {}


void EmbedMiddlewareInfo()
{
    SetModuleId("[SDK+Nintendo:PIA_5_9_0]");
}

void Result::SetNexNetworkErrorCode(unsigned int code)
{
    g_nexNetworkErrorCode = code;
}

void Result::ClearNexNetworkErrorCode()
{
    g_nexNetworkErrorCode = 0;
}

int Result::GetErrorCode() const
{
    if (m_code == 0) {
        return 0;
    }
    if (m_code == 0xe437 && g_nexNetworkErrorCode != 0) {
        return static_cast<int>(g_nexNetworkErrorCode);
    }
    if (m_code == 0xe46c && g_nexNetworkErrorCodeSecondary != 0) {
        return static_cast<int>(g_nexNetworkErrorCodeSecondary);
    }
    return 0x120160;
}

}
}
