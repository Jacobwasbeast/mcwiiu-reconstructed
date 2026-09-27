#ifndef GAME_NETWORK_CONNECTION_HPP
#define GAME_NETWORK_CONNECTION_HPP
#include <deque>
#include <stdint.h>
#include <string>
#include <boost/shared_ptr.hpp>
#include "game/core/CriticalSection.hpp"
#include "game/network/DisconnectPacket.hpp"
class ByteArrayOutputStream;
class DataInputStream;
class DataOutputStream;
class OutputStream;
class Packet;
class PacketListener;
class ServerGamePacketListenerImpl;
class Socket;
class C4JEventImpl;
class C4JThreadImpl;
class Connection
{
public:
    Connection(Socket*, const std::wstring&, PacketListener*);
    ~Connection();
    uint8_t getAndSetRunning(bool);
    void _init();
    uint32_t readTick();
    static int runRead(void*);
    uint32_t writeTick();
    static int runWrite(void*);
    void setListener(PacketListener*);
    void send(boost::shared_ptr<Packet>);
    void queueSend(boost::shared_ptr<Packet>);
    void flush();
    void close(DisconnectPacket::eDisconnectReason);
    void tick();
    void sendAndQuit();
    uint32_t countDelayedPackets();
    void* getRemoteAddress();
    static int runClose(void*);
    static int runSendAndQuit(void*);
private:
    friend class ServerGamePacketListenerImpl;
    friend class ClientPacketListener;
    friend class PlayerList;

    Socket* m_socket;
    unsigned int m_remoteAddress;
    DataInputStream* m_input;
    DataOutputStream* m_output;
    DataOutputStream* m_packetOutput;
    ByteArrayOutputStream* m_packetBuffer;
    OutputStream* m_rawOutput;
    C4JThreadImpl* m_readThread;
    C4JThreadImpl* m_writeThread;
    bool m_running;
    OSFastMutex m_runningMutex;
    unsigned int m_reserved98;
    std::deque<boost::shared_ptr<Packet> > m_receivedPackets;
    OSFastMutex m_readMutex;
    unsigned int m_reserved9C;
    std::deque<boost::shared_ptr<Packet> > m_sendQueue;
    std::deque<boost::shared_ptr<Packet> > m_delayedQueue;
    unsigned int m_delayedPackets;
    PacketListener* m_listener;
    bool m_sendAndQuit;
    bool m_compressionError;
    C4JEventImpl* m_readEvent;
    C4JEventImpl* m_writeEvent;
    C4JEventImpl* m_closeEvent;
    C4JEventImpl* m_sendEvent;
    unsigned int m_packetsRead;
    unsigned int m_packetsWritten;
    unsigned char m_closing;
    unsigned char m_reservedD5;
    unsigned int m_disconnectReason;
    unsigned int m_disconnectData;
    unsigned int m_idleTicks;
    unsigned int m_bytesWritten;
    unsigned int m_bytesRead;
    unsigned int m_writeDelay;
    OSFastMutex m_threadMutex;
    unsigned int m_reservedE0;
    OSFastMutex m_queueMutex;
    uint64_t m_lastSendTime;
};
#endif
