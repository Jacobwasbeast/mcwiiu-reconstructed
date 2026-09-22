#include "nn/pia/inet/SessionParticipantInfo.hpp"
#include <cstring>

namespace nn { namespace pia { namespace inet {

SessionParticipantInfo::SessionParticipantInfo()
    : participantState(0), participantEntries(0), participantCapacity(0), obsoleteEntryCount(0)
{
    std::memset(participantStorage, 0, sizeof(participantStorage));
}

SessionParticipantInfo::~SessionParticipantInfo() {}

void SessionParticipantInfo::Cleanup()
{
    participantState = 0;
    participantEntries = 0;
    obsoleteEntryCount = 0;
}

void SessionParticipantInfo::RemoveObsoleteEntry()
{
    if (obsoleteEntryCount != 0) --obsoleteEntryCount;
    if (participantEntries != 0) --participantEntries;
    if (participantEntries == 0) participantState = 0;
}

}}}
