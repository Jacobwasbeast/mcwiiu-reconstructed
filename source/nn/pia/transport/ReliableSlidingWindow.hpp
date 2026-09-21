#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RELIABLESLIDINGWINDOW_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RELIABLESLIDINGWINDOW_HPP

#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace transport {

class StationPacketHandler;
class ProtocolMessageReader;

class ReliableSlidingWindow
{
public:
    enum { cHeaderSize = 24, cPayloadSizeMax = 0x568 };

    class MessageHeader
    {
    public:
        virtual ~MessageHeader();
        virtual Result Serialize(unsigned char* buffer, unsigned int* size) const;
        virtual Result Deserialize(const unsigned char* buffer);
        virtual unsigned int GetSerializedSize() const;

        unsigned short m_flags;
        unsigned short m_size;
        unsigned int m_field08;
        unsigned int m_sequenceId;
        unsigned int m_field10;
        unsigned long long m_field18;
    };

    class MessageData
    {
    public:
        void DeserializeHeader(const void* buffer);
        void SerializeHeader();
        static void CopyPayload(void* destination, const void* source, unsigned int size);
        void SetPayload(const void* data, unsigned int size);

        MessageHeader m_header;
        unsigned char m_data[cHeaderSize + cPayloadSizeMax];
    };

    struct SendData
    {
        unsigned long long m_sendTime;
        unsigned int m_size;
        unsigned short m_field0c;
        MessageData m_message;
    };

    struct ReceiveData
    {
        bool m_valid;
        bool m_end;
        unsigned short m_size;
        unsigned char m_payload[cPayloadSizeMax];
    };

    template <typename T> class Buffer
    {
    public:
        Buffer();
        ~Buffer();
        Result Initialize(unsigned int capacity);
        void Finalize();
        void Clear();
        void Withdraw(unsigned int count);
        T* GetData(unsigned int sequence);

        T* m_data;
        unsigned int m_capacity;
        unsigned int m_head;
        unsigned int m_sequence;
        unsigned int m_count;
    };

    class SendBuffer : public Buffer<SendData>
    {
    public:
        SendData* Assign();
    };

    class ReceiveBuffer : public Buffer<ReceiveData>
    {
    public:
        ReceiveData* Assign(unsigned int sequence);
    };

    ReliableSlidingWindow();
    virtual ~ReliableSlidingWindow();
    virtual void Trace(unsigned long long value) const;
    Result Initialize(unsigned int sendCapacity, unsigned int receiveCapacity);
    void Finalize();
    void Cleanup();
    bool IsInCommunication() const;
    Result PopData(void* buffer, unsigned int* size, unsigned int bufferSize);
    Result PushData(const void* data, unsigned int size);
    unsigned int CanPushData(unsigned int size);
    Result Startup(StationPacketHandler* handler, unsigned short generation,
                   unsigned long long localStationId, unsigned long long remoteStationId);
    Result AnalyzeProtocolMessage(const ProtocolMessageReader& reader);
    Result Dispatch(StationPacketHandler* handler, unsigned int* size, unsigned int limit);

    unsigned short m_generation;
    unsigned long long m_localStationId;
    unsigned long long m_remoteStationId;
    SendBuffer m_sendBuffer;
    ReceiveBuffer m_receiveBuffer;
    unsigned int m_payloadLimit;
    unsigned long long m_time;
    unsigned long long m_retrySpan;
    unsigned int m_sequence;
    unsigned long long m_receivedBits;
    bool m_field68;
    unsigned int m_byteCount;
    unsigned long long m_profileStart;
};

extern unsigned short g_reliableSlidingWindowGeneration;

}}}

#endif
