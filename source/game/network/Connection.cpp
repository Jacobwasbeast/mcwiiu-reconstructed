#include "game/network/Connection.hpp"
#include "game/core/BufferedOutputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/Compression.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/ShutdownManager.hpp"
#include "game/core/Socket.hpp"
#include "game/core/System.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"
#include "platform/wiiu/C4JEventImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

Connection::Connection(Socket* socket, const std::wstring& name, PacketListener* listener)
    : m_socket(socket), m_remoteAddress(0), m_input(0), m_output(0), m_packetOutput(0), m_packetBuffer(0), m_rawOutput(0), m_readThread(0), m_writeThread(0), m_running(true), m_unknown48(0), m_unknown88(0), m_delayedPackets(0), m_listener(listener), m_sendAndQuit(false), m_compressionError(false), m_readEvent(0), m_writeEvent(0), m_closeEvent(0), m_sendEvent(0), m_packetsRead(0), m_packetsWritten(0), m_closing(0), m_unknownD5(0), m_disconnectReason(0), m_disconnectData(0), m_idleTicks(0), m_bytesWritten(0), m_bytesRead(0), m_writeDelay(0), m_unknown11C(0), m_lastSendTime(0)
{
    _init();
    m_remoteAddress = socket->getRemoteSocketAddress();
    socket->setSoTimeout(30000);
    socket->setTrafficClass(24);
    bool server = listener->isServerPacketListener();
    m_input = new DataInputStream(socket->getInputStream(server));
    m_rawOutput = socket->getOutputStream(server);
    m_output = new DataOutputStream(new BufferedOutputStream(m_rawOutput, 5120));
    m_packetBuffer = new ByteArrayOutputStream(5120);
    m_packetOutput = new DataOutputStream(m_packetBuffer);
    m_readEvent = new C4JEventImpl(C4JEvent::Mode0);
    m_writeEvent = new C4JEventImpl(C4JEvent::Mode0);
    m_readThread = new C4JThreadImpl((int (*)(void*))(void*)&Connection::runRead, this, "Connection read", 0);
    m_writeThread = new C4JThreadImpl((int (*)(void*))(void*)&Connection::runWrite, this, "Connection write", 0);
    static_cast<C4JThreadImpl*>(m_readThread)->Run();
    static_cast<C4JThreadImpl*>(m_writeThread)->Run();
}

Connection::~Connection()
{
    m_running = false;
    DeleteCriticalSection(&m_runningMutex);
    if (m_input != 0)
        m_input->close();
    static_cast<C4JThreadImpl*>(m_readThread)->WaitForCompletion(-1);
    static_cast<C4JThreadImpl*>(m_writeThread)->WaitForCompletion(-1);
    DeleteCriticalSection(&m_queueMutex);
    DeleteCriticalSection(&m_threadMutex);
    DeleteCriticalSection(&m_readMutex);
    delete m_readEvent;
    delete m_writeEvent;
    delete m_packetOutput;
    m_packetOutput = 0;
    delete m_packetBuffer;
    m_packetBuffer = 0;
    if (m_output != 0)
    {
        m_output->deleteChildStream();
        delete m_output;
        m_output = 0;
    }
    delete m_input;
    m_input = 0;
    delete static_cast<C4JThreadImpl*>(m_readThread);
    m_readThread = 0;
    delete static_cast<C4JThreadImpl*>(m_writeThread);
    m_writeThread = 0;
}

void Connection::_init()
{
    InitializeCriticalSection(&m_queueMutex);
    InitializeCriticalSection(&m_threadMutex);
    InitializeCriticalSection(&m_readMutex);
    m_running = true;
    InitializeCriticalSection(&m_runningMutex);
    m_sendAndQuit = false;
    m_compressionError = false;
    m_closing = 0;
    m_disconnectReason = 0;
    m_disconnectData = 0;
    m_idleTicks = 0;
    m_bytesWritten = 0;
    m_bytesRead = 0;
    m_packetsRead = 0;
    m_packetsWritten = 0;
    m_delayedPackets = 0;
    m_writeDelay = 50;
}

uint8_t Connection::getAndSetRunning(bool running)
{
    EnterCriticalSection(&m_runningMutex);
    bool old = m_running;
    m_running = running;
    LeaveCriticalSection(&m_runningMutex);
    return old ? 1 : 0;
}

uint32_t Connection::readTick()
{
    if (m_input == 0 || m_compressionError) return 0;
    bool protocolError = false;
    boost::shared_ptr<Packet> packet = Packet::readPacket(m_input, m_listener->isServerPacketListener(), m_packetsRead, protocolError);
    if (packet != 0) {
        EnterCriticalSection(&m_readMutex);
        if (!m_sendAndQuit) m_receivedPackets.push_back(packet);
        LeaveCriticalSection(&m_readMutex);
        return 1;
    }
    if (protocolError) m_compressionError = true;
    return 0;
}

uint32_t Connection::runRead(int32_t argument)
{
    ShutdownManager::HasStarted(ShutdownManager::eThreadId_ConnectionRead);
    Connection* connection = (Connection*)(intptr_t)argument;
    if (connection != 0) {
        Compression::CreateNewThreadStorage();
        while (connection->m_running && !connection->m_compressionError && ShutdownManager::ShouldRun(ShutdownManager::eThreadId_ConnectionRead)) {
            if (connection->readTick() == 0) connection->m_readEvent->WaitForSignal(100);
        }
        Compression::ReleaseThreadStorage();
    }
    ShutdownManager::HasFinished(ShutdownManager::eThreadId_ConnectionRead);
    return 0;
}

uint32_t Connection::writeTick()
{
    if (m_output == 0 || m_packetOutput == 0) return 0;
    boost::shared_ptr<Packet> packet;
    EnterCriticalSection(&m_queueMutex);
    if (!m_sendQueue.empty()) { packet = m_sendQueue.front(); m_sendQueue.pop_front(); }
    else if (!m_delayedQueue.empty()) { packet = m_delayedQueue.front(); m_delayedQueue.pop_front(); }
    LeaveCriticalSection(&m_queueMutex);
    if (packet == 0) return 0;
    Packet::writePacket(packet, m_output, m_listener->isServerPacketListener(), m_packetsWritten);
    return 1;
}

uint32_t Connection::runWrite(int32_t argument)
{
    ShutdownManager::HasStarted(ShutdownManager::eThreadId_ConnectionWrite);
    Connection* connection = (Connection*)(intptr_t)argument;
    if (connection != 0) {
        Compression::CreateNewThreadStorage();
        while (connection->m_running && ShutdownManager::ShouldRun(ShutdownManager::eThreadId_ConnectionWrite)) {
            connection->writeTick();
            connection->m_writeEvent->WaitForSignal(100);
            if (connection->m_output != 0)
                connection->m_output->flush();
        }
        Compression::ReleaseThreadStorage();
    }
    ShutdownManager::HasFinished(ShutdownManager::eThreadId_ConnectionWrite);
    return 0;
}

void Connection::setListener(PacketListener* listener) { m_listener = listener; }

void Connection::send(boost::shared_ptr<Packet> packet)
{
    if (m_sendAndQuit) return;
    m_lastSendTime = System::processTimeInMilliSecs();
    EnterCriticalSection(&m_queueMutex);
    if (packet != 0) {
        m_bytesWritten += packet->getEstimatedSize() + 1;
        if (packet->isAync()) m_delayedQueue.push_back(packet); else m_sendQueue.push_back(packet);
    }
    LeaveCriticalSection(&m_queueMutex);
}

uint64_t Connection::queueSend(boost::shared_ptr<Packet> packet)
{
    if (!m_sendAndQuit) {
        EnterCriticalSection(&m_queueMutex);
        if (packet != 0) { m_bytesWritten += packet->getEstimatedSize() + 1; m_delayedQueue.push_back(packet); }
        LeaveCriticalSection(&m_queueMutex);
    }
    return (uint64_t)(uintptr_t)this;
}

void Connection::flush()
{
    m_readEvent->Set();
    m_writeEvent->Set();
}

void Connection::close(DisconnectPacket::eDisconnectReason reason)
{
    if (!getAndSetRunning(false))
        return;
    m_disconnectReason = reason;
    m_disconnectData = 0;
    m_closing = 1;
    if (m_input != 0)
        m_input->close();
    if (m_readThread != 0)
        static_cast<C4JThreadImpl*>(m_readThread)->WaitForCompletion(-1);
    if (m_writeThread != 0)
        static_cast<C4JThreadImpl*>(m_writeThread)->WaitForCompletion(-1);
    delete m_input;
    m_input = 0;
    if (m_output != 0)
    {
        m_output->close();
        m_output->deleteChildStream();
        delete m_output;
        m_output = 0;
    }
    if (m_packetOutput != 0)
    {
        m_packetOutput->close();
        delete m_packetOutput;
        m_packetOutput = 0;
    }
    if (m_socket != 0)
    {
        m_socket->close(m_listener->isServerPacketListener());
        m_socket = 0;
    }
}

void Connection::tick()
{
    if (m_compressionError) close(DisconnectPacket::eDisconnectReason_BadPacket);
    if (m_bytesWritten > 0x100000) close(DisconnectPacket::eDisconnectReason_SendQueueOverflow);
    EnterCriticalSection(&m_readMutex);
    if (m_receivedPackets.empty()) ++m_idleTicks; else m_idleTicks = 0;
    while (!m_receivedPackets.empty()) {
        boost::shared_ptr<Packet> packet = m_receivedPackets.front();
        m_receivedPackets.pop_front();
        if (packet != 0) packet->handle(m_listener);
    }
    LeaveCriticalSection(&m_readMutex);
    flush();
}

void Connection::sendAndQuit()
{
    if (!m_sendAndQuit) { flush(); m_sendAndQuit = true; close(DisconnectPacket::eDisconnectReason_ConnectionClosed); }
}

uint32_t Connection::countDelayedPackets() { return m_delayedPackets; }
