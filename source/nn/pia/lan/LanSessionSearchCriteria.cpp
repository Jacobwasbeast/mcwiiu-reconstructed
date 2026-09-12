#include "nn/pia/lan/LanSessionSearchCriteria.hpp"

#include <cstring>

namespace nn { namespace pia { namespace lan {

LanSessionSearchCriteria::LanSessionSearchCriteria()
    : session::SessionSearchCriteria(),
      m_field10(0),
      m_field14(0),
      m_minParticipantNumRangeMin(0xffff),
      m_minParticipantNumRangeMax(0xffff),
      m_maxParticipantNumRangeMin(0xffff),
      m_maxParticipantNumRangeMax(0xffff),
      m_openedOnly(true),
      m_vacantOnly(true),
      m_setFlags(0)
{
    std::memset(m_attributeRange, 0, sizeof(m_attributeRange));
    for (unsigned int index = 0; index < 6; ++index)
    {
        m_attributeField1[index] = 0;
        m_attributeField2[index] = 0;
        m_attributeField3[index] = 0;
        m_attributeFlags[index] = 0;
    }

    m_field04 = 100;
}

LanSessionSearchCriteria::~LanSessionSearchCriteria()
{
}

void LanSessionSearchCriteria::SetMaxParticipantNum(unsigned short participantNum)
{
    m_maxParticipantNumRangeMin = participantNum;
    m_maxParticipantNumRangeMax = participantNum;
    m_setFlags |= 2;
}

void LanSessionSearchCriteria::SetVacantOnly(bool vacantOnly)
{
    m_vacantOnly = vacantOnly;
    m_setFlags |= 8;
}

void LanSessionSearchCriteria::SetOpenedOnly(bool openedOnly)
{
    m_openedOnly = openedOnly;
    m_setFlags |= 4;
}

}}}
