#include "nn/pia/common/SessionStateMonitoringContent.hpp"

#include <algorithm>
#include <string.h>

namespace nn { namespace pia { namespace common {

namespace
{
void WriteMetric(unsigned char*& cursor, unsigned char value)
{
    *cursor++ = value;
}

void WriteMetric(unsigned char*& cursor, unsigned short value)
{
    serializeU16(cursor, value);
    cursor += 2;
}

void WriteMetric(unsigned char*& cursor, unsigned int value)
{
    serializeU32(cursor, value);
    cursor += 4;
}

void WriteMetric(unsigned char*& cursor, unsigned long long value)
{
    serializeU32(cursor, static_cast<unsigned int>(value >> 32));
    serializeU32(cursor + 4, static_cast<unsigned int>(value));
    cursor += 8;
}
}

SessionStateMonitoringContent g_sessionStateMonitoringContent;

void SessionStateMonitoringContent::Initialize()
{
    memset(this, 0xff, sizeof(*this));
    m_contentType = 0x12;
    m_contentVersion = 1;
    m_contentSize = static_cast<unsigned short>(GetSerializedSize());
    m_marker = 0x5069614d;
}

void SessionStateMonitoringContent::Cleanup()
{
    m_schedulerDispatchCount = 0xffffffff;
    m_metric1[2] = 0xffffffff;
    for (unsigned int index = 3; index < 19; ++index)
        m_metric1[index] = 0xffffffff;
    std::fill(m_metric20, m_metric20 + 16, static_cast<unsigned int>(0xffffffff));
    std::fill(m_metric36, m_metric36 + 16, 0xffffffffffffffffULL);
    std::fill(m_metric52, m_metric52 + 16, static_cast<unsigned int>(0xffffffff));
    std::fill(m_metric68, m_metric68 + 16, static_cast<unsigned int>(0xffffffff));
    std::fill(m_metric84, m_metric84 + 16, 0xffffffffffffffffULL);
    std::fill(m_metric100, m_metric100 + 15, static_cast<unsigned int>(0xffffffff));
    std::fill(m_metric115, m_metric115 + 15, static_cast<unsigned char>(0xff));
    std::fill(m_metric130, m_metric130 + 15, static_cast<unsigned short>(0xffff));
    std::fill(m_metric145, m_metric145 + 15, static_cast<unsigned short>(0xffff));

    m_sendThreadSleepMilliSecond = 0xff;
    m_metric163 = 0xffffffff;
    m_metric165 = 0xffffffff;
    m_receiveThreadSleepMilliSecond = 0xff;
    m_metric162 = 0xffff;
    m_metric164 = 0xffff;
    for (unsigned int index = 1; index < 5; ++index)
        m_metric167[index] = 0xffffffff;
    for (unsigned int index = 0; index < 4; ++index)
        m_metric172[index] = 0xffffffff;
    m_metric185 = 0xffff;
    m_metric184 = 0xffff;
    m_metric187[3] = 0xff;
    m_metric198 = 0xffffffff;
    m_metric195 = 0xffffffff;
    m_metric179 = 0xffffffff;
    m_metric181 = 0xff;
    m_metric225[1] = 0xff;
    m_metric196 = 0xff;
    m_metric187[1] = 0xff;
    m_metric187[0] = 0xff;
    m_metric197 = 0xff;
    m_metric177 = 0xffffffff;
    m_metric199 = 0xffffffff;
    m_metric193 = 0xffffffff;
    m_metric222 = 0xffffffff;
    m_metric187[2] = 0xff;
    m_metric225[0] = 0xff;
    m_metric180 = 0xff;
    m_metric186 = 0xffffffff;
    m_metric224 = 0xffffffff;
    m_metric178 = 0xff;
    m_metric182 = 0xffffffff;
    m_metric187[5] = 0xff;
    m_metric187[4] = 0xff;
    m_metric223[0] = 0xff;
    m_metric223[1] = 0xff;
    m_metric223[2] = 0xff;
    m_metric225[2] = 0xff;
    m_metric194 = 0xff;
    m_metric176 = 0xffffffff;
    m_metric183 = 0xffffffff;
    for (unsigned int index = 0; index < 4; ++index)
        m_metric200[index] = 0xff;
    for (unsigned int index = 0; index < 5; ++index)
        m_metric201[index] = 0xff;
    m_metric203 = 0xffff;
    m_metric202 = 0xffffffff;
    m_metric206 = 0xffffffff;
    m_metric205 = 0xffffffff;
}

unsigned int SessionStateMonitoringContent::GetSerializedSize()
{
    return MonitoringContentHeader::GetSerializedSize() + 883;
}

Result SessionStateMonitoringContent::Serialize(unsigned char* buffer, unsigned int* size,
                                               unsigned int maxSize) const
{
    if (buffer == 0 || size == 0 || maxSize < GetSerializedSize())
        return 0x10c07;

    unsigned int headerSize;
    const Result result = MonitoringContentHeader::Serialize(buffer, &headerSize, maxSize);
    if (result != 0)
        return result;
    unsigned char* cursor = buffer + headerSize;

    WriteMetric(cursor, m_schedulerDispatchCount);
    for (unsigned int index = 0; index < 19; ++index)
        WriteMetric(cursor, m_metric1[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric20[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric36[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric52[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric68[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric84[index]);
    for (unsigned int index = 0; index < 15; ++index)
        WriteMetric(cursor, m_metric100[index]);
    for (unsigned int index = 0; index < 15; ++index)
        WriteMetric(cursor, m_metric115[index]);
    for (unsigned int index = 0; index < 15; ++index)
        WriteMetric(cursor, m_metric130[index]);
    for (unsigned int index = 0; index < 15; ++index)
        WriteMetric(cursor, m_metric145[index]);

    WriteMetric(cursor, m_sendThreadSleepMilliSecond);
    WriteMetric(cursor, m_receiveThreadSleepMilliSecond);
    WriteMetric(cursor, m_metric162);
    WriteMetric(cursor, m_metric163);
    WriteMetric(cursor, m_metric164);
    WriteMetric(cursor, m_metric165);
    WriteMetric(cursor, m_receiveThreadMetric);
    for (unsigned int index = 0; index < 5; ++index)
        WriteMetric(cursor, m_metric167[index]);
    for (unsigned int index = 0; index < 4; ++index)
        WriteMetric(cursor, m_metric172[index]);
    WriteMetric(cursor, m_metric176);
    WriteMetric(cursor, m_metric177);
    WriteMetric(cursor, m_metric178);
    WriteMetric(cursor, m_metric179);
    WriteMetric(cursor, m_metric180);
    WriteMetric(cursor, m_metric181);
    WriteMetric(cursor, m_metric182);
    WriteMetric(cursor, m_metric183);
    WriteMetric(cursor, m_metric184);
    WriteMetric(cursor, m_metric185);
    WriteMetric(cursor, m_metric186);
    for (unsigned int index = 0; index < 6; ++index)
        WriteMetric(cursor, m_metric187[index]);
    WriteMetric(cursor, m_metric193);
    WriteMetric(cursor, m_metric194);
    WriteMetric(cursor, m_metric195);
    WriteMetric(cursor, m_metric196);
    WriteMetric(cursor, m_metric197);
    WriteMetric(cursor, m_metric198);
    WriteMetric(cursor, m_metric199);
    for (unsigned int index = 0; index < 4; ++index)
        WriteMetric(cursor, m_metric200[index]);
    for (unsigned int index = 0; index < 5; ++index)
        WriteMetric(cursor, m_metric201[index]);
    WriteMetric(cursor, m_metric202);
    WriteMetric(cursor, m_metric203);
    WriteMetric(cursor, m_metric204);
    WriteMetric(cursor, m_metric205);
    WriteMetric(cursor, m_metric206);
    WriteMetric(cursor, m_metric207);
    WriteMetric(cursor, m_metric208);
    WriteMetric(cursor, m_metric209);
    WriteMetric(cursor, m_metric210);
    WriteMetric(cursor, m_metric211);
    WriteMetric(cursor, m_metric212);
    WriteMetric(cursor, m_metric213);
    WriteMetric(cursor, m_metric214);
    WriteMetric(cursor, m_metric215);
    WriteMetric(cursor, m_metric216);
    WriteMetric(cursor, m_metric217);
    WriteMetric(cursor, m_metric218);
    WriteMetric(cursor, m_metric219);
    WriteMetric(cursor, m_metric220);
    for (unsigned int index = 0; index < 4; ++index)
        WriteMetric(cursor, m_metric221[index]);
    WriteMetric(cursor, m_metric222);
    for (unsigned int index = 0; index < 3; ++index)
        WriteMetric(cursor, m_metric223[index]);
    WriteMetric(cursor, m_metric224);
    for (unsigned int index = 0; index < 3; ++index)
        WriteMetric(cursor, m_metric225[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric226[index]);
    WriteMetric(cursor, m_marker);

    *size = static_cast<unsigned int>(cursor - buffer);
    return 0;
}

}}}
