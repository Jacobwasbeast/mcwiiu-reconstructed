#include "nn/pia/nat/NatTraverser.hpp"

#include "nn/pia/inet/IFacade.hpp"
#include "nn/pia/nat/NatTraversalProtocol.hpp"
#include "nn/pia/nat/NatTraversalTime.hpp"

namespace nn { namespace pia { namespace nat {

void NatTraverser::RelayProbeRequest(const transport::StationLocation& source,
                                     const transport::StationLocation& destination,
                                     bool relay, unsigned short port)
{
    m_facade->RelayProbeRequest(source, destination);
    if (!relay)
        return;

    NatTraversalTime* traversalTime = m_protocol->m_traversalTimes.Find(source);
    if (traversalTime != 0)
    {
        traversalTime->m_port = port;
        traversalTime->m_endTime.SetNow();
        traversalTime->m_count = 0;
        return;
    }

    NatTraversalTime newTraversalTime;
    newTraversalTime.m_location = source;
    newTraversalTime.m_startTime.SetNow();
    newTraversalTime.m_endTime.SetNow();
    newTraversalTime.m_count = 0;
    newTraversalTime.m_port = port;
    m_protocol->m_traversalTimes.Add(newTraversalTime);
}

void NatTraverser::SetLocalPublicLocation(const transport::StationLocation& location)
{
    if (m_protocol == 0)
        return;
    m_protocol->m_localLocation = location;
}

}}}
