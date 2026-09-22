#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_SESSION_PARTICIPANT_INFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_SESSION_PARTICIPANT_INFO_HPP

namespace nn { namespace pia { namespace inet {

class SessionParticipantInfo
{
public:
    SessionParticipantInfo();
    virtual ~SessionParticipantInfo();
    void Cleanup();
    void RemoveObsoleteEntry();

private:
    unsigned int participantState;
    unsigned int participantEntries;
    unsigned int participantCapacity;
    unsigned int obsoleteEntryCount;
    unsigned char participantStorage[1296];
};

}}}

#endif
