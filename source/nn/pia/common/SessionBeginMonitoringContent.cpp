#include "nn/pia/common/SessionBeginMonitoringContent.hpp"

#include <algorithm>
#include <coreinit/memory.h>
#include <nn/act/client_cpp.h>

namespace nn { namespace pia { namespace common {

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
    Result result(0x10c07);
    if (buffer == 0)
        return result;
    if (size == 0)
        return result;
    if (maxSize < GetSerializedSize())
        return result;

    unsigned int headerSize;
    MonitoringContentHeader::Serialize(buffer, &headerSize, maxSize);
    unsigned char* cursor = buffer + headerSize;
    serializeU32(cursor, m_piaVersion);
    cursor += 4;
    serializeU32(cursor, m_sdkVersion);
    cursor += 4;
    serializeU32(cursor, m_nexVersion);
    cursor += 4;
    serializeU8(cursor, m_environment);
    cursor += 1;
    serializeU8(cursor, m_platform);
    cursor += 1;
    serializeU8(cursor, m_platformRevision);
    cursor += 1;
    serializeU8(cursor, m_country);
    cursor += 1;
    serializeU8(cursor, m_region);
    cursor += 1;
    serializeU32(cursor, m_metric9);
    cursor += 4;
    serializeU32(cursor, m_metric10);
    cursor += 4;
    serializeU64(cursor, m_metric11);
    cursor += 8;
    serializeU8(cursor, m_metric12);
    cursor += 1;
    serializeU32(cursor, m_metric13);
    cursor += 4;
    serializeU32(cursor, m_metric14);
    cursor += 4;
    serializeU8(cursor, m_metric15);
    cursor += 1;
    serializeU8(cursor, m_metric16);
    cursor += 1;
    serializeU32(cursor, m_metric17);
    cursor += 4;
    serializeU32(cursor, m_metric18);
    cursor += 4;
    serializeU8(cursor, m_metric19);
    cursor += 1;
    serializeU8(cursor, m_metric20);
    cursor += 1;
    serializeU8(cursor, m_metric21);
    cursor += 1;
    serializeU8(cursor, m_metric22);
    cursor += 1;
    serializeU16(cursor, m_metric23);
    cursor += 2;
    serializeU16(cursor, m_metric24);
    cursor += 2;
    serializeU16(cursor, m_metric25);
    cursor += 2;
    serializeU32(cursor, m_metric26);
    cursor += 4;
    serializeU32(cursor, m_metric27);
    cursor += 4;
    serializeU32(cursor, m_metric28);
    cursor += 4;
    serializeU32(cursor, m_metric29);
    cursor += 4;
    serializeU8(cursor, m_metric30);
    cursor += 1;
    serializeU16(cursor, m_metric31);
    cursor += 2;
    serializeU32(cursor, m_metric32);
    cursor += 4;
    serializeU32(cursor, m_metric33);
    cursor += 4;
    serializeU32(cursor, m_metric34);
    cursor += 4;
    serializeU32(cursor, m_metric35);
    cursor += 4;
    serializeU32(cursor, m_metric36);
    cursor += 4;
    serializeU16(cursor, m_metric37);
    cursor += 2;
    serializeU32(cursor, m_metric38);
    cursor += 4;
    serializeU16(cursor, m_metric39);
    cursor += 2;
    serializeU32(cursor, m_metric40);
    cursor += 4;
    serializeU16(cursor, m_metric41);
    cursor += 2;
    serializeU16(cursor, m_metric42);
    cursor += 2;
    serializeU16(cursor, m_metric43);
    cursor += 2;
    serializeU32(cursor, m_metric44);
    cursor += 4;
    serializeU8(cursor, m_metric45);
    cursor += 1;
    serializeU8(cursor, m_metric46);
    cursor += 1;
    serializeU8(cursor, m_metric47);
    cursor += 1;
    serializeU32(cursor, m_joinResult);
    cursor += 4;
    serializeU32(cursor, m_joinProcessingTime);
    cursor += 4;
    serializeU8(cursor, m_metric50);
    cursor += 1;
    for (unsigned int index = 0; index < 15; ++index)
        {
        serializeU32(cursor, m_metric51[index]);
        cursor += 4;
    }
    serializeU8(cursor, m_metric52);
    cursor += 1;
    for (unsigned int index = 0; index < 8; ++index)
        {
        serializeU32(cursor, m_metric53[index]);
        cursor += 4;
    }
    serializeU16(cursor, m_metric54);
    cursor += 2;
    serializeU8(cursor, m_metric55);
    cursor += 1;
    serializeU8(cursor, m_metric56);
    cursor += 1;
    serializeU32(cursor, m_metric57);
    cursor += 4;
    serializeU16(cursor, m_metric58);
    cursor += 2;
    serializeU16(cursor, m_metric59);
    cursor += 2;
    serializeU16(cursor, m_metric60);
    cursor += 2;
    serializeU16(cursor, m_metric61);
    cursor += 2;
    serializeU16(cursor, m_metric62);
    cursor += 2;
    serializeU16(cursor, m_metric63);
    cursor += 2;
    serializeU16(cursor, m_metric64);
    cursor += 2;
    serializeU16(cursor, m_metric65);
    cursor += 2;
    serializeU8(cursor, m_metric66);
    cursor += 1;
    serializeU16(cursor, m_metric67);
    cursor += 2;
    serializeU16(cursor, m_metric68);
    cursor += 2;
    serializeU16(cursor, m_metric69);
    cursor += 2;
    serializeU8(cursor, m_metric70);
    cursor += 1;
    serializeU32(cursor, m_metric71);
    cursor += 4;
    serializeU8(cursor, m_metric72);
    cursor += 1;
    serializeU8(cursor, m_metric73);
    cursor += 1;
    serializeU8(cursor, m_metric74);
    cursor += 1;
    serializeU32(cursor, m_metric75);
    cursor += 4;
    serializeU32(cursor, m_metric76);
    cursor += 4;
    serializeU32(cursor, m_metric77);
    cursor += 4;
    serializeU8(cursor, m_metric78);
    cursor += 1;
    serializeU8(cursor, m_metric79);
    cursor += 1;
    serializeU32(cursor, m_metric80);
    cursor += 4;
    serializeU32(cursor, m_metric81);
    cursor += 4;
    for (unsigned int index = 0; index < 6; ++index)
        {
        serializeU32(cursor, m_metric82[index]);
        cursor += 4;
    }
    serializeU16(cursor, m_metric83);
    cursor += 2;
    serializeU32(cursor, m_metric84);
    cursor += 4;
    serializeU8(cursor, m_metric85);
    cursor += 1;
    serializeU8(cursor, m_metric86);
    cursor += 1;
    serializeU8(cursor, m_metric87);
    cursor += 1;
    serializeU8(cursor, m_metric88);
    cursor += 1;
    for (unsigned int index = 0; index < 6; ++index)
        {
        serializeU8(cursor, m_metric89[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 6; ++index)
        {
        serializeU32(cursor, m_metric90[index]);
        cursor += 4;
    }
    for (unsigned int index = 0; index < 6; ++index)
        {
        serializeU32(cursor, m_metric91[index]);
        cursor += 4;
    }
    serializeU8(cursor, m_metric92);
    cursor += 1;
    serializeU8(cursor, m_metric93);
    cursor += 1;
    serializeU8(cursor, m_metric94);
    cursor += 1;
    serializeU32(cursor, m_metric95);
    cursor += 4;
    serializeU8(cursor, m_metric96);
    cursor += 1;
    serializeU8(cursor, m_metric97);
    cursor += 1;
    serializeU8(cursor, m_metric98);
    cursor += 1;
    serializeU8(cursor, m_metric99);
    cursor += 1;
    for (unsigned int index = 0; index < 6; ++index)
        {
        serializeU8(cursor, m_metric100[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 6; ++index)
        {
        serializeU32(cursor, m_metric101[index]);
        cursor += 4;
    }
    for (unsigned int index = 0; index < 6; ++index)
        {
        serializeU32(cursor, m_metric102[index]);
        cursor += 4;
    }
    serializeU8(cursor, m_metric103);
    cursor += 1;
    serializeU8(cursor, m_metric104);
    cursor += 1;
    serializeU8(cursor, m_metric105);
    cursor += 1;
    serializeU8(cursor, m_joinRetryCount);
    cursor += 1;
    serializeU8(cursor, m_metric107);
    cursor += 1;
    serializeU8(cursor, m_metric108);
    cursor += 1;
    serializeU32(cursor, m_metric109);
    cursor += 4;
    serializeU32(cursor, m_metric110);
    cursor += 4;
    serializeU32(cursor, m_metric111);
    cursor += 4;
    serializeU8(cursor, m_metric112);
    cursor += 1;
    serializeU8(cursor, m_metric113);
    cursor += 1;
    serializeU32(cursor, m_metric114);
    cursor += 4;
    serializeU32(cursor, m_metric115);
    cursor += 4;
    serializeU32(cursor, m_metric116);
    cursor += 4;
    serializeU8(cursor, m_metric117);
    cursor += 1;
    serializeU8(cursor, m_metric118);
    cursor += 1;
    serializeU8(cursor, m_metric119);
    cursor += 1;
    serializeU8(cursor, m_retryReasonACount);
    cursor += 1;
    serializeU8(cursor, m_connectionInfoRetryCount);
    cursor += 1;
    serializeU8(cursor, m_ownerChangeRetryCount);
    cursor += 1;
    serializeU8(cursor, m_sessionInfoRetryCount);
    cursor += 1;
    serializeU8(cursor, m_retryReasonBCount);
    cursor += 1;
    serializeU32(cursor, m_joinTargetIdHash);
    cursor += 4;
    serializeU32(cursor, m_metric126);
    cursor += 4;
    serializeU8(cursor, m_metric127);
    cursor += 1;
    serializeU8(cursor, m_metric128);
    cursor += 1;
    serializeU8(cursor, m_joinSessionState);
    cursor += 1;
    serializeU8(cursor, m_metric130);
    cursor += 1;
    serializeU8(cursor, m_joinMeshState);
    cursor += 1;
    serializeU8(cursor, m_metric132);
    cursor += 1;
    serializeU16(cursor, m_metric133);
    cursor += 2;
    serializeU16(cursor, m_metric134);
    cursor += 2;
    serializeU32(cursor, m_metric135);
    cursor += 4;
    serializeU32(cursor, m_metric136);
    cursor += 4;
    serializeU16(cursor, m_metric137);
    cursor += 2;
    serializeU16(cursor, m_metric138);
    cursor += 2;
    for (unsigned int index = 0; index < 15; ++index)
        {
        serializeU32(cursor, m_metric139[index]);
        cursor += 4;
    }
    for (unsigned int index = 0; index < 15; ++index)
        {
        serializeU32(cursor, m_metric140[index]);
        cursor += 4;
    }
    for (unsigned int index = 0; index < 15; ++index)
        {
        serializeU32(cursor, m_metric141[index]);
        cursor += 4;
    }
    for (unsigned int index = 0; index < 15; ++index)
        {
        serializeU32(cursor, m_metric142[index]);
        cursor += 4;
    }
    for (unsigned int index = 0; index < 15; ++index)
        {
        serializeU32(cursor, m_metric143[index]);
        cursor += 4;
    }
    for (unsigned int index = 0; index < 15; ++index)
        {
        serializeU8(cursor, m_metric144[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 15; ++index)
        {
        serializeU8(cursor, m_metric145[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 15; ++index)
        {
        serializeU8(cursor, m_metric146[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 15; ++index)
        {
        serializeU8(cursor, m_metric147[index]);
        cursor += 1;
    }
    serializeU8(cursor, m_metric148);
    cursor += 1;
    serializeU8(cursor, m_metric149);
    cursor += 1;
    serializeU16(cursor, m_metric150);
    cursor += 2;
    serializeU16(cursor, m_metric151);
    cursor += 2;
    serializeU16(cursor, m_metric152);
    cursor += 2;
    serializeU16(cursor, m_metric153);
    cursor += 2;
    serializeU16(cursor, m_metric154);
    cursor += 2;
    for (unsigned int index = 0; index < 15; ++index)
        {
        serializeU8(cursor, m_metric155[index]);
        cursor += 1;
    }
    serializeU32(cursor, m_metric156);
    cursor += 4;
    serializeU16(cursor, m_metric157);
    cursor += 2;
    serializeU8(cursor, m_metric158);
    cursor += 1;
    serializeU8(cursor, m_metric159);
    cursor += 1;
    serializeU8(cursor, m_metric160);
    cursor += 1;
    for (unsigned int index = 0; index < 16; ++index)
        {
        serializeU8(cursor, m_metric161[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 16; ++index)
        {
        serializeU8(cursor, m_metric162[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 16; ++index)
        {
        serializeU8(cursor, m_metric163[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 16; ++index)
        {
        serializeU8(cursor, m_metric164[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 150; ++index)
        {
        serializeU8(cursor, m_metric165[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 150; ++index)
        {
        serializeU8(cursor, m_metric166[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 16; ++index)
        {
        serializeU8(cursor, m_metric167[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 16; ++index)
        {
        serializeU8(cursor, m_metric168[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 150; ++index)
        {
        serializeU8(cursor, m_metric169[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 16; ++index)
        {
        serializeU8(cursor, m_metric170[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 16; ++index)
        {
        serializeU8(cursor, m_metric171[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 16; ++index)
        {
        serializeU8(cursor, m_metric172[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 16; ++index)
        {
        serializeU8(cursor, m_metric173[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 16; ++index)
        {
        serializeU8(cursor, m_metric174[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 16; ++index)
        {
        serializeU8(cursor, m_metric175[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 16; ++index)
        {
        serializeU8(cursor, m_metric176[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 16; ++index)
        {
        serializeU8(cursor, m_metric177[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 16; ++index)
        {
        serializeU8(cursor, m_metric178[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 16; ++index)
        {
        serializeU8(cursor, m_metric179[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 16; ++index)
        {
        serializeU8(cursor, m_metric180[index]);
        cursor += 1;
    }
    serializeU32(cursor, m_metric181);
    cursor += 4;
    serializeU32(cursor, m_metric182);
    cursor += 4;
    for (unsigned int index = 0; index < 300; ++index)
        {
        serializeU8(cursor, m_metric183[index]);
        cursor += 1;
    }
    for (unsigned int index = 0; index < 8; ++index)
        {
        serializeU8(cursor, m_metric184[index]);
        cursor += 1;
    }
    serializeU32(cursor, m_integrityMarker);
    cursor += 4;
    *size = static_cast<unsigned int>(cursor - buffer);
    return 0;
}

}}}
