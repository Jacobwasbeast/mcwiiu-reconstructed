#ifndef NN_PIA_SESSION_COMMONMATCHMAKESESSION_HPP
#define NN_PIA_SESSION_COMMONMATCHMAKESESSION_HPP

#include <stdint.h>
#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace session {

class CommonMatchmakeSession
{
public:
    CommonMatchmakeSession();
    virtual ~CommonMatchmakeSession();
    virtual void Cleanup();
    virtual unsigned int ConvertGatheringIdToPersistentGatheringCode(unsigned int gatheringId);
    virtual unsigned int ConvertPersistentGatheringCodeToGatheringId(unsigned long long code);
    virtual unsigned int GetBrowsedCommunityResult();
    virtual bool IsMatchmakeOwner() const;
    virtual void SetMatchmakeOwner(bool owner);
    virtual unsigned long long GetMatchmakeOwnerUniqueId() const;
    virtual void SetMatchmakeOwnerUniqueId(unsigned long long id);
    virtual nn::pia::common::Result UpdateSelectionPriority(unsigned int priority, unsigned char state);
    virtual unsigned int IsUpdatableSelectionPriority(unsigned char state);
    virtual void UpdateStationInfoList();
    void Trace(unsigned long long value) const;

private:
    unsigned long long m_matchmakeOwnerUniqueId;
    bool m_matchmakeOwner;
};

}}}

#endif
