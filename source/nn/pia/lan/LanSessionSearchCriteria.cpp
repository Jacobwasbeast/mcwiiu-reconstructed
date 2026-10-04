#include "nn/pia/lan/LanSessionSearchCriteria.hpp"

#include <coreinit/memory.h>

#include "nn/pia/lan/LanSessionInfo.hpp"

namespace nn { namespace pia { namespace lan {

namespace
{
enum
{
    cResultInvalidArgument = 0x10c07,
    cParticipantNumUnset = 0xffff,
    cDefaultResultRangeSize = 100,
    cSerializedSize = 0x23a
};
}

LanSessionSearchCriteria::LanSessionSearchCriteria()
    : session::SessionSearchCriteria(),
      m_gameMode(0),
      m_sessionType(0),
      m_minParticipantRangeMax(cParticipantNumUnset),
      m_minParticipantRangeMin(cParticipantNumUnset),
      m_maxParticipantRangeMax(cParticipantNumUnset),
      m_maxParticipantRangeMin(cParticipantNumUnset),
      m_openedOnly(true),
      m_vacantOnly(true),
      m_setFlags(0)
{
    for (unsigned int index = 0; index < cAttributeCount; ++index)
    {
        OSBlockSet(m_attributeList[index], 0, sizeof(m_attributeList[index]));
        m_attributeListSize[index] = 0;
        m_attributeRangeMin[index] = 0;
        m_attributeRangeMax[index] = 0;
        m_attributeRangeUsed[index] = false;
    }

    m_resultRangeSize = cDefaultResultRangeSize;
}

LanSessionSearchCriteria::~LanSessionSearchCriteria()
{
}

void LanSessionSearchCriteria::SetMaxParticipantNum(unsigned short participantNum)
{
    m_maxParticipantRangeMax = participantNum;
    m_maxParticipantRangeMin = participantNum;
    m_setFlags |= cSetFlagMaxParticipantNum;
}

void LanSessionSearchCriteria::SetVacantOnly(bool vacantOnly)
{
    m_vacantOnly = vacantOnly;
    m_setFlags |= cSetFlagVacantOnly;
}

void LanSessionSearchCriteria::SetOpenedOnly(bool openedOnly)
{
    m_openedOnly = openedOnly;
    m_setFlags |= cSetFlagOpenedOnly;
}

bool LanSessionSearchCriteria::IsSetGameMode() const
{
    return (m_setFlags & cSetFlagGameMode) != 0;
}

bool LanSessionSearchCriteria::IsSetMinParticipantNum() const
{
    return (m_setFlags & cSetFlagMinParticipantNum) != 0;
}

bool LanSessionSearchCriteria::IsSetMaxParticipantNum() const
{
    return (m_setFlags & cSetFlagMaxParticipantNum) != 0;
}

bool LanSessionSearchCriteria::IsSetSessionType() const
{
    return (m_setFlags & cSetFlagSessionType) != 0;
}

bool LanSessionSearchCriteria::IsSetAttribute(unsigned int index) const
{
    if (index >= cAttributeCount)
        return false;
    return ((m_setFlags >> (index + cAttributeFlagShift)) & 1) != 0;
}

bool LanSessionSearchCriteria::IsHit(const LanSessionInfo* sessionInfo)
{
    bool hit = true;
    const unsigned int gameMode = sessionInfo->GetGameMode();
    if ((!IsSetGameMode() || m_gameMode == gameMode) &&
        (!IsSetMinParticipantNum() || m_minParticipantRangeMin == cParticipantNumUnset ||
         m_minParticipantRangeMax == cParticipantNumUnset ||
         (m_minParticipantRangeMin <= sessionInfo->GetMinParticipantNum() &&
          sessionInfo->GetMinParticipantNum() <= m_minParticipantRangeMax)) &&
        (!IsSetMaxParticipantNum() || m_maxParticipantRangeMin == cParticipantNumUnset ||
         m_maxParticipantRangeMax == cParticipantNumUnset ||
         (m_maxParticipantRangeMin <= sessionInfo->GetMaxParticipantNum() &&
          sessionInfo->GetMaxParticipantNum() <= m_maxParticipantRangeMax)) &&
        (!IsSetSessionType() || m_sessionType == sessionInfo->GetSessionType()) &&
        (!m_openedOnly || sessionInfo->IsOpened()))
    {
        if (!m_vacantOnly ||
            sessionInfo->GetCurrentParticipantNum() != sessionInfo->GetMaxParticipantNum())
        {
            for (unsigned int index = 0; index < cAttributeCount; ++index)
            {
                if (!IsSetAttribute(index))
                    continue;

                if (m_attributeRangeUsed[index])
                {
                    unsigned int value = 0;
                    const Result result =
                        sessionInfo->GetAttribute(static_cast<unsigned char>(index), &value);
                    if (result != 0 || value < m_attributeRangeMin[index] ||
                        value > m_attributeRangeMax[index])
                    {
                        hit = false;
                        break;
                    }
                }
                else if (m_attributeListSize[index] != 0)
                {
                    unsigned int value = 0;
                    hit = false;
                    const Result result =
                        sessionInfo->GetAttribute(static_cast<unsigned char>(index), &value);
                    if (result != 0)
                        return false;
                    for (unsigned int entry = 0; entry < m_attributeListSize[index]; ++entry)
                    {
                        if (value == m_attributeList[index][entry])
                            hit = true;
                    }
                    if (!hit)
                        return false;
                }
            }
        }
        else
        {
            hit = false;
        }
    }
    else
    {
        hit = false;
    }
    return hit;
}

unsigned int LanSessionSearchCriteria::GetSerializedSize() const
{
    return cSerializedSize;
}

Result LanSessionSearchCriteria::Serialize(unsigned char* buffer, unsigned int* size,
                                           unsigned int bufferSize) const
{
    if (buffer == 0)
        return cResultInvalidArgument;
    if (size == 0)
        return cResultInvalidArgument;
    if (bufferSize < GetSerializedSize())
        return cResultInvalidArgument;

    unsigned char* cursor = buffer;
    common::serializeU16(cursor, m_minParticipantRangeMax);
    common::serializeU16(cursor + 2, m_minParticipantRangeMin);
    common::serializeU16(cursor + 4, m_maxParticipantRangeMax);
    common::serializeU16(cursor + 6, m_maxParticipantRangeMin);
    common::serializeU8(cursor + 8, m_openedOnly);
    common::serializeU8(cursor + 9, m_vacantOnly);
    common::serializeU32(cursor + 10, m_resultRangeOffset);
    common::serializeU32(cursor + 14, m_resultRangeSize);
    common::serializeU32(cursor + 18, m_gameMode);
    common::serializeU32(cursor + 22, m_sessionType);
    cursor += 26;
    for (unsigned int index = 0; index < cAttributeCount; ++index)
    {
        for (unsigned int entry = 0; entry < cAttributeListMax; ++entry)
        {
            common::serializeU32(cursor, m_attributeList[index][entry]);
            cursor += 4;
        }
    }
    for (unsigned int index = 0; index < cAttributeCount; ++index)
    {
        common::serializeU8(cursor, static_cast<unsigned char>(m_attributeListSize[index]));
        cursor += 1;
    }
    for (unsigned int index = 0; index < cAttributeCount; ++index)
    {
        common::serializeU32(cursor, m_attributeRangeMin[index]);
        cursor += 4;
    }
    for (unsigned int index = 0; index < cAttributeCount; ++index)
    {
        common::serializeU32(cursor, m_attributeRangeMax[index]);
        cursor += 4;
    }
    for (unsigned int index = 0; index < cAttributeCount; ++index)
    {
        common::serializeU8(cursor, m_attributeRangeUsed[index]);
        cursor += 1;
    }
    common::serializeU32(cursor, m_setFlags);
    cursor += 4;
    *size = static_cast<unsigned int>(cursor - buffer);
    return 0;
}

Result LanSessionSearchCriteria::Deserialize(const unsigned char* buffer, unsigned int bufferSize)
{
    if (buffer == 0)
        return cResultInvalidArgument;
    if (bufferSize < GetSerializedSize())
        return cResultInvalidArgument;

    const unsigned char* cursor = buffer;
    m_minParticipantRangeMax = common::deserializeU16(cursor);
    m_minParticipantRangeMin = common::deserializeU16(cursor + 2);
    m_maxParticipantRangeMax = common::deserializeU16(cursor + 4);
    m_maxParticipantRangeMin = common::deserializeU16(cursor + 6);
    m_openedOnly = common::deserializeU8(cursor + 8) != 0;
    m_vacantOnly = common::deserializeU8(cursor + 9) != 0;
    m_resultRangeOffset = common::deserializeU32(cursor + 10);
    m_resultRangeSize = common::deserializeU32(cursor + 14);
    m_gameMode = common::deserializeU32(cursor + 18);
    m_sessionType = common::deserializeU32(cursor + 22);
    cursor += 26;
    for (unsigned int index = 0; index < cAttributeCount; ++index)
    {
        for (unsigned int entry = 0; entry < cAttributeListMax; ++entry)
        {
            m_attributeList[index][entry] = common::deserializeU32(cursor);
            cursor += 4;
        }
    }
    for (unsigned int index = 0; index < cAttributeCount; ++index)
    {
        m_attributeListSize[index] = common::deserializeU8(cursor);
        cursor += 1;
    }
    for (unsigned int index = 0; index < cAttributeCount; ++index)
    {
        m_attributeRangeMin[index] = common::deserializeU32(cursor);
        cursor += 4;
    }
    for (unsigned int index = 0; index < cAttributeCount; ++index)
    {
        m_attributeRangeMax[index] = common::deserializeU32(cursor);
        cursor += 4;
    }
    for (unsigned int index = 0; index < cAttributeCount; ++index)
    {
        m_attributeRangeUsed[index] = common::deserializeU8(cursor) != 0;
        cursor += 1;
    }
    m_setFlags = common::deserializeU32(cursor);
    return 0;
}

void LanSessionSearchCriteria::Trace(unsigned long long value) const
{
}

void LanSessionSearchCriteria::Reset()
{
    session::SessionSearchCriteria::Reset();
    m_setFlags = 0;
    m_maxParticipantRangeMin = cParticipantNumUnset;
    m_maxParticipantRangeMax = cParticipantNumUnset;
    m_openedOnly = true;
    m_minParticipantRangeMax = cParticipantNumUnset;
    m_sessionType = 0;
    m_vacantOnly = true;
    m_minParticipantRangeMin = cParticipantNumUnset;
    m_gameMode = 0;
    for (unsigned int index = 0; index < cAttributeCount; ++index)
    {
        OSBlockSet(m_attributeList[index], 0, sizeof(m_attributeList[index]));
        m_attributeListSize[index] = 0;
        m_attributeRangeMin[index] = 0;
        m_attributeRangeMax[index] = 0;
        m_attributeRangeUsed[index] = false;
    }
    m_resultRangeSize = cDefaultResultRangeSize;
}

void LanSessionSearchCriteria::Copy(const LanSessionSearchCriteria& other)
{
    session::SessionSearchCriteria::Copy(other);
    m_minParticipantRangeMax = other.m_minParticipantRangeMax;
    m_minParticipantRangeMin = other.m_minParticipantRangeMin;
    m_maxParticipantRangeMax = other.m_maxParticipantRangeMax;
    m_maxParticipantRangeMin = other.m_maxParticipantRangeMin;
    m_openedOnly = other.m_openedOnly;
    m_vacantOnly = other.m_vacantOnly;
    m_gameMode = other.m_gameMode;
    m_sessionType = other.m_sessionType;
    m_setFlags = other.m_setFlags;
    for (unsigned int index = 0; index < cAttributeCount; ++index)
    {
        OSBlockMove(m_attributeList[index], other.m_attributeList[index],
                    sizeof(m_attributeList[index]), FALSE);
        m_attributeListSize[index] = other.m_attributeListSize[index];
        m_attributeRangeMin[index] = other.m_attributeRangeMin[index];
        m_attributeRangeMax[index] = other.m_attributeRangeMax[index];
        m_attributeRangeUsed[index] = other.m_attributeRangeUsed[index];
    }
}

LanSessionSearchCriteria& LanSessionSearchCriteria::operator=(const LanSessionSearchCriteria& other)
{
    Copy(other);
    return *this;
}

}}}
