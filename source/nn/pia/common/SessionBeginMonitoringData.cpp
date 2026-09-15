#include "nn/pia/common/SessionBeginMonitoringData.hpp"
#include "nn/pia/common/MonitoringDataHeader.hpp"
#include "platform/wiiu/pead.hpp"

#include <string.h>

namespace nn { namespace pia { namespace common {

int SessionBeginMonitoringData::GetSerializedSize()
{
    return static_cast<int>(MonitoringContentHeader::GetSerializedSize() + SessionBeginMonitoringContent::GetSerializedSize());
}

void SessionBeginMonitoringData::Initialize()
{
    memset(this, 0xff, MonitoringDataHeader::GetSerializedSize());
    m_dataType = 0x12;
    m_version = 0;
    m_contentSize = static_cast<unsigned short>(GetSerializedSize());
    pead::Random random;
    random.init();
    m_identifier = random.getU64();
    m_environment = static_cast<unsigned char>(random.getU32());
    m_content.Initialize();
}

Result SessionBeginMonitoringData::Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const
{
    if (buffer == 0 || size == 0 || maxSize < static_cast<unsigned int>(GetSerializedSize()))
        return 0x10c07;
    MonitoringDataHeader header;
    memcpy(&header, this, MonitoringDataHeader::GetSerializedSize());
    unsigned int headerSize = 0;
    Result result = header.Serialize(buffer, &headerSize, maxSize);
    if (result != 0)
        return result;
    unsigned int contentSize = 0;
    result = m_content.Serialize(buffer + headerSize, &contentSize, maxSize - headerSize);
    if (result != 0)
        return result;
    *size = headerSize + contentSize;
    return 0;
}

}}}
