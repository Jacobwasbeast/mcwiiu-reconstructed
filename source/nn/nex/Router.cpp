#include "nn/nex/Router.hpp"

#include "nn/nex/Core.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/RoutingAddressResolver.hpp"
#include "nn/nex/RoutingStream.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

Router::Router()
    : m_bEnabled(false)
    , m_bStreamRegistered(false)
    , m_pResolver(new RoutingAddressResolver())
    , m_pRoutingStream(new RoutingStream())
{
}

Router::~Router()
{
    UnregisterStream();
    delete m_pResolver;
    delete m_pRoutingStream;
}

void Router::EnableRouting(bool bEnable)
{
    m_bEnabled = bEnable;
    if (bEnable)
    {
        RootTransport* pTransport = m_pRoutingStream->m_pTransport;
        if (pTransport != 0)
            m_bStreamRegistered = pTransport->m_streamManager.RegisterStream(m_pRoutingStream, 1, 0);
    }
    else
    {
        UnregisterStream();
    }
}

void Router::SetTransport(RootTransport* pTransport)
{
    UnregisterStream();
    m_pRoutingStream->m_pTransport = pTransport;
    if (m_bEnabled)
        m_bStreamRegistered = pTransport->m_streamManager.RegisterStream(m_pRoutingStream, 1, 0);
}

void Router::UnregisterStream()
{
    if (m_bStreamRegistered)
    {
        RootTransport* pTransport = m_pRoutingStream->m_pTransport;
        if (pTransport != 0)
        {
            m_bStreamRegistered = !pTransport->m_streamManager.UnregisterStream(
                m_pRoutingStream->m_Type, 1);
        }
    }
}

bool Router::ShouldRoute(const InetAddress* pAddress)
{
    if (!m_bEnabled)
        return false;
    return HasRoute(pAddress, 0);
}

bool Router::HasRoute(const InetAddress* pAddress, const InetAddress* pSource)
{
    InetAddress route;
    if (m_pRoutingStream->m_routingTable.Find(*pAddress, route))
    {
        if (pAddress->GetAddress() == route.GetAddress() &&
            pAddress->GetPortNumber() == route.GetPortNumber())
            return false;
        if (pSource == 0 || *pSource == route)
            return true;
    }
    return false;
}

void Router::AddRoute(const InetAddress& rAddress, const InetAddress& rRoute, bool bReplace)
{
    m_pRoutingStream->m_routingTable.Add(rAddress, rRoute, bReplace);
}

void Router::Clean()
{
    CriticalSection& csScheduler = Core::GetInstance()->GetScheduler()->GetCriticalSection();
    csScheduler.Enter();
    m_pRoutingStream->m_routingTable.Clean();
    m_pRoutingStream->m_history.clear();
    m_pRoutingStream->m_denied.clear();
    csScheduler.Leave();
}

unsigned int Router::GetRoutingIPAddressTemplate()
{
    static InetAddress s_routingTemplate(L"255.0.0.0", 0);
    return s_routingTemplate.GetAddress();
}

void Router::ConstructAndAssociateRoutingAddress(unsigned short uiPort, InetAddress oAddress,
    InetAddress* pRoutingAddress)
{
    m_pResolver->AddMapping(uiPort, oAddress);
    pRoutingAddress->SetAddress(GetRoutingIPAddressTemplate());
    pRoutingAddress->SetPortNumber(uiPort);
}

}
}
