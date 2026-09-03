#ifndef GAME_NETWORK_CONNECTION_HPP
#define GAME_NETWORK_CONNECTION_HPP
#include <deque>
#include <string>
#include <stdint.h>
#include <boost/shared_ptr.hpp>
#include "game/core/CriticalSection.hpp"
#include "game/network/Packet.hpp"
class Socket;
class DataOutputStream;
class PacketListener;
class Connection
{
public:
    Connection(Socket*, const std::wstring&, PacketListener*);
    ~Connection();
    uint8_t getAndSetRunning(bool);
    void _init();
    uint32_t readTick();
    static uint32_t runRead(int32_t);
    uint32_t writeTick();
    static uint32_t runWrite(int32_t);
    void setListener(PacketListener*);
    void send(boost::shared_ptr<Packet>);
    uint64_t queueSend(boost::shared_ptr<Packet>);
    void flush();
    void close(int);
    void tick();
    void sendAndQuit();
    uint32_t countDelayedPackets();
    void* getRemoteAddress();
    static uint32_t runClose(int32_t);
    static uint32_t runSendAndQuit(int32_t);
private:
    Socket* m_socket;
    void* m_remoteAddress;
    DataOutputStream* m_input;
    DataOutputStream* m_output;
    DataOutputStream* m_packetOutput;
    void* m_readThread;
    void* m_writeThread;
    bool m_running;
    OSFastMutex m_runningMutex;
    uint32_t m_unknown48;
    std::deque<boost::shared_ptr<Packet> > m_receivedPackets;
    OSFastMutex m_readMutex;
    uint32_t m_unknown88;
    std::deque<boost::shared_ptr<Packet> > m_sendQueue;
    uint32_t m_delayedPackets;
    PacketListener* m_listener;
    bool m_sendAndQuit;
    bool m_compressionError;
    void* m_readEvent;
    void* m_writeEvent;
    void* m_closeEvent;
    void* m_sendEvent;
    uint32_t m_packetsRead;
    uint32_t m_packetsWritten;
    uint8_t m_closing;
    uint8_t m_unknownD5;
    uint8_t m_unknownD6;
    uint8_t m_unknownD7;
    int m_disconnectReason;
    int m_disconnectData;
    uint32_t m_idleTicks;
    uint32_t m_bytesWritten;
    uint32_t m_bytesRead;
    uint32_t m_writeDelay;
    OSFastMutex m_threadMutex;
    uint32_t m_unknown11c;
    uint32_t m_flushDelay;
    uint64_t m_lastSendTime;
    uint32_t m_unknown158;
    uint32_t m_unknown15c;
};
#endif
