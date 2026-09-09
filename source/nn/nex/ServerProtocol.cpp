#include "nn/nex/ServerProtocol.hpp"

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

}
}
