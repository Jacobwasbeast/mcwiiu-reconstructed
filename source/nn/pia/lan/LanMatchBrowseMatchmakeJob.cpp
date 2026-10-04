#include "nn/pia/lan/LanMatchBrowseMatchmakeJob.hpp"
#include "nn/pia/lan/LanMatchmakeSession.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/SessionSearchCriteria.hpp"
#include <cstring>

namespace nn { namespace pia { namespace lan {

LanMatchBrowseMatchmakeJob::LanMatchBrowseMatchmakeJob()
{
    std::memset(m_storage, 0, sizeof(m_storage));
}

LanMatchBrowseMatchmakeJob::~LanMatchBrowseMatchmakeJob()
{
}

void LanMatchBrowseMatchmakeJob::Cleanup()
{
    std::memset(m_storage + 0x5c, 0, sizeof(uint32_t));
}

uint32_t LanMatchBrowseMatchmakeJob::BrowseMatchmake()
{
    const uint32_t session = *reinterpret_cast<const uint32_t*>(m_storage + 0x5c);
    if (session == 0)
        return 0x01000000u;
    LanMatchmakeSession* matchmake = reinterpret_cast<LanMatchmakeSession*>(static_cast<uintptr_t>(session));
    const uint64_t result = matchmake->BrowseMatchmakeSession(
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(m_storage + 0x40)),
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(m_storage + 0x48)));
    if (static_cast<uint32_t>(result >> 32) != 0)
        return 0x01000000u;
    const uint32_t next = 0x036154F0u;
    std::memcpy(m_storage + 0x24, &next, sizeof(next));
    const uint32_t state = 0xffffu;
    std::memcpy(m_storage + 0x20, &state, sizeof(state));
    return 0x040001F4u;
}

uint32_t LanMatchBrowseMatchmakeJob::WaitBrowseMatchmake()
{
    uint32_t process = 0;
    std::memcpy(&process, m_storage + 0x48, sizeof(process));
    const uint32_t session = *reinterpret_cast<const uint32_t*>(m_storage + 0x5c);
    if (session != 0)
    {
        LanMatchmakeSession* matchmake = reinterpret_cast<LanMatchmakeSession*>(static_cast<uintptr_t>(session));
        const uint32_t result = matchmake->WaitBrowseMatchmakeSession();
        if (result != 0)
            return 0x01000000u;
    }
    if (process == 3)
        return 0x01000000u;
    if (process == 2)
    {
        const uint32_t state = 0xffffu;
        std::memcpy(m_storage + 0x20, &state, sizeof(state));
        return 0;
    }
    return 0x05000000u;
}

uint32_t LanMatchBrowseMatchmakeJob::WaitRequestSessionInfo()
{
    uint32_t process = 0;
    std::memcpy(&process, m_storage + 0x48, sizeof(process));
    const uint32_t session = *reinterpret_cast<const uint32_t*>(m_storage + 0x5c);
    if (session != 0)
    {
        LanMatchmakeSession* matchmake = reinterpret_cast<LanMatchmakeSession*>(static_cast<uintptr_t>(session));
        const uint32_t result = matchmake->WaitRequestSessionInfo();
        if (result != 0)
            return 0x01000000u;
    }
    if (process == 3)
        return 0x01000000u;
    if (process == 2)
    {
        const uint32_t state = 0xffffu;
        std::memcpy(m_storage + 0x20, &state, sizeof(state));
        return 0;
    }
    return 0x05000000u;
}

void LanMatchBrowseMatchmakeJob::Trace(uint64_t a1) const
{
}

Result LanMatchBrowseMatchmakeJob::StartupImpl(session::IMatchmakeSession* matchmake, const session::SessionSearchCriteria* criteria)
{
    const uint32_t sessionAddress = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(matchmake));
    std::memcpy(m_storage + 0x5c, &sessionAddress, sizeof(sessionAddress));
    LanMatchmakeSession* session = reinterpret_cast<LanMatchmakeSession*>(matchmake);
    const uint32_t registered = session->RegisterSearchCriteria(
        const_cast<session::SessionSearchCriteria*>(criteria), 1);
    if (registered == 0)
        return 0x10c07;
    const uint32_t next = 0x036153A4u;
    std::memcpy(m_storage + 0x24, &next, sizeof(next));
    const uint32_t state = 0xffffu;
    std::memcpy(m_storage + 0x20, &state, sizeof(state));
    return 0;
}

Result LanMatchBrowseMatchmakeJob::StartupImpl(session::IMatchmakeSession* matchmake, unsigned int a2)
{
    const uint32_t sessionAddress = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(matchmake));
    std::memcpy(m_storage + 0x5c, &sessionAddress, sizeof(sessionAddress));
    const uint32_t next = 0x03615670u;
    std::memcpy(m_storage + 0x24, &next, sizeof(next));
    const uint32_t state = 0xffffu;
    std::memcpy(m_storage + 0x20, &state, sizeof(state));
    return 0;
}

}}}
