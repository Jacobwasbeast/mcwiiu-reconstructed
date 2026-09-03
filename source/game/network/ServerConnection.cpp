#include "game/network/ServerConnection.hpp"
#include "game/network/Connection.hpp"
#include "game/core/Socket.hpp"
#include "game/network/ServerConnectionListener.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/MinecraftServer.hpp"
#include <cwchar>

ServerConnection::ServerConnection(MinecraftServer* server)
    : mConnectionCount(0), mServer(server)
{
    InitializeCriticalSection(&mCriticalSection);
}

ServerConnection::~ServerConnection()
{
    DeleteCriticalSection(&mCriticalSection);
}

int ServerConnection::addPendingTextureRequest(const std::wstring& request)
{
    std::vector<std::wstring>::const_iterator it;
    for (it = mPendingTextureRequests.begin(); it != mPendingTextureRequests.end(); ++it) {
        if (*it == request) return 1;
    }
    mPendingTextureRequests.push_back(request);
    return 1;
}

void ServerConnection::addServerGamePacketListenerImpl(boost::shared_ptr<ServerGamePacketListenerImpl> listener)
{
    mPlayers.push_back(listener);
}

void ServerConnection::handleTextureReceived(const std::wstring& request)
{
    std::vector<std::wstring>::iterator texture;
    for (texture = mPendingTextureRequests.begin(); texture != mPendingTextureRequests.end(); ++texture) {
        if (*texture == request) {
            mPendingTextureRequests.erase(texture);
            break;
        }
    }
    std::vector<boost::shared_ptr<ServerGamePacketListenerImpl> >::iterator it;
    for (it = mPlayers.begin(); it != mPlayers.end(); ++it) {
        if (*it && !(*it)->isServerPacketListener()) (*it)->handleTextureReceived(request);
    }
}

void ServerConnection::handleTextureAndGeometryReceived(const std::wstring& request)
{
    std::vector<std::wstring>::iterator texture;
    for (texture = mPendingTextureRequests.begin(); texture != mPendingTextureRequests.end(); ++texture) {
        if (*texture == request) {
            mPendingTextureRequests.erase(texture);
            break;
        }
    }
    std::vector<boost::shared_ptr<ServerGamePacketListenerImpl> >::iterator it;
    for (it = mPlayers.begin(); it != mPlayers.end(); ++it) {
        if (*it && !(*it)->isServerPacketListener()) (*it)->handleTextureAndGeometryReceived(request);
    }
}

void ServerConnection::handleConnection(boost::shared_ptr<ServerConnectionListener> listener)
{
    EnterCriticalSection(&mCriticalSection);
    mListeners.push_back(listener);
    LeaveCriticalSection(&mCriticalSection);
}

void ServerConnection::NewIncomingSocket(Socket* socket)
{
    ++mConnectionCount;
    wchar_t buffer[32];
    std::swprintf(buffer, 32, L"Connection #%d", mConnectionCount);
    boost::shared_ptr<ServerConnectionListener> listener(
        new ServerConnectionListener(mServer, socket, std::wstring(buffer)));
    handleConnection(listener);
}

void ServerConnection::stop()
{
    EnterCriticalSection(&mCriticalSection);
    mListeners.clear();
    LeaveCriticalSection(&mCriticalSection);
    std::vector<boost::shared_ptr<ServerGamePacketListenerImpl> >::iterator it;
    for (it = mPlayers.begin(); it != mPlayers.end(); ++it) {
        if (*it) {
            Connection* connection = (*it)->getConnection();
            if (connection) connection->close(2);
        }
    }
}

void ServerConnection::tick()
{
    std::vector<boost::shared_ptr<ServerConnectionListener> > listeners;
    EnterCriticalSection(&mCriticalSection);
    listeners = mListeners;
    LeaveCriticalSection(&mCriticalSection);
    std::vector<boost::shared_ptr<ServerConnectionListener> >::iterator listener;
    for (listener = listeners.begin(); listener != listeners.end(); ++listener) {
        if (*listener) (*listener)->tick();
    }
    std::vector<boost::shared_ptr<ServerGamePacketListenerImpl> >::iterator player;
    for (player = mPlayers.begin(); player != mPlayers.end(); ++player) {
        if (*player) {
            (*player)->tick();
            Connection* connection = (*player)->getConnection();
            if (connection) connection->flush();
        }
    }
}

std::vector<boost::shared_ptr<ServerGamePacketListenerImpl> >& ServerConnection::getPlayers()
{
    return mPlayers;
}

void ServerConnection::handleServerSettingsChanged(boost::shared_ptr<ServerSettingsChangedPacket> packet)
{
    (void)packet;
}
