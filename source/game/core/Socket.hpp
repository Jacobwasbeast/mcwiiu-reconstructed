#ifndef GAME_CORE_SOCKET_HPP
#define GAME_CORE_SOCKET_HPP
#include <deque>
#include "game/core/InputStream.hpp"
#include "game/core/OutputStream.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/world/biome/arrayWithLength.hpp"
class INetworkPlayer;
class ServerConnection;
class Socket {
    friend class ServerGamePacketListenerImpl;
public:
 class SocketInputStreamLocal:public InputStream { public: explicit SocketInputStreamLocal(int); virtual ~SocketInputStreamLocal(); virtual int read(); virtual int read(arrayWithLength<unsigned char>); virtual int read(arrayWithLength<unsigned char>,unsigned int,unsigned int); virtual void close(); virtual long long skip(long long); virtual void flush(); private: int mIndex; bool mOpen; };
 class SocketInputStreamNetwork:public InputStream { public: SocketInputStreamNetwork(Socket*,int); virtual ~SocketInputStreamNetwork(); virtual int read(); virtual int read(arrayWithLength<unsigned char>); virtual int read(arrayWithLength<unsigned char>,unsigned int,unsigned int); virtual void close(); virtual long long skip(long long); virtual void flush(); private: int mIndex; bool mOpen; Socket* mSocket; };
 class SocketOutputStream:public OutputStream { public: virtual ~SocketOutputStream(); virtual void writeWithFlags(arrayWithLength<unsigned char>,unsigned int,unsigned int,int); };
 class SocketOutputStreamLocal:public SocketOutputStream { public: explicit SocketOutputStreamLocal(int); virtual ~SocketOutputStreamLocal(); virtual void write(unsigned int); virtual void write(arrayWithLength<unsigned char>); virtual void write(arrayWithLength<unsigned char>,unsigned int,unsigned int); virtual void close(); virtual void flush(); private: int mIndex; bool mOpen; };
 class SocketOutputStreamNetwork:public SocketOutputStream { public: SocketOutputStreamNetwork(Socket*,int); virtual ~SocketOutputStreamNetwork(); virtual void write(unsigned int); virtual void write(arrayWithLength<unsigned char>); virtual void write(arrayWithLength<unsigned char>,unsigned int,unsigned int); virtual void writeWithFlags(arrayWithLength<unsigned char>,unsigned int,unsigned int,int); virtual void close(); virtual void flush(); private: int mIndex; bool mOpen; Socket* mSocket; };
 Socket(bool); Socket(INetworkPlayer*,bool,bool); INetworkPlayer* getPlayer(); void setPlayer(INetworkPlayer*); static void Initialise(ServerConnection*); unsigned int getRemoteSocketAddress(); void pushDataToQueue(const unsigned char*,unsigned int,bool); static void addIncomingSocket(Socket*); InputStream* getInputStream(bool); void setSoTimeout(int); void setTrafficClass(int); OutputStream* getOutputStream(bool); bool close(bool);
private:
 bool mLocal,mNetwork; int mIndex; std::deque<unsigned char> mQueues[2]; OSFastMutex mMutexes[2]; InputStream* mInputStreams[2]; OutputStream* mOutputStreams[2]; bool mInputClosed[2]; bool mOutputClosed[2]; unsigned char mPlayerId; INetworkPlayer* mPlayer; void* mEvent; bool mOpen; static ServerConnection* sConnection;
};
#endif
