#include "game/core/Socket.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/network/ServerConnection.hpp"
ServerConnection* Socket::sConnection = NULL;
static std::deque<unsigned char> gLocalQueues[2];
Socket::SocketInputStreamLocal::SocketInputStreamLocal(int i):mIndex(i),mOpen(true){}
Socket::SocketInputStreamLocal::~SocketInputStreamLocal(){}
int Socket::SocketInputStreamLocal::read()
{
    if (!mOpen || gLocalQueues[mIndex].empty()) return -1;
    int value = gLocalQueues[mIndex].front();
    gLocalQueues[mIndex].pop_front();
    return value;
}
int Socket::SocketInputStreamLocal::read(arrayWithLength<unsigned char> d){return read(d,0,d.length);}
int Socket::SocketInputStreamLocal::read(arrayWithLength<unsigned char> d,unsigned int o,unsigned int n)
{
    if (!mOpen || n > gLocalQueues[mIndex].size()) return -1;
    for (unsigned int i=0; i<n; ++i) {
        d.data[o+i] = gLocalQueues[mIndex].front();
        gLocalQueues[mIndex].pop_front();
    }
    return n;
}
void Socket::SocketInputStreamLocal::close(){mOpen=false;}
long long Socket::SocketInputStreamLocal::skip(long long n){return n;}
void Socket::SocketInputStreamLocal::flush(){}
Socket::SocketInputStreamNetwork::SocketInputStreamNetwork(Socket* s,int i):mIndex(i),mOpen(true),mSocket(s){}
Socket::SocketInputStreamNetwork::~SocketInputStreamNetwork(){}
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
void Socket::SocketInputStreamNetwork::flush(){}
Socket::SocketOutputStream::~SocketOutputStream(){}
void Socket::SocketOutputStream::writeWithFlags(arrayWithLength<unsigned char>,unsigned int,unsigned int,int){}
Socket::SocketOutputStreamLocal::SocketOutputStreamLocal(int i):mIndex(i),mOpen(true){}
Socket::SocketOutputStreamLocal::~SocketOutputStreamLocal(){}
void Socket::SocketOutputStreamLocal::write(unsigned int v){if(mOpen)gLocalQueues[mIndex].push_back((unsigned char)v);}
void Socket::SocketOutputStreamLocal::write(arrayWithLength<unsigned char> d){write(d,0,d.length);}
void Socket::SocketOutputStreamLocal::write(arrayWithLength<unsigned char> d,unsigned int o,unsigned int n)
{
    if (mOpen && d.data)
        for (unsigned int i=0;i<n;++i) gLocalQueues[mIndex].push_back(d.data[o+i]);
}
void Socket::SocketOutputStreamLocal::close(){mOpen=false;}
void Socket::SocketOutputStreamLocal::flush(){}
Socket::SocketOutputStreamNetwork::SocketOutputStreamNetwork(Socket* s,int i):mIndex(i),mOpen(true),mSocket(s){}
Socket::SocketOutputStreamNetwork::~SocketOutputStreamNetwork(){}
void Socket::SocketOutputStreamNetwork::write(unsigned int v){unsigned char b=(unsigned char)v;arrayWithLength<unsigned char> d;d.data=&b;d.length=1;writeWithFlags(d,0,1,0);}
void Socket::SocketOutputStreamNetwork::write(arrayWithLength<unsigned char> d){write(d,0,d.length);}
void Socket::SocketOutputStreamNetwork::write(arrayWithLength<unsigned char> d,unsigned int o,unsigned int n){writeWithFlags(d,o,n,0);}
void Socket::SocketOutputStreamNetwork::writeWithFlags(arrayWithLength<unsigned char> d,unsigned int o,unsigned int n,int)
{
    if (mOpen && mSocket && d.data && n)
        mSocket->pushDataToQueue(d.data+o,n,mIndex!=0);
}
void Socket::SocketOutputStreamNetwork::close(){mOpen=false;}
void Socket::SocketOutputStreamNetwork::flush(){}
Socket::Socket(bool b)
    : mLocal(true),mNetwork(true),mIndex(b?1:0),mPlayerId(0),mEvent(NULL),mOpen(true)
{
    mInputStreams[0]=mInputStreams[1]=NULL;
    mOutputStreams[0]=mOutputStreams[1]=NULL;
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
void Socket::Initialise(ServerConnection* c){sConnection=c;}
unsigned int Socket::getRemoteSocketAddress(){return 0;}
void Socket::pushDataToQueue(const unsigned char* d,unsigned int n,bool b)
{
    if (!d) return;
    unsigned int i=b?1:0;
    for (unsigned int j=0;j<n;++j) mQueues[i].push_back(d[j]);
}
void Socket::addIncomingSocket(Socket* s){if(sConnection)sConnection->NewIncomingSocket(s);}
InputStream* Socket::getInputStream(bool b){return mInputStreams[b?1:0];}
void Socket::setSoTimeout(int){}
void Socket::setTrafficClass(int){}
OutputStream* Socket::getOutputStream(bool b){return mOutputStreams[b?1:0];}
bool Socket::close(bool b){unsigned int i=b?1:0;if(mInputClosed[i]&&mOutputClosed[i])return false;mInputClosed[i]=true;mOutputClosed[i]=true;mOpen=false;return true;}
