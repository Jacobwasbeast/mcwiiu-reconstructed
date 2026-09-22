#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_CREATESESSIONSETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_CREATESESSIONSETTING_HPP

#include <stdint.h>

namespace nn { namespace pia { namespace session {

class CreateSessionSetting
{
public:
    CreateSessionSetting();
    virtual ~CreateSessionSetting();

    uint16_t GetMinParticipantNum() const;
    uint8_t IsSetMinParticipantNum() const;
    void SetMaxParticipantNum(uint16_t value);
    uint16_t GetMaxParticipantNum() const;
    uint8_t IsSetMaxParticipantNum() const;
    void Reset();
    void Trace(uint64_t value) const;
    void Copy(const CreateSessionSetting& other);

    void SetMinParticipantNum(uint16_t value);

private:
    uint16_t mMinParticipantNum;
    uint8_t mMinParticipantNumSet;
    uint8_t mReserved;
    uint16_t mMaxParticipantNum;
    uint8_t mMaxParticipantNumSet;
};

} } }

#endif
