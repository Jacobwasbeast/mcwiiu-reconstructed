#include "nn/nex/ConnectionOrientedStream.hpp"

namespace nn
{
namespace nex
{

namespace
{

const int STREAM_RESULT_SUCCESS = 0x00010001;
const int STREAM_RESULT_NOT_IMPLEMENTED = 0x80010002;
const int STREAM_RESULT_ALREADY_REGISTERED = 0x8001000E;

}

bool ConnectionOrientedStream::OpenEndPoint(EndPoint*)
{
    return false;
}

EndPoint* ConnectionOrientedStream::FindEndPointByPID(unsigned int)
{
    return 0;
}

unsigned int ConnectionOrientedStream::GetEndPointNumber()
{
    return 0;
}

EndPoint* ConnectionOrientedStream::FindEndPointByCID(unsigned int)
{
    return 0;
}

qVector<unsigned int> ConnectionOrientedStream::GetAllConnectionIDs() const
{
    return qVector<unsigned int>();
}

void ConnectionOrientedStream::LockEndPointMap()
{
}

void ConnectionOrientedStream::UnlockEndPointMap()
{
}

qResult ConnectionOrientedStream::Initialize()
{
    return qResult(STREAM_RESULT_SUCCESS);
}

qResult ConnectionOrientedStream::Teardown()
{
    return qResult(STREAM_RESULT_SUCCESS);
}

qResult ConnectionOrientedStream::RegisterEventHandler(TransportEventHandler* pEventHandler)
{
    if (m_pEventHandler != 0)
    {
        return qResult(STREAM_RESULT_ALREADY_REGISTERED);
    }

    m_pEventHandler = pEventHandler;
    return qResult(STREAM_RESULT_SUCCESS);
}

qResult ConnectionOrientedStream::SendBroadcast(StationURL*, Buffer*)
{
    return qResult(STREAM_RESULT_NOT_IMPLEMENTED);
}

qResult ConnectionOrientedStream::Send(StationURL*, Buffer*)
{
    return qResult(STREAM_RESULT_NOT_IMPLEMENTED);
}

qResult ConnectionOrientedStream::AddEndPointToGroup(EndPoint*, EndPointGroup*)
{
    return qResult(STREAM_RESULT_NOT_IMPLEMENTED);
}

qResult ConnectionOrientedStream::RemoveEndPointFromGroup(EndPoint*, EndPointGroup*)
{
    return qResult(STREAM_RESULT_NOT_IMPLEMENTED);
}

void ConnectionOrientedStream::Trace(unsigned long long)
{
}

void ConnectionOrientedStream::AddPIDEndPointAssociation(unsigned int, EndPoint*)
{
}

bool ConnectionOrientedStream::RemovePIDEndPointAssociation(unsigned int, EndPoint*)
{
    return false;
}

void ConnectionOrientedStream::AddCIDEndPointAssociation(unsigned int, EndPoint*)
{
}

bool ConnectionOrientedStream::RemoveCIDEndPointAssociation(unsigned int, EndPoint*)
{
    return false;
}

}
}
