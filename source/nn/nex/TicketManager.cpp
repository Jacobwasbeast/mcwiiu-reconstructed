#include "nn/nex/TicketManager.hpp"

#include "nn/nex/AuthenticationClient.hpp"
#include "nn/nex/CallContext.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/JobTicketManagerAcquireTicket.hpp"
#include "nn/nex/JobTicketManagerLogin.hpp"
#include "nn/nex/JobTicketManagerLoginWithData.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/Ticket.hpp"

namespace nn
{
namespace nex
{

TicketManager::TicketManager(AuthenticationClient* pAuthenticationClient)
    : m_tickets()
    , m_pAuthenticationClient(pAuthenticationClient)
{
}

TicketManager::~TicketManager()
{
}

bool TicketManager::Login(CallContext* pCallContext, qResult* pResult, const String& rUserName,
    const char* pPassword, unsigned int* pPrincipalID, RVConnectionData* pConnectionData,
    String* pReturnMessage, AnyObjectHolder<Data, String>* pCustomData)
{
    CriticalSection& csScheduler = Core::GetInstance()->GetScheduler()->GetCriticalSection();
    csScheduler.Enter();
    if (!pCallContext->InitiateCall())
    {
        csScheduler.Leave();
        return false;
    }
    JobTicketManagerLogin* pJob = new JobTicketManagerLogin(pCallContext->m_uiCallId, this,
        pResult, rUserName, pPassword, pPrincipalID, pConnectionData, pReturnMessage,
        pCustomData);
    Core::GetInstance()->GetScheduler()->Queue(pJob, false);
    csScheduler.Leave();
    return true;
}

bool TicketManager::LoginWithData(CallContext* pCallContext, qResult* pResult,
    const char* pPassword, unsigned int* pPrincipalID, RVConnectionData* pConnectionData,
    CustomDataHolder<LoginData>* pLoginData)
{
    CriticalSection& csScheduler = Core::GetInstance()->GetScheduler()->GetCriticalSection();
    csScheduler.Enter();
    if (!pCallContext->InitiateCall())
    {
        csScheduler.Leave();
        return false;
    }
    JobTicketManagerLoginWithData* pJob = new JobTicketManagerLoginWithData(
        pCallContext->m_uiCallId, this, pResult, pPassword, pPrincipalID, pConnectionData,
        pLoginData);
    Core::GetInstance()->GetScheduler()->Queue(pJob, false);
    csScheduler.Leave();
    return true;
}

bool TicketManager::AcquireTicket(CallContext* pCallContext, unsigned int uiSourcePrincipalID,
    unsigned int uiTargetPrincipalID, Ticket** ppTicket)
{
    CriticalSection& csScheduler = Core::GetInstance()->GetScheduler()->GetCriticalSection();
    csScheduler.Enter();
    if (!pCallContext->InitiateCall())
    {
        csScheduler.Leave();
        return false;
    }
    pCallContext->m_expirationTime = m_pAuthenticationClient->m_loginDeadline;
    JobTicketManagerAcquireTicket* pJob = new JobTicketManagerAcquireTicket(
        pCallContext->m_uiCallId, this, uiSourcePrincipalID, uiTargetPrincipalID, ppTicket);
    Core::GetInstance()->GetScheduler()->Queue(pJob, false);
    csScheduler.Leave();
    return true;
}

Ticket* TicketManager::FindTicket(unsigned int uiTargetPrincipalID)
{
    qMap<unsigned int, Ticket*>::iterator it = m_tickets.find(uiTargetPrincipalID);
    if (it == m_tickets.end())
        return 0;
    return it->second;
}

void TicketManager::InsertTicket(unsigned int uiTargetPrincipalID, Ticket* pTicket)
{
    m_tickets[uiTargetPrincipalID] = pTicket;
}

void TicketManager::ReleaseTicket(Ticket* pTicket)
{
    pTicket->ReleaseRef();
    if (pTicket->GetRefCount() == 1)
    {
        m_tickets.erase(pTicket->m_uiTargetPrincipalID);
        pTicket->ReleaseRef();
    }
}

}
}
