#include "nn/nex/ServerProtocol.hpp"

#include "nn/nex/CallProtocolMethodOperation.hpp"
#include "nn/nex/SecurityContextManager.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/InetAddress.hpp"

#include <stdint.h>

#include <cwchar>

namespace nn
{
namespace nex
{

ServerProtocol::ServerProtocol(unsigned int uiFlags)
    : Protocol(uiFlags)
{
}

ServerProtocol::~ServerProtocol()
{
}

const wchar_t* ServerProtocol::GetType() const
{
    return L"ServerProtocol";
}

bool ServerProtocol::IsAKindOf(const wchar_t* pType) const
{
    return std::wcscmp(L"ServerProtocol", pType) == 0
        || std::wcscmp(L"Protocol", pType) == 0
        || std::wcscmp(L"SystemComponent", pType) == 0;
}

void ServerProtocol::EnforceDeclareSysComponentMacro()
{
}

Protocol::_Type ServerProtocol::GetProtocolType() const
{
    return TYPE_SERVER;
}

void ServerProtocol::DispatchProtocolMessageWithAttemptCount(unsigned int, Message*, Message*,
                                                             bool*, int*, EndPoint*)
{
}

bool ServerProtocol::UseAttemptCountMethod()
{
    return false;
}

void ServerProtocol::SetCallError(qResult result)
{
    static_cast<CallProtocolMethodOperation*>(GetCurrentOperation())->m_result = result;
}

void ServerProtocol::DispatchCallRequest(unsigned int uiAttemptCount, Message* pRequest,
                                         Message* pResponse, bool* pResponseRequired,
                                         int* pAttempt, EndPoint* pEndPoint)
{
    SecurityContextManager* pSecurity = SecurityContextManager::GetInstance();
    if (reinterpret_cast<intptr_t>(pEndPoint) == -8)
    {
        pSecurity->Push(GetCallerPID(), GetCallerCID(), pSecurity->GetCurrentAddress());
    }
    else if (pEndPoint == 0)
    {
        pSecurity->Push(GetCallerPID(), GetCallerCID(), pSecurity->GetCurrentAddress());
    }
    else
    {
        const unsigned char* pBytes = reinterpret_cast<const unsigned char*>(pEndPoint);
        StationURL* pStationURL = reinterpret_cast<StationURL*>(
            const_cast<unsigned char*>(pBytes) + 16);
        const InetAddress* pInetAddress = pStationURL->GetInetAddress();
        unsigned int uiAddress = pInetAddress == 0 ? 0 : pInetAddress->GetAddress();
        if (uiAddress == 0)
            uiAddress = pSecurity->GetCurrentAddress();
        pSecurity->Push(*reinterpret_cast<const unsigned int*>(pBytes + 0x4C),
                        *reinterpret_cast<const unsigned int*>(pBytes + 0x48), uiAddress);
        SetIncomingConnection(pEndPoint);
    }

    const bool bOldRVDDL = reinterpret_cast<intptr_t>(pEndPoint) != -8
        && IsOldRVDDLVersion(pEndPoint);
    if (bOldRVDDL)
    {
        *reinterpret_cast<unsigned char*>(reinterpret_cast<unsigned char*>(pRequest) + 0x41) = 1;
        *reinterpret_cast<unsigned char*>(reinterpret_cast<unsigned char*>(pResponse) + 0x41) = 1;
    }

    if (FlagIsSet(2))
    {
        typedef bool (*AttemptMethod)(ServerProtocol*);
        typedef void (*DispatchMethod)(ServerProtocol*, Message*, Message*, bool*, EndPoint*);
        typedef void (*DispatchAttemptMethod)(ServerProtocol*, unsigned int, Message*, Message*,
                                              bool*, int*, EndPoint*);
        void** pVTable = *reinterpret_cast<void***>(this);
        const bool bUseAttemptCount = reinterpret_cast<AttemptMethod>(pVTable[45])(this);
        if (bUseAttemptCount)
            reinterpret_cast<DispatchAttemptMethod>(pVTable[43])(
                this, uiAttemptCount, pRequest, pResponse, pResponseRequired, pAttempt, pEndPoint);
        else
            reinterpret_cast<DispatchMethod>(pVTable[41])(
                this, pRequest, pResponse, pResponseRequired, pEndPoint);
    }
    else
    {
        if (UseAttemptCountMethod())
            DispatchProtocolMessageWithAttemptCount(uiAttemptCount, pRequest, pResponse,
                                                     pResponseRequired, pAttempt, pEndPoint);
        else
            DispatchProtocolMessage(pRequest, pResponse, pResponseRequired, pEndPoint);
    }

    if (pEndPoint != 0 && reinterpret_cast<intptr_t>(pEndPoint) != -8)
        SetIncomingConnection(0);
    pSecurity->Pop();

    if (FlagIsSet(8) && pResponseRequired != 0)
        *pResponseRequired = false;
}

}
}
