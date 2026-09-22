#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SESSION_END_MONITORING_DATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SESSION_END_MONITORING_DATA_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"

namespace nn { namespace pia { namespace common {

class SessionEndMonitoringData
{
public:
    static int GetSerializedSize();
    void Initialize(unsigned char reason);
    Result Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const;
    Result Deserialize(const unsigned char* buffer, unsigned int size);
    void Cleanup();
    void Trace(unsigned long long value) const;

    unsigned char m_dataType;
    unsigned char m_reason;
    unsigned short m_reserved;
    unsigned short m_contentSize;
    unsigned long long m_identifier;
    unsigned char m_environment;
    unsigned char m_reserved2;
    SessionBeginMonitoringContent m_beginContent;
    SessionStateMonitoringContent m_stateContent;
};

}}}

#endif
