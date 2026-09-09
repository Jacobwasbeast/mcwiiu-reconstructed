#include "nn/nex/Protocol.hpp"

#include <cwchar>

#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

Protocol::Protocol(unsigned int uiFlags)
    : SystemComponent(String(L"<unknown>"))
    , m_uiProtocolID(0)
    , m_pOutgoingConnection(0)
    , m_pProtocolRequestBroker(0)
    , m_uiFlags(uiFlags)
    , m_pIncomingConnection(0)
    , m_bLocalLoopback(false)
    , m_uiCallerPID(0)
    , m_uiCallerCID(0)
{
}

Protocol::~Protocol()
{
}

const wchar_t* Protocol::GetType() const
{
    return L"Protocol";
}

bool Protocol::IsAKindOf(const wchar_t* pType) const
{
    return std::wcscmp(L"Protocol", pType) == 0 || std::wcscmp(L"SystemComponent", pType) == 0;
}

void Protocol::EnforceDeclareSysComponentMacro()
{
}

void Protocol::EndPointDisconnected(EndPoint*)
{
}

void Protocol::FaultDetected(EndPoint*, unsigned int)
{
}

Protocol* Protocol::Clone() const
{
    return 0;
}

bool Protocol::FlagIsSet(unsigned int uiFlag) const
{
    return (m_uiFlags & uiFlag) == uiFlag;
}

void Protocol::SetFlag(unsigned int uiFlag)
{
    m_uiFlags |= uiFlag;
}

void Protocol::SetProtocolID(unsigned short uiProtocolID)
{
    m_uiProtocolID = uiProtocolID;
}

EndPoint* Protocol::GetOutgoingConnection() const
{
    return m_pOutgoingConnection;
}

void Protocol::SetOutgoingConnection(EndPoint* pEndPoint)
{
    m_pOutgoingConnection = pEndPoint;
}

void Protocol::UseLocalLoopback(unsigned int uiCallerPID, unsigned int uiCallerCID)
{
    m_uiCallerPID = uiCallerPID;
    m_uiCallerCID = uiCallerCID;
    m_pIncomingConnection = 0;
    m_bLocalLoopback = true;
}

unsigned int Protocol::GetCallerPID() const
{
    return m_uiCallerPID;
}

unsigned int Protocol::GetCallerCID() const
{
    return m_uiCallerCID;
}

void Protocol::AssociateProtocolRequestBroker(ProtocolRequestBrokerInterface* pRequestBroker)
{
    m_pProtocolRequestBroker = pRequestBroker;
}

void Protocol::AddProtocolKey(Message* pMessage, _Command command, unsigned short uiProtocolID)
{
    if (uiProtocolID < 0x7F)
    {
        unsigned char uiKey = static_cast<unsigned char>(uiProtocolID & 0x7F);
        if (command != COMMAND_RESPONSE)
            uiKey |= 0x80;
        pMessage->Append(&uiKey, 1, 1);
        return;
    }

    unsigned char uiKey = 0x7F;
    if (command != COMMAND_RESPONSE)
        uiKey |= 0x80;
    pMessage->Append(&uiKey, 1, 1);

    unsigned short uiExtendedProtocolID = uiProtocolID;
    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiExtendedProtocolID), 2, 1);
}

unsigned int Protocol::ExtractMethodID(Message* pMessage)
{
    unsigned int uiMethodID;
    pMessage->ExtractPrimitive(&uiMethodID);
    return uiMethodID;
}

void Protocol::AddMethodID(Message* pMessage, unsigned int uiMethodID)
{
    unsigned int uiValue = uiMethodID;
    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiValue), 4, 1);
}

}
}
