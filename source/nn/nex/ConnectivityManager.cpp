#include "nn/nex/ConnectivityManager.hpp"

#include "nn/nex/InstanceTable.hpp"

namespace nn
{
namespace nex
{

ConnectivityManager* ConnectivityManager::GetInstance()
{
    return GetInstanceWithoutNetworkCheck();
}

ConnectivityManager* ConnectivityManager::GetInstanceWithoutNetworkCheck()
{
    unsigned int contextId = 0;
    if (g_usingInstantiationContext)
        contextId = g_instantiationContextVariable->GetRef();

    return static_cast<ConnectivityManager*>(g_instanceTable.FindInstance(11, contextId));
}

bool ConnectivityManager::RegisterConnectivityTester(ConnectivityTester* connectivityTester)
{
    if (m_connectivityTester != 0)
        return false;

    m_connectivityTester = connectivityTester;
    return true;
}

bool ConnectivityManager::UnregisterConnectivityTester()
{
    if (m_connectivityTester == 0)
        return false;

    m_connectivityTester = 0;
    return true;
}

}
}
