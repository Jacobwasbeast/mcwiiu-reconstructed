#include "nn/pia/lan/LanMatchCreateSessionJob.hpp"
#include "nn/pia/session/CreateSessionSetting.hpp"
#include <cstring>

namespace nn { namespace pia { namespace lan {

LanMatchCreateSessionJob::LanMatchCreateSessionJob()
{
    std::memset(m_storage, 0, sizeof(m_storage));
}

LanMatchCreateSessionJob::~LanMatchCreateSessionJob()
{
}

uint32_t LanMatchCreateSessionJob::CreateMatchmakeSession()
{
    const uint32_t session = *reinterpret_cast<const uint32_t*>(m_storage + 0x64);
    if (session == 0)
        return 0x01000000u;
    const uint32_t step = 0x03615C1Cu;
    std::memcpy(m_storage + 0x24, &step, sizeof(step));
    return 0x05000000u;
}

uint32_t LanMatchCreateSessionJob::WaitCreateMatchmake()
{
    uint32_t status = 0;
    std::memcpy(&status, m_storage + 0x44, sizeof(status));
    if (status == 2)
        return 0;
    if (status == 3)
        return 0x01000000u;
    return 0x05000000u;
}

uint32_t LanMatchCreateSessionJob::OpenParticipate()
{
    const uint32_t step = 0x0361600Cu;
    std::memcpy(m_storage + 0x24, &step, sizeof(step));
    return 0x05000000u;
}

uint32_t LanMatchCreateSessionJob::WaitOpenParticipate()
{
    uint32_t status = 0;
    std::memcpy(&status, m_storage + 0x44, sizeof(status));
    if (status == 2)
        return 0;
    if (status == 3)
        return 0x01000000u;
    return 0x05000000u;
}

uint32_t LanMatchCreateSessionJob::UnregisterGathering()
{
    const uint32_t step = 0x036162D8u;
    std::memcpy(m_storage + 0x24, &step, sizeof(step));
    return 0x05000000u;
}

uint32_t LanMatchCreateSessionJob::WaitUnregisterGathering()
{
    uint32_t status = 0;
    std::memcpy(&status, m_storage + 0x44, sizeof(status));
    if (status == 0)
        return 0x054D0000u;
    return 0x014D0000u;
}

void LanMatchCreateSessionJob::Cleanup()
{
    std::memset(m_storage + 0x64, 0, sizeof(uint32_t));
}

void LanMatchCreateSessionJob::Trace(uint64_t a1) const
{
}

Result LanMatchCreateSessionJob::StartupImpl(const session::CreateSessionSetting* setting)
{
    if (setting->GetMaxParticipantNum() == 0)
        return 0x10c07;
    const uint32_t step = 0x03615C1Cu;
    std::memcpy(m_storage + 0x24, &step, sizeof(step));
    const uint32_t state = 0xffffu;
    std::memcpy(m_storage + 0x20, &state, sizeof(state));
    return 0;
}

void LanMatchCreateSessionJob::SetNextStepAtWaitCreateMesh()
{
    const uint32_t step = 0x036165BCu;
    std::memcpy(m_storage + 0x24, &step, sizeof(step));
}

uint32_t LanMatchCreateSessionJob::SetNextStepAtFailedCreateMesh(Result result)
{
    const uint32_t code = result.m_code;
    std::memcpy(m_storage + 0x60, &code, sizeof(code));
    const uint32_t step = 0x036162D8u;
    std::memcpy(m_storage + 0x24, &step, sizeof(step));
    const uint32_t state = 0xffffu;
    std::memcpy(m_storage + 0x20, &state, sizeof(state));
    return 0x05000000u;
}

}}}
