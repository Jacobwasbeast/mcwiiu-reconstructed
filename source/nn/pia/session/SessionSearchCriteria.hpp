#ifndef NN_PIA_SESSION_SESSIONSEARCHCRITERIA_HPP
#define NN_PIA_SESSION_SESSIONSEARCHCRITERIA_HPP

#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace session {

class SessionSearchCriteria
{
public:
    SessionSearchCriteria();
    virtual ~SessionSearchCriteria();
    nn::pia::common::Result SetResultRange(unsigned int offset, unsigned int size);
    void Reset();
    unsigned int GetResultRangeOffset() const;
    unsigned int GetResultRangeSize() const;
    int GetKeyType() const;
    void Copy(const SessionSearchCriteria& other);
    virtual void Trace(unsigned long long value) const;

private:
    unsigned int m_resultRangeOffset;
    unsigned int m_resultRangeSize;
    unsigned int m_keyType;
};

}}}

#endif
