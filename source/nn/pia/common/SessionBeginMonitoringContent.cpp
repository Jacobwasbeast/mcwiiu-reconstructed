#include "nn/pia/common/SessionBeginMonitoringContent.hpp"

#include <algorithm>
#include <coreinit/memory.h>
#include <nn/act/client_cpp.h>

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

SessionBeginMonitoringContent g_sessionBeginMonitoringContent;

void SessionBeginMonitoringContent::Initialize()
{
    OSBlockSet(this, 0xff, sizeof(*this));
    m_contentType = 0x12;
    m_contentVersion = 0;
    m_contentSize = static_cast<unsigned short>(GetSerializedSize());
    m_environment = 0;
    m_piaVersion = 0x050900ff;
    m_platformRevision = 3;
    m_platform = 3;
    nn::act::Initialize();
    const unsigned int addressId = nn::act::GetSimpleAddressId();
    if (addressId != 0)
    {
        m_country = static_cast<unsigned char>(addressId >> 24);
        m_region = static_cast<unsigned char>(addressId >> 16);
    }
    nn::act::Finalize();
    m_integrityMarker = 0x5069614d;
}

void SessionBeginMonitoringContent::SetSdkVersion(unsigned int version)
{
    m_sdkVersion = version;
}

void SessionBeginMonitoringContent::SetNexVersion(unsigned char major, unsigned char minor,
                                                 unsigned char patch)
{
    m_nexVersion = (static_cast<unsigned int>(major) << 24) |
                   (static_cast<unsigned int>(minor) << 16) |
                   (static_cast<unsigned int>(patch) << 8) | 0xff;
}

void SessionBeginMonitoringContent::Cleanup()
{
    m_metric37 = 0xffff;
    m_metric38 = 0xffffffff;
    m_metric39 = 0xffff;
    m_metric40 = 0xffffffff;
    m_metric45 = 0xff;
    m_metric46 = 0xff;
    m_metric47 = 0xff;
    m_joinResult = 0xffffffff;
    m_joinProcessingTime = 0xffffffff;
    m_metric50 = 0xff;
    std::fill(m_metric51, m_metric51 + 15, static_cast<unsigned int>(0xffffffff));
    m_metric52 = 0xff;
    std::fill(m_metric53, m_metric53 + 8, static_cast<unsigned int>(0xffffffff));
    m_metric54 = 0xffff;
    m_metric80 = 0xffffffff;
    m_metric81 = 0xffffffff;
    std::fill(m_metric82, m_metric82 + 6, static_cast<unsigned int>(0xffffffff));
    m_metric83 = 0xffff;
    m_metric84 = 0xffffffff;
    m_metric85 = 0xff;
    m_metric86 = 0xff;
    m_metric87 = 0xff;
    m_metric88 = 0xff;
    std::fill(m_metric89, m_metric89 + 6, static_cast<unsigned char>(0xff));
    std::fill(m_metric90, m_metric90 + 6, static_cast<unsigned int>(0xff));
    std::fill(m_metric91, m_metric91 + 6, static_cast<unsigned int>(0xff));
    m_metric92 = 0xff;
    m_metric94 = 0xff;
    m_metric95 = 0xffffffff;
    m_metric96 = 0xff;
    m_metric97 = 0xff;
    m_metric98 = 0xff;
    m_metric99 = 0xff;
    std::fill(m_metric100, m_metric100 + 6, static_cast<unsigned char>(0xff));
    std::fill(m_metric101, m_metric101 + 6, static_cast<unsigned int>(0xff));
    std::fill(m_metric102, m_metric102 + 6, static_cast<unsigned int>(0xff));
    m_metric103 = 0xff;
    m_metric105 = 0xff;
    m_metric107 = 0xff;
    m_metric108 = 0xff;
    m_metric109 = 0xffffffff;
    m_metric110 = 0xffffffff;
    m_metric111 = 0xffffffff;
    m_metric112 = 0xff;
    m_metric113 = 0xff;
    m_metric114 = 0xffffffff;
    m_metric115 = 0xffffffff;
    m_metric116 = 0xffffffff;
    m_metric117 = 0xff;
    m_metric118 = 0xff;
    m_joinTargetIdHash = 0xffffffff;
    m_metric126 = 0xffffffff;
    m_metric133 = 0xffff;
    m_metric134 = 0xffff;
    m_metric135 = 0xffffffff;
    m_metric136 = 0xffffffff;
    std::fill(m_metric139, m_metric139 + 15, static_cast<unsigned int>(0xffffffff));
    std::fill(m_metric140, m_metric140 + 15, static_cast<unsigned int>(0xffffffff));
    std::fill(m_metric141, m_metric141 + 15, static_cast<unsigned int>(0xffffffff));
    std::fill(m_metric142, m_metric142 + 15, static_cast<unsigned int>(0xffffffff));
    std::fill(m_metric143, m_metric143 + 15, static_cast<unsigned int>(0xff));
    std::fill(m_metric144, m_metric144 + 15, static_cast<unsigned char>(0xff));
    m_metric148 = 0xff;
    m_metric149 = 0xff;
    m_metric150 = 0xffff;
    m_metric151 = 0xffff;
    m_metric152 = 0xffff;
    std::fill(m_metric161, m_metric161 + 16, static_cast<unsigned char>(0xff));
    std::fill(m_metric162, m_metric162 + 16, static_cast<unsigned char>(0xff));
    std::fill(m_metric163, m_metric163 + 16, static_cast<unsigned char>(0xff));
    std::fill(m_metric164, m_metric164 + 16, static_cast<unsigned char>(0xff));
    std::fill(m_metric165, m_metric165 + 150, static_cast<unsigned char>(0xff));
    std::fill(m_metric166, m_metric166 + 150, static_cast<unsigned char>(0xff));
    std::fill(m_metric167, m_metric167 + 16, static_cast<unsigned char>(0xff));
    std::fill(m_metric168, m_metric168 + 16, static_cast<unsigned char>(0xff));
    std::fill(m_metric169, m_metric169 + 150, static_cast<unsigned char>(0xff));
    std::fill(m_metric170, m_metric170 + 16, static_cast<unsigned char>(0xff));
    std::fill(m_metric171, m_metric171 + 16, static_cast<unsigned char>(0xff));
    std::fill(m_metric172, m_metric172 + 16, static_cast<unsigned char>(0xff));
    std::fill(m_metric173, m_metric173 + 16, static_cast<unsigned char>(0xff));
    std::fill(m_metric174, m_metric174 + 16, static_cast<unsigned char>(0xff));
    std::fill(m_metric175, m_metric175 + 16, static_cast<unsigned char>(0xff));
    std::fill(m_metric176, m_metric176 + 16, static_cast<unsigned char>(0xff));
    std::fill(m_metric177, m_metric177 + 16, static_cast<unsigned char>(0xff));
    std::fill(m_metric178, m_metric178 + 16, static_cast<unsigned char>(0xff));
    std::fill(m_metric179, m_metric179 + 16, static_cast<unsigned char>(0xff));
    std::fill(m_metric180, m_metric180 + 16, static_cast<unsigned char>(0xff));
    m_metric181 = 0xffffffff;
    m_metric182 = 0xffffffff;
    std::fill(m_metric183, m_metric183 + 300, static_cast<unsigned char>(0xff));
}

unsigned int SessionBeginMonitoringContent::GetSerializedSize()
{
    return MonitoringContentHeader::GetSerializedSize() + 0x79e;
}

Result SessionBeginMonitoringContent::Serialize(unsigned char* buffer, unsigned int* size,
                                               unsigned int maxSize) const
{
    if (buffer == 0 || size == 0 || maxSize < GetSerializedSize())
        return 0x10c07;

    unsigned int headerSize;
    MonitoringContentHeader::Serialize(buffer, &headerSize, maxSize);
    unsigned char* cursor = buffer + headerSize;
    WriteMetric(cursor, m_piaVersion);
    WriteMetric(cursor, m_sdkVersion);
    WriteMetric(cursor, m_nexVersion);
    WriteMetric(cursor, m_environment);
    WriteMetric(cursor, m_platform);
    WriteMetric(cursor, m_platformRevision);
    WriteMetric(cursor, m_country);
    WriteMetric(cursor, m_region);
    WriteMetric(cursor, m_metric9);
    WriteMetric(cursor, m_metric10);
    WriteMetric(cursor, m_metric11);
    WriteMetric(cursor, m_metric12);
    WriteMetric(cursor, m_metric13);
    WriteMetric(cursor, m_metric14);
    WriteMetric(cursor, m_metric15);
    WriteMetric(cursor, m_metric16);
    WriteMetric(cursor, m_metric17);
    WriteMetric(cursor, m_metric18);
    WriteMetric(cursor, m_metric19);
    WriteMetric(cursor, m_metric20);
    WriteMetric(cursor, m_metric21);
    WriteMetric(cursor, m_metric22);
    WriteMetric(cursor, m_metric23);
    WriteMetric(cursor, m_metric24);
    WriteMetric(cursor, m_metric25);
    WriteMetric(cursor, m_metric26);
    WriteMetric(cursor, m_metric27);
    WriteMetric(cursor, m_metric28);
    WriteMetric(cursor, m_metric29);
    WriteMetric(cursor, m_metric30);
    WriteMetric(cursor, m_metric31);
    WriteMetric(cursor, m_metric32);
    WriteMetric(cursor, m_metric33);
    WriteMetric(cursor, m_metric34);
    WriteMetric(cursor, m_metric35);
    WriteMetric(cursor, m_metric36);
    WriteMetric(cursor, m_metric37);
    WriteMetric(cursor, m_metric38);
    WriteMetric(cursor, m_metric39);
    WriteMetric(cursor, m_metric40);
    WriteMetric(cursor, m_metric41);
    WriteMetric(cursor, m_metric42);
    WriteMetric(cursor, m_metric43);
    WriteMetric(cursor, m_metric44);
    WriteMetric(cursor, m_metric45);
    WriteMetric(cursor, m_metric46);
    WriteMetric(cursor, m_metric47);
    WriteMetric(cursor, m_joinResult);
    WriteMetric(cursor, m_joinProcessingTime);
    WriteMetric(cursor, m_metric50);
    for (unsigned int index = 0; index < 15; ++index)
        WriteMetric(cursor, m_metric51[index]);
    WriteMetric(cursor, m_metric52);
    for (unsigned int index = 0; index < 8; ++index)
        WriteMetric(cursor, m_metric53[index]);
    WriteMetric(cursor, m_metric54);
    WriteMetric(cursor, m_metric55);
    WriteMetric(cursor, m_metric56);
    WriteMetric(cursor, m_metric57);
    WriteMetric(cursor, m_metric58);
    WriteMetric(cursor, m_metric59);
    WriteMetric(cursor, m_metric60);
    WriteMetric(cursor, m_metric61);
    WriteMetric(cursor, m_metric62);
    WriteMetric(cursor, m_metric63);
    WriteMetric(cursor, m_metric64);
    WriteMetric(cursor, m_metric65);
    WriteMetric(cursor, m_metric66);
    WriteMetric(cursor, m_metric67);
    WriteMetric(cursor, m_metric68);
    WriteMetric(cursor, m_metric69);
    WriteMetric(cursor, m_metric70);
    WriteMetric(cursor, m_metric71);
    WriteMetric(cursor, m_metric72);
    WriteMetric(cursor, m_metric73);
    WriteMetric(cursor, m_metric74);
    WriteMetric(cursor, m_metric75);
    WriteMetric(cursor, m_metric76);
    WriteMetric(cursor, m_metric77);
    WriteMetric(cursor, m_metric78);
    WriteMetric(cursor, m_metric79);
    WriteMetric(cursor, m_metric80);
    WriteMetric(cursor, m_metric81);
    for (unsigned int index = 0; index < 6; ++index)
        WriteMetric(cursor, m_metric82[index]);
    WriteMetric(cursor, m_metric83);
    WriteMetric(cursor, m_metric84);
    WriteMetric(cursor, m_metric85);
    WriteMetric(cursor, m_metric86);
    WriteMetric(cursor, m_metric87);
    WriteMetric(cursor, m_metric88);
    for (unsigned int index = 0; index < 6; ++index)
        WriteMetric(cursor, m_metric89[index]);
    for (unsigned int index = 0; index < 6; ++index)
        WriteMetric(cursor, m_metric90[index]);
    for (unsigned int index = 0; index < 6; ++index)
        WriteMetric(cursor, m_metric91[index]);
    WriteMetric(cursor, m_metric92);
    WriteMetric(cursor, m_metric93);
    WriteMetric(cursor, m_metric94);
    WriteMetric(cursor, m_metric95);
    WriteMetric(cursor, m_metric96);
    WriteMetric(cursor, m_metric97);
    WriteMetric(cursor, m_metric98);
    WriteMetric(cursor, m_metric99);
    for (unsigned int index = 0; index < 6; ++index)
        WriteMetric(cursor, m_metric100[index]);
    for (unsigned int index = 0; index < 6; ++index)
        WriteMetric(cursor, m_metric101[index]);
    for (unsigned int index = 0; index < 6; ++index)
        WriteMetric(cursor, m_metric102[index]);
    WriteMetric(cursor, m_metric103);
    WriteMetric(cursor, m_metric104);
    WriteMetric(cursor, m_metric105);
    WriteMetric(cursor, m_joinRetryCount);
    WriteMetric(cursor, m_metric107);
    WriteMetric(cursor, m_metric108);
    WriteMetric(cursor, m_metric109);
    WriteMetric(cursor, m_metric110);
    WriteMetric(cursor, m_metric111);
    WriteMetric(cursor, m_metric112);
    WriteMetric(cursor, m_metric113);
    WriteMetric(cursor, m_metric114);
    WriteMetric(cursor, m_metric115);
    WriteMetric(cursor, m_metric116);
    WriteMetric(cursor, m_metric117);
    WriteMetric(cursor, m_metric118);
    WriteMetric(cursor, m_metric119);
    WriteMetric(cursor, m_retryReasonACount);
    WriteMetric(cursor, m_connectionInfoRetryCount);
    WriteMetric(cursor, m_ownerChangeRetryCount);
    WriteMetric(cursor, m_sessionInfoRetryCount);
    WriteMetric(cursor, m_retryReasonBCount);
    WriteMetric(cursor, m_joinTargetIdHash);
    WriteMetric(cursor, m_metric126);
    WriteMetric(cursor, m_metric127);
    WriteMetric(cursor, m_metric128);
    WriteMetric(cursor, m_joinSessionState);
    WriteMetric(cursor, m_metric130);
    WriteMetric(cursor, m_joinMeshState);
    WriteMetric(cursor, m_metric132);
    WriteMetric(cursor, m_metric133);
    WriteMetric(cursor, m_metric134);
    WriteMetric(cursor, m_metric135);
    WriteMetric(cursor, m_metric136);
    WriteMetric(cursor, m_metric137);
    WriteMetric(cursor, m_metric138);
    for (unsigned int index = 0; index < 15; ++index)
        WriteMetric(cursor, m_metric139[index]);
    for (unsigned int index = 0; index < 15; ++index)
        WriteMetric(cursor, m_metric140[index]);
    for (unsigned int index = 0; index < 15; ++index)
        WriteMetric(cursor, m_metric141[index]);
    for (unsigned int index = 0; index < 15; ++index)
        WriteMetric(cursor, m_metric142[index]);
    for (unsigned int index = 0; index < 15; ++index)
        WriteMetric(cursor, m_metric143[index]);
    for (unsigned int index = 0; index < 15; ++index)
        WriteMetric(cursor, m_metric144[index]);
    for (unsigned int index = 0; index < 15; ++index)
        WriteMetric(cursor, m_metric145[index]);
    for (unsigned int index = 0; index < 15; ++index)
        WriteMetric(cursor, m_metric146[index]);
    for (unsigned int index = 0; index < 15; ++index)
        WriteMetric(cursor, m_metric147[index]);
    WriteMetric(cursor, m_metric148);
    WriteMetric(cursor, m_metric149);
    WriteMetric(cursor, m_metric150);
    WriteMetric(cursor, m_metric151);
    WriteMetric(cursor, m_metric152);
    WriteMetric(cursor, m_metric153);
    WriteMetric(cursor, m_metric154);
    for (unsigned int index = 0; index < 15; ++index)
        WriteMetric(cursor, m_metric155[index]);
    WriteMetric(cursor, m_metric156);
    WriteMetric(cursor, m_metric157);
    WriteMetric(cursor, m_metric158);
    WriteMetric(cursor, m_metric159);
    WriteMetric(cursor, m_metric160);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric161[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric162[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric163[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric164[index]);
    for (unsigned int index = 0; index < 150; ++index)
        WriteMetric(cursor, m_metric165[index]);
    for (unsigned int index = 0; index < 150; ++index)
        WriteMetric(cursor, m_metric166[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric167[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric168[index]);
    for (unsigned int index = 0; index < 150; ++index)
        WriteMetric(cursor, m_metric169[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric170[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric171[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric172[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric173[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric174[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric175[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric176[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric177[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric178[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric179[index]);
    for (unsigned int index = 0; index < 16; ++index)
        WriteMetric(cursor, m_metric180[index]);
    WriteMetric(cursor, m_metric181);
    WriteMetric(cursor, m_metric182);
    for (unsigned int index = 0; index < 300; ++index)
        WriteMetric(cursor, m_metric183[index]);
    for (unsigned int index = 0; index < 8; ++index)
        WriteMetric(cursor, m_metric184[index]);
    WriteMetric(cursor, m_integrityMarker);
    *size = static_cast<unsigned int>(cursor - buffer);
    return 0;
}

}}}
