#include "game/network/Connection.hpp"
#include "game/core/Compression.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/ShutdownManager.hpp"
#include "game/core/System.hpp"

Connection::Connection(Socket* socket, const std::wstring& address,
                       PacketListener* listener)
    : m_socket(socket), m_remoteAddress(0), m_input(0), m_output(0),
      m_packetOutput(0), m_readThread(0), m_writeThread(0), m_running(true),
      m_sendAndQuit(false), m_compressionError(false), m_readEvent(0),
      m_writeEvent(0), m_closeEvent(0), m_sendEvent(0), m_packetsRead(0),
      m_packetsWritten(0), m_closing(0), m_disconnectReason(0),
      m_disconnectData(0), m_idleTicks(0), m_bytesWritten(0), m_bytesRead(0),
      m_writeDelay(0), m_flushDelay(0), m_lastSendTime(0),
      m_listener(listener)
{
    _init();
}

void Connection::_init()
{
    InitializeCriticalSection(&m_runningMutex);
    InitializeCriticalSection(&m_readMutex);
    InitializeCriticalSection(&m_threadMutex);
    m_running = true;
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
    m_writeDelay = 0;
    m_flushDelay = 50;
}

Connection::~Connection()
{
    m_running = false;
    DeleteCriticalSection(&m_runningMutex);
    DeleteCriticalSection(&m_readMutex);
    DeleteCriticalSection(&m_threadMutex);
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
    ++m_packetsRead;
    return 0;
}

uint32_t Connection::runRead(int32_t value)
{
    ShutdownManager::HasStarted(ShutdownManager::eThreadId_ConnectionRead);
    Connection* connection = (Connection*)value;
    if (connection != 0) {
        Compression::CreateNewThreadStorage();
        while (connection->m_running && !connection->m_sendAndQuit &&
               ShutdownManager::ShouldRun(ShutdownManager::eThreadId_ConnectionRead)) {
            if (connection->readTick() == 0) break;
        }
        Compression::ReleaseThreadStorage();
    }
    ShutdownManager::HasFinished(ShutdownManager::eThreadId_ConnectionRead);
    return 0;
}

uint32_t Connection::writeTick()
{
    if (m_input == 0 || m_output == 0) return 0;
    EnterCriticalSection(&m_threadMutex);
    if (m_sendQueue.empty()) {
        LeaveCriticalSection(&m_threadMutex);
        return 0;
    }
    boost::shared_ptr<Packet> packet = m_sendQueue.front();
    m_sendQueue.pop_front();
    LeaveCriticalSection(&m_threadMutex);
    if (packet) Packet::writePacket(packet, m_output, false, m_packetsWritten);
    return 1;
}

uint32_t Connection::runWrite(int32_t value)
{
    ShutdownManager::HasStarted(ShutdownManager::eThreadId_ConnectionWrite);
    Connection* connection = (Connection*)value;
    if (connection != 0) {
        Compression::CreateNewThreadStorage();
        while (connection->m_running && !connection->m_sendAndQuit &&
               ShutdownManager::ShouldRun(ShutdownManager::eThreadId_ConnectionWrite)) {
            if (connection->writeTick() == 0) break;
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
    EnterCriticalSection(&m_threadMutex);
    if (packet) {
        m_bytesWritten += packet->getEstimatedSize() + 1;
        if (packet->isAync()) m_sendQueue.push_back(packet);
        else m_receivedPackets.push_back(packet);
    }
    LeaveCriticalSection(&m_threadMutex);
}

uint64_t Connection::queueSend(boost::shared_ptr<Packet> packet)
{
    if (!m_sendAndQuit) {
        EnterCriticalSection(&m_threadMutex);
        if (packet) {
            m_bytesWritten += packet->getEstimatedSize() + 1;
            m_sendQueue.push_back(packet);
        }
        LeaveCriticalSection(&m_threadMutex);
    }
    return (uint64_t)this;
}

void Connection::flush()
{
    if (m_output != 0) m_output->flush();
    if (m_packetOutput != 0) m_packetOutput->flush();
}

void Connection::close(int reason)
{
    if (getAndSetRunning(false) == 0) return;
    m_disconnectReason = reason;
    m_disconnectData = 0;
    m_closing = 1;
    if (m_input != 0) { m_input->close(); m_input = 0; }
    if (m_output != 0) { m_output->close(); m_output = 0; }
    if (m_packetOutput != 0) { m_packetOutput->close(); m_packetOutput = 0; }
    m_socket = 0;
}

void Connection::tick()
{
    if (m_closing) return;
    EnterCriticalSection(&m_readMutex);
    while (!m_receivedPackets.empty()) {
        boost::shared_ptr<Packet> packet = m_receivedPackets.front();
        m_receivedPackets.pop_front();
        if (packet != 0 && m_listener != 0) packet->handle(m_listener);
    }
    LeaveCriticalSection(&m_readMutex);
    flush();
}

void Connection::sendAndQuit()
{
    if (!m_sendAndQuit) {
        flush();
        m_sendAndQuit = true;
        close(2);
    }
}

uint32_t Connection::countDelayedPackets() { return m_delayedPackets; }
void* Connection::getRemoteAddress() { return m_remoteAddress; }

uint32_t Connection::runClose(int32_t value)
{
    Connection* connection = (Connection*)value;
    if (connection != 0) connection->close(2);
    return 0;
}

uint32_t Connection::runSendAndQuit(int32_t value)
{
    Connection* connection = (Connection*)value;
    if (connection != 0) connection->sendAndQuit();
    return 0;
}
