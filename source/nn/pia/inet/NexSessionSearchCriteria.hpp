#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXSESSIONSEARCHCRITERIA_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXSESSIONSEARCHCRITERIA_HPP

#include <cstdint>

#include "nn/pia/common/FixedString.hpp"
#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/inet/ParamVariant.hpp"
#include "nn/pia/inet/SessionType.hpp"
#include "nn/pia/session/SessionSearchCriteria.hpp"

namespace nn { namespace nex {
class InetAddress;
class MatchmakeParam;
class MatchmakeSessionSearchCriteria;
class ResultRange;
}}

namespace nn { namespace pia { namespace inet {

enum SelectionMethod
{
    cSelectionMethod_0 = 0,
    cSelectionMethod_1 = 1,
    cSelectionMethod_2 = 2
};

class NexSessionSearchCriteria : public session::SessionSearchCriteria
{
public:
    enum { cAttributeNum = 6, cAttributeListSize = 100 };

    NexSessionSearchCriteria();
    virtual ~NexSessionSearchCriteria();

    virtual nn::pia::common::Result SetResultRange(unsigned int offset, unsigned int size);
    void SetMinParticipantNum(unsigned short value);
    void SetMaxParticipantNum(unsigned short value);
    void SetGameMode(unsigned int value);
    unsigned int IsSetSessionMatchmakeKeyword() const;
    nn::pia::common::Result SetAttribute(unsigned int index, unsigned int value);
    nn::pia::common::Result SetAttributeRange(unsigned int index, unsigned int minimum, unsigned int maximum);
    void SetRandomSessionSelectionMethod(SelectionMethod method);
    void Copy(const NexSessionSearchCriteria& other);
    void ConvertToNexMatchmakeSessionSearchCriteria(
        nn::nex::MatchmakeSessionSearchCriteria* criteria,
        nn::nex::InetAddress* address,
        nn::nex::MatchmakeParam* param,
        nn::nex::ResultRange* resultRange,
        unsigned char method) const;
    virtual void Trace(unsigned long long value) const;
    virtual void Reset();
    unsigned int IsValid() const;

private:
    unsigned int m_gameMode;
    SessionType m_sessionType;
    unsigned int m_attributeList[cAttributeNum][cAttributeListSize];
    unsigned int m_attributeListSize[cAttributeNum];
    unsigned int m_attributeRangeMin[cAttributeNum];
    unsigned int m_attributeRangeMax[cAttributeNum];
    bool m_attributeRangeUsed[cAttributeNum];
    bool m_excludeNonHostPrincipalId;
    bool m_excludeUserPasswordSet;
    SelectionMethod m_randomSessionSelectionMethod;
    unsigned int m_scoreBasedSettingIndex;
    unsigned int m_scoreBasedRatingValue;
    unsigned int m_scoreBasedDisconnectionRate;
    unsigned int m_scoreBasedViolationRate;
    unsigned int m_countryCode;
    bool m_scoreBasedGeoIpUsed;
    common::InetAddress m_overrideIpAddress;
    unsigned int m_communityId;
    unsigned short m_minParticipantRangeMin;
    unsigned short m_minParticipantRangeMax;
    unsigned short m_maxParticipantRangeMin;
    unsigned short m_maxParticipantRangeMax;
    bool m_openedOnly;
    bool m_vacantOnly;
    common::FixedString<8> m_additionalScoreBasedParameterValueKey[cAttributeNum / 2];
    ParamVariant m_additionalScoreBasedParameterValues[cAttributeNum / 2];
    common::FixedString<8> m_additionalScoreBasedParameterStringKey;
    common::FixedString<64> m_additionalScoreBasedParameterString;
    common::FixedString<32> m_sessionMatchmakeKeyword;
    unsigned int m_conditionMask;
};

}}}

#endif
