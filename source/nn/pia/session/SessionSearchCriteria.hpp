#ifndef NN_PIA_SESSION_SESSIONSEARCHCRITERIA_HPP
#define NN_PIA_SESSION_SESSIONSEARCHCRITERIA_HPP

#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace session {

class SessionSearchCriteria
{
public:
    static const int cKeyTypeOwner = 1;
    static const int cKeyTypeSessionId = 3;
    SessionSearchCriteria();
    virtual ~SessionSearchCriteria();
    virtual nn::pia::common::Result SetResultRange(unsigned int offset, unsigned int size);
    virtual void Reset();
    unsigned int GetResultRangeOffset() const;
    unsigned int GetResultRangeSize() const;
    int GetKeyType() const;
    void SetKeyType(int keyType);
    void Copy(const SessionSearchCriteria& other);
    virtual void Trace(unsigned long long value) const;

    unsigned int m_resultRangeOffset;
    unsigned int m_resultRangeSize;
    unsigned int m_keyType;
};

}}}

#endif
