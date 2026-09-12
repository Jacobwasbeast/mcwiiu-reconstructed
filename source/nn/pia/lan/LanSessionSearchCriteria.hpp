#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANSESSIONSEARCHCRITERIA_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANSESSIONSEARCHCRITERIA_HPP

#include "nn/pia/Reconstruction.hpp"

namespace nn { namespace pia { namespace lan {

class LanSessionSearchCriteria : public session::SessionSearchCriteria
{
public:
    LanSessionSearchCriteria();
    virtual ~LanSessionSearchCriteria();

    void SetMaxParticipantNum(unsigned short participantNum);
    void SetVacantOnly(bool vacantOnly);
    void SetOpenedOnly(bool openedOnly);

    unsigned int m_field10;
    unsigned int m_field14;
    unsigned char m_attributeRange[6][0x50];
    unsigned int m_attributeField1[6];
    unsigned int m_attributeField2[6];
    unsigned int m_attributeField3[6];
    unsigned char m_attributeFlags[6];
    unsigned short m_minParticipantNumRangeMin;
    unsigned short m_minParticipantNumRangeMax;
    unsigned short m_maxParticipantNumRangeMin;
    unsigned short m_maxParticipantNumRangeMax;
    bool m_openedOnly;
    bool m_vacantOnly;
    unsigned int m_setFlags;
};

}}}

#endif
