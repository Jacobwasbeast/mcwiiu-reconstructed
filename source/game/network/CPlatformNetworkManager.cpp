#include "game/network/CPlatformNetworkManager.hpp"

class CPlatformNetworkManagerNintendo;

CPlatformNetworkManager* CPlatformNetworkManager::Create()
{
    return new CPlatformNetworkManagerNintendo();
}

void CPlatformNetworkManager::CancelJoinGame()
{
}

void CPlatformNetworkManager::FakeLocalPlayerJoined()
{
}
