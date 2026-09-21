#include "nn/nex/JobConnectEndPoint.hpp"

namespace nn
{
namespace nex
{

void JobConnectEndPoint::ReportConnectResult(qResult result)
{
    Trace(m_traceData);
    m_result = result;
    SetToReady();
    ReleaseRef();
}

}
}
