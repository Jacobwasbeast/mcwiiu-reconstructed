#include "game/core/Socket.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/network/ServerConnection.hpp"
#include "game/core/ShutdownManager.hpp"

extern void Sleep(unsigned int milliseconds);
ServerConnection* Socket::sConnection = NULL;
static std::deque<unsigned char> gLocalQueues[2];
static OSFastMutex gLocalMutexes[2];
static bool gLocalMutexesInitialised;
static Socket::SocketInputStreamLocal* gLocalInputStreams[2];
static Socket::SocketOutputStreamLocal* gLocalOutputStreams[2];
Socket::SocketInputStreamLocal::SocketInputStreamLocal(int i):mIndex(i),mOpen(true){}
Socket::SocketInputStreamLocal::~SocketInputStreamLocal() = default;
int Socket::SocketInputStreamLocal::read()
{
    while (mOpen && ShutdownManager::ShouldRun(ShutdownManager::eThreadId_ConnectionRead))
    {
        if (TryEnterCriticalSection(&gLocalMutexes[mIndex]))
        {
            if (!gLocalQueues[mIndex].empty())
            {
                int value = gLocalQueues[mIndex].front();
                gLocalQueues[mIndex].pop_front();
                LeaveCriticalSection(&gLocalMutexes[mIndex]);
                return value;
            }
            LeaveCriticalSection(&gLocalMutexes[mIndex]);
        }
        Sleep(1);
    }
    return -1;
}
int Socket::SocketInputStreamLocal::read(arrayWithLength<unsigned char> d){return read(d,0,d.length);}
int Socket::SocketInputStreamLocal::read(arrayWithLength<unsigned char> d,unsigned int o,unsigned int n)
{
    while (mOpen)
    {
        if (TryEnterCriticalSection(&gLocalMutexes[mIndex]))
        {
            if (n <= gLocalQueues[mIndex].size())
            {
                for (unsigned int i=0; i<n; ++i) {
                    d.data[o+i] = gLocalQueues[mIndex].front();
                    gLocalQueues[mIndex].pop_front();
                }
                LeaveCriticalSection(&gLocalMutexes[mIndex]);
                return n;
            }
            LeaveCriticalSection(&gLocalMutexes[mIndex]);
        }
        Sleep(1);
    }
    return -1;
}
void Socket::SocketInputStreamLocal::close()
{
    mOpen=false;
    EnterCriticalSection(&gLocalMutexes[mIndex]);
    LeaveCriticalSection(&gLocalMutexes[mIndex]);
}
long long Socket::SocketInputStreamLocal::skip(long long n){return n;}
void Socket::SocketInputStreamLocal::flush(){ return; }
Socket::SocketInputStreamNetwork::SocketInputStreamNetwork(Socket* s,int i):mIndex(i),mOpen(true),mSocket(s){}
Socket::SocketInputStreamNetwork::~SocketInputStreamNetwork() = default;
int Socket::SocketInputStreamNetwork::read()
{
    if (!mOpen || !mSocket || mSocket->mQueues[mIndex].empty()) return -1;
    int value = mSocket->mQueues[mIndex].front();
    mSocket->mQueues[mIndex].pop_front();
    return value;
}
int Socket::SocketInputStreamNetwork::read(arrayWithLength<unsigned char> d){return read(d,0,d.length);}
int Socket::SocketInputStreamNetwork::read(arrayWithLength<unsigned char> d,unsigned int o,unsigned int n)
{
    if (!mOpen || !mSocket || n > mSocket->mQueues[mIndex].size()) return -1;
    for (unsigned int i=0; i<n; ++i) {
        d.data[o+i] = mSocket->mQueues[mIndex].front();
        mSocket->mQueues[mIndex].pop_front();
    }
    return n;
}
void Socket::SocketInputStreamNetwork::close(){mOpen=false;}
long long Socket::SocketInputStreamNetwork::skip(long long n){return n;}
void Socket::SocketInputStreamNetwork::flush(){ return; }
Socket::SocketOutputStream::~SocketOutputStream() = default;
void Socket::SocketOutputStream::writeWithFlags(arrayWithLength<unsigned char> data,
                                                unsigned int,
                                                unsigned int,
                                                int)
{
    write(data);
}
Socket::SocketOutputStreamLocal::SocketOutputStreamLocal(int i):mIndex(i),mOpen(true){}
Socket::SocketOutputStreamLocal::~SocketOutputStreamLocal() = default;
void Socket::SocketOutputStreamLocal::write(unsigned int v)
{
    EnterCriticalSection(&gLocalMutexes[mIndex]);
    if (mOpen)
        gLocalQueues[mIndex].push_back((unsigned char)v);
    LeaveCriticalSection(&gLocalMutexes[mIndex]);
}
void Socket::SocketOutputStreamLocal::write(arrayWithLength<unsigned char> d){write(d,0,d.length);}
void Socket::SocketOutputStreamLocal::write(arrayWithLength<unsigned char> d,unsigned int o,unsigned int n)
{
    EnterCriticalSection(&gLocalMutexes[mIndex]);
    if (mOpen && d.data)
        for (unsigned int i=0;i<n;++i) gLocalQueues[mIndex].push_back(d.data[o+i]);
    LeaveCriticalSection(&gLocalMutexes[mIndex]);
}
void Socket::SocketOutputStreamLocal::close()
{
    mOpen=false;
    EnterCriticalSection(&gLocalMutexes[mIndex]);
    LeaveCriticalSection(&gLocalMutexes[mIndex]);
}
void Socket::SocketOutputStreamLocal::flush(){ return; }
Socket::SocketOutputStreamNetwork::SocketOutputStreamNetwork(Socket* s,int i):mIndex(i),mOpen(true),mSocket(s){}
Socket::SocketOutputStreamNetwork::~SocketOutputStreamNetwork() = default;
void Socket::SocketOutputStreamNetwork::write(unsigned int v){unsigned char b=(unsigned char)v;arrayWithLength<unsigned char> d;d.data=&b;d.length=1;writeWithFlags(d,0,1,0);}
void Socket::SocketOutputStreamNetwork::write(arrayWithLength<unsigned char> d){write(d,0,d.length);}
void Socket::SocketOutputStreamNetwork::write(arrayWithLength<unsigned char> d,unsigned int o,unsigned int n){writeWithFlags(d,o,n,0);}
void Socket::SocketOutputStreamNetwork::writeWithFlags(arrayWithLength<unsigned char> d,unsigned int o,unsigned int n,int)
{
    if (mOpen && mSocket && d.data && n)
        mSocket->pushDataToQueue(d.data+o,n,mIndex!=0);
}
void Socket::SocketOutputStreamNetwork::close(){mOpen=false;}
void Socket::SocketOutputStreamNetwork::flush(){ return; }
Socket::Socket(bool b)
    : mLocal(true),mNetwork(true),mIndex(b?1:0),mPlayerId(0),mEvent(NULL),mOpen(true)
{
    if (!b)
        sConnection->NewIncomingSocket(new Socket(true));
    INetworkPlayer* hostPlayer = CGameNetworkManager::sInstance.GetHostPlayer();
    if (hostPlayer != NULL)
        mPlayerId = hostPlayer->GetSmallId();
    mInputStreams[0]=gLocalInputStreams[0]; mInputStreams[1]=gLocalInputStreams[1];
    mOutputStreams[0]=gLocalOutputStreams[0]; mOutputStreams[1]=gLocalOutputStreams[1];
    mInputClosed[0]=mInputClosed[1]=false;
    mOutputClosed[0]=mOutputClosed[1]=false;
}
Socket::Socket(INetworkPlayer* p,bool a,bool b)
    : mLocal(false),mNetwork(b),mIndex(a?1:0),mPlayerId(0),mEvent(NULL),mOpen(true)
{
    mInputStreams[0]=new SocketInputStreamNetwork(this,0);
    mInputStreams[1]=new SocketInputStreamNetwork(this,1);
    mOutputStreams[0]=new SocketOutputStreamNetwork(this,0);
    mOutputStreams[1]=new SocketOutputStreamNetwork(this,1);
    if (p) mPlayerId=p->GetSmallId();
}
INetworkPlayer* Socket::getPlayer(){return CGameNetworkManager::sInstance.GetPlayerBySmallId(mPlayerId);}
void Socket::setPlayer(INetworkPlayer* p){mPlayerId=p?p->GetSmallId():0;}
void Socket::Initialise(ServerConnection* c)
{
    sConnection = c;
    if (gLocalMutexesInitialised)
    {
        if (TryEnterCriticalSection(&gLocalMutexes[0]))
        {
            gLocalQueues[0].clear();
            LeaveCriticalSection(&gLocalMutexes[0]);
        }
        gLocalOutputStreams[0]->mOpen = true;
        gLocalInputStreams[0]->mOpen = true;
        if (TryEnterCriticalSection(&gLocalMutexes[1]))
        {
            gLocalQueues[1].clear();
            LeaveCriticalSection(&gLocalMutexes[1]);
        }
        gLocalOutputStreams[1]->mOpen = true;
        gLocalInputStreams[1]->mOpen = true;
        return;
    }
    gLocalMutexesInitialised = true;
    InitializeCriticalSection(&gLocalMutexes[0]);
    gLocalOutputStreams[0] = new SocketOutputStreamLocal(0);
    gLocalInputStreams[0] = new SocketInputStreamLocal(0);
    InitializeCriticalSection(&gLocalMutexes[1]);
    gLocalOutputStreams[1] = new SocketOutputStreamLocal(1);
    gLocalInputStreams[1] = new SocketInputStreamLocal(1);
}
unsigned int Socket::getRemoteSocketAddress(){return 0;}
void Socket::pushDataToQueue(const unsigned char* d,unsigned int n,bool b)
{
    if (!d) return;
    unsigned int i=b?1:0;
    for (unsigned int j=0;j<n;++j) mQueues[i].push_back(d[j]);
}
void Socket::addIncomingSocket(Socket* s){if(sConnection)sConnection->NewIncomingSocket(s);}
InputStream* Socket::getInputStream(bool b)
{
    if (mLocal)
        return gLocalInputStreams[mIndex];
    return mInputStreams[mNetwork ? (b ? 1 : 0) : mIndex];
}
void Socket::setSoTimeout(int timeout){ (void)timeout; return; }
void Socket::setTrafficClass(int trafficClass){ (void)trafficClass; return; }
OutputStream* Socket::getOutputStream(bool b)
{
    if (mLocal)
        return gLocalOutputStreams[1 - mIndex];
    return mOutputStreams[mNetwork ? (b ? 1 : 0) : mIndex];
}
bool Socket::close(bool b){unsigned int i=b?1:0;if(mInputClosed[i]&&mOutputClosed[i])return false;mInputClosed[i]=true;mOutputClosed[i]=true;mOpen=false;return true;}
