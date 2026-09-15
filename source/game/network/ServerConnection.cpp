#include "game/network/ServerConnection.hpp"
#include "game/network/Connection.hpp"
#include "game/core/Socket.hpp"
#include "game/network/ServerConnectionListener.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/MinecraftServer.hpp"
#include <cwchar>

ServerConnection::ServerConnection(MinecraftServer* server)
{
    mConnectionCount = 0;
    InitializeCriticalSection(&mCriticalSection);
    mServer = server;
}

ServerConnection::~ServerConnection()
{
    DeleteCriticalSection(&mCriticalSection);
}

int ServerConnection::addPendingTextureRequest(const std::wstring& request)
{
    std::vector<std::wstring>::const_iterator requestIterator;
    for (requestIterator = mPendingTextureRequests.begin();
         requestIterator != mPendingTextureRequests.end(); ++requestIterator) {
        if (*requestIterator == request) {
            return 1;
        }
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
    std::vector<std::wstring>::iterator requestIterator;
    for (requestIterator = mPendingTextureRequests.begin();
         requestIterator != mPendingTextureRequests.end(); ++requestIterator) {
        if (*requestIterator == request) {
            mPendingTextureRequests.erase(requestIterator);
            break;
        }
    }
    std::vector<boost::shared_ptr<ServerGamePacketListenerImpl> >::iterator listenerIterator;
    for (listenerIterator = mPlayers.begin(); listenerIterator != mPlayers.end(); ++listenerIterator) {
        if (*listenerIterator && !(*listenerIterator)->isServerPacketListener()) {
            (*listenerIterator)->handleTextureReceived(request);
        }
    }
}

void ServerConnection::handleTextureAndGeometryReceived(const std::wstring& request)
{
    std::vector<std::wstring>::iterator requestIterator;
    for (requestIterator = mPendingTextureRequests.begin();
         requestIterator != mPendingTextureRequests.end(); ++requestIterator) {
        if (*requestIterator == request) {
            mPendingTextureRequests.erase(requestIterator);
            break;
        }
    }
    std::vector<boost::shared_ptr<ServerGamePacketListenerImpl> >::iterator listenerIterator;
    for (listenerIterator = mPlayers.begin(); listenerIterator != mPlayers.end(); ++listenerIterator) {
        if (*listenerIterator && !(*listenerIterator)->isServerPacketListener()) {
            (*listenerIterator)->handleTextureAndGeometryReceived(request);
        }
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
    std::wstring name(L"Connection #");
    wchar_t buffer[32];
    std::swprintf(buffer, 32, L"%d", mConnectionCount);
    name.append(buffer);
    boost::shared_ptr<ServerConnectionListener> listener(
        new ServerConnectionListener(mServer, socket, name));
    handleConnection(listener);
}

void ServerConnection::stop()
{
    EnterCriticalSection(&mCriticalSection);
    std::vector<boost::shared_ptr<ServerConnectionListener> >::iterator listenerIterator;
    for (listenerIterator = mListeners.begin(); listenerIterator != mListeners.end(); ++listenerIterator) {
        if (*listenerIterator) {
            (*listenerIterator)->disconnect(DisconnectPacket::eDisconnectReason_ConnectionClosed);
        }
    }
    LeaveCriticalSection(&mCriticalSection);

    std::vector<boost::shared_ptr<ServerGamePacketListenerImpl> >::iterator playerIterator;
    for (playerIterator = mPlayers.begin(); playerIterator != mPlayers.end(); ++playerIterator) {
        if (*playerIterator) {
            Connection* connection = (*playerIterator)->getConnection();
            if (connection) {
                connection->close(DisconnectPacket::eDisconnectReason_ConnectionClosed);
            }
        }
    }
}

void ServerConnection::tick()
{
    std::vector<boost::shared_ptr<ServerConnectionListener> > listeners;
    EnterCriticalSection(&mCriticalSection);
    listeners = mListeners;
    LeaveCriticalSection(&mCriticalSection);

    std::vector<boost::shared_ptr<ServerConnectionListener> >::iterator listenerIterator;
    for (listenerIterator = listeners.begin(); listenerIterator != listeners.end(); ++listenerIterator) {
        (*listenerIterator)->tick();
    }

    EnterCriticalSection(&mCriticalSection);
    listenerIterator = mListeners.begin();
    while (listenerIterator != mListeners.end()) {
        if ((*listenerIterator)->isDisconnected()) {
            listenerIterator = mListeners.erase(listenerIterator);
        } else {
            ++listenerIterator;
        }
    }
    LeaveCriticalSection(&mCriticalSection);

    std::vector<boost::shared_ptr<ServerGamePacketListenerImpl> >::iterator playerIterator;
    for (playerIterator = mPlayers.begin(); playerIterator != mPlayers.end(); ++playerIterator) {
        (*playerIterator)->tick();
        Connection* connection = (*playerIterator)->getConnection();
        connection->flush();
    }
}

std::vector<boost::shared_ptr<ServerGamePacketListenerImpl> >& ServerConnection::getPlayers()
{
    return mPlayers;
}
