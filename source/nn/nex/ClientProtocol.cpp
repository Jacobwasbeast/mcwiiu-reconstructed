#include "nn/nex/ClientProtocol.hpp"

#include <cwchar>

#include "nn/nex/Credentials.hpp"
#include "nn/nex/Platform.hpp"

namespace nn
{
namespace nex
{

ClientProtocol::ClientProtocol(unsigned int uiFlags)
    : Protocol(uiFlags)
    , m_pDefaultCredentials(0)
{
}

ClientProtocol::~ClientProtocol()
{
    SetDefaultCredentials(0);
}

const wchar_t* ClientProtocol::GetType() const
{
    return L"ClientProtocol";
}

bool ClientProtocol::IsAKindOf(const wchar_t* pType) const
{
    return std::wcscmp(L"ClientProtocol", pType) == 0
        || std::wcscmp(L"Protocol", pType) == 0
        || std::wcscmp(L"SystemComponent", pType) == 0;
}

void ClientProtocol::EnforceDeclareSysComponentMacro()
{
}

Protocol::_Type ClientProtocol::GetProtocolType() const
{
    return TYPE_CLIENT;
}

void ClientProtocol::SetDefaultCredentials(Credentials* pCredentials)
{
    if (m_pDefaultCredentials != 0)
    {
        m_pDefaultCredentials->ReleaseRef();
        m_pDefaultCredentials = 0;
    }

    if (pCredentials == 0)
        return;

    m_pDefaultCredentials = pCredentials;
    m_pDefaultCredentials->AddRef();
}

void ClientProtocol::SetCallError(qResult)
{
    Platform::Breakpoint();
}

}
}
