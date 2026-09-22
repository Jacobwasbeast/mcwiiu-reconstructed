#include "nn/pia/common/SessionEndMonitoringData.hpp"
#include "nn/pia/common/MonitoringDataHeader.hpp"
#include "platform/wiiu/pead.hpp"

#include <string.h>

namespace nn { namespace pia { namespace common {

int SessionEndMonitoringData::GetSerializedSize()
{
    return static_cast<int>(MonitoringDataHeader::GetSerializedSize() +
                            SessionBeginMonitoringContent::GetSerializedSize() +
                            SessionStateMonitoringContent::GetSerializedSize());
}

void SessionEndMonitoringData::Initialize(unsigned char reason)
{
    memset(this, 0xff, MonitoringDataHeader::GetSerializedSize());
    m_dataType = 0x12;
    m_reason = reason;
    m_contentSize = static_cast<unsigned short>(GetSerializedSize());
    pead::Random random;
    random.init();
    m_identifier = random.getU64();
    m_environment = static_cast<unsigned char>(random.getU32());
    m_beginContent.Initialize();
    m_stateContent.Initialize();
}

Result SessionEndMonitoringData::Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const
{
    if (buffer == 0 || size == 0 || maxSize < static_cast<unsigned int>(GetSerializedSize()))
        return 0x10c07;
    MonitoringDataHeader header;
    memcpy(&header, this, MonitoringDataHeader::GetSerializedSize());
    unsigned int headerSize = 0;
    Result result = header.Serialize(buffer, &headerSize, maxSize);
    if (result != 0)
        return result;
    unsigned int written = 0;
    result = m_beginContent.Serialize(buffer + headerSize, &written, maxSize - headerSize);
    if (result != 0)
        return result;
    unsigned int stateWritten = 0;
    result = m_stateContent.Serialize(buffer + headerSize + written, &stateWritten,
                                      maxSize - headerSize - written);
    if (result != 0)
        return result;
    *size = headerSize + written + stateWritten;
    return 0;
}

}}}
