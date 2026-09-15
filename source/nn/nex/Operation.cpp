#include "nn/nex/Operation.hpp"

namespace nn { namespace nex {

Operation::Operation(unsigned int flags)
    : m_state(false), m_signalState(false), m_userContext(), m_flags(flags)
{
}

Operation::~Operation()
{
}

unsigned short Operation::GetSignal() const
{
    return static_cast<unsigned short>(GetType());
}

}}
