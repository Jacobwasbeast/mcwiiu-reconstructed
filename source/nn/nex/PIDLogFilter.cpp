#include "nn/nex/PIDLogFilter.hpp"

#include "nn/nex/ProtocolRequestBrokerInterface.hpp"

namespace nn
{
namespace nex
{

bool PIDLogFilter::OutputCurrentTrace()
{
    unsigned int current;
    if (g_protocolCurrentPID != 0 && (current = g_protocolCurrentPID()) < m_upperBound &&
        m_lowerBound < current)
        return true;
    return false;
}

}
}
