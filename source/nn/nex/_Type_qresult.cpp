#include "nn/nex/_Type_qresult.hpp"

#include "nn/nex/Message.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

void _Type_qresult::Extract(Message* pMessage, qResult* pDestination)
{
    int iCode;
    pMessage->ExtractPrimitive(&iCode);
    *pDestination = qResult(iCode);
}

}
}
