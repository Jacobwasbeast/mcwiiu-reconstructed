#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RELIABLESLIDINGWINDOW_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RELIABLESLIDINGWINDOW_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/ISerializable.hpp"

namespace nn { namespace pia { namespace transport {

class StationPacketHandler;
class ProtocolMessageReader;

class ReliableSlidingWindow
{
public:
    enum { cHeaderSize = 24, cPayloadSizeMax = 1384 };

    class MessageHeader : public common::ISerializable
    {
    public:
        virtual ~MessageHeader()
        {
        }

        virtual Result Serialize(unsigned char* buffer, unsigned int* size) const
        {
            if (buffer == 0)
                return static_cast<Result>(cResultInvalidArgument);
            if (size == 0)
                return static_cast<Result>(cResultInvalidArgument);

            buffer[0] = (m_flags >> 8) & 0xff;
            buffer[1] = m_flags & 0xff;
            buffer[2] = (m_size >> 8) & 0xff;
            buffer[3] = m_size & 0xff;
            buffer[4] = (m_field08 >> 24) & 0xff;
            buffer[5] = (m_field08 >> 16) & 0xff;
            buffer[6] = (m_field08 >> 8) & 0xff;
            buffer[7] = m_field08 & 0xff;
            buffer[8] = (m_sequenceId >> 24) & 0xff;
            buffer[9] = (m_sequenceId >> 16) & 0xff;
            buffer[10] = (m_sequenceId >> 8) & 0xff;
            buffer[11] = m_sequenceId & 0xff;
            buffer[12] = (m_field10 >> 24) & 0xff;
            buffer[13] = (m_field10 >> 16) & 0xff;
            buffer[14] = (m_field10 >> 8) & 0xff;
            buffer[15] = m_field10 & 0xff;
            buffer[16] = (m_field18 >> 56) & 0xff;
            buffer[17] = (m_field18 >> 48) & 0xff;
            buffer[18] = (m_field18 >> 40) & 0xff;
            buffer[19] = (m_field18 >> 32) & 0xff;
            buffer[20] = (m_field18 >> 24) & 0xff;
            buffer[21] = (m_field18 >> 16) & 0xff;
            buffer[22] = (m_field18 >> 8) & 0xff;
            buffer[23] = m_field18 & 0xff;
            *size = cHeaderSize;
            return 0;
        }

        virtual Result Deserialize(const unsigned char* buffer)
        {
            if (buffer == 0)
                return static_cast<Result>(cResultInvalidArgument);

            m_flags = (buffer[0] << 8) | buffer[1];
            m_size = (buffer[2] << 8) | buffer[3];
            m_field08 = static_cast<unsigned int>(buffer[4]) << 24 |
                static_cast<unsigned int>(buffer[5]) << 16 |
                static_cast<unsigned int>(buffer[6]) << 8 | buffer[7];
            m_sequenceId = static_cast<unsigned int>(buffer[8]) << 24 |
                static_cast<unsigned int>(buffer[9]) << 16 |
                static_cast<unsigned int>(buffer[10]) << 8 | buffer[11];
            m_field10 = static_cast<unsigned int>(buffer[12]) << 24 |
                static_cast<unsigned int>(buffer[13]) << 16 |
                static_cast<unsigned int>(buffer[14]) << 8 | buffer[15];
            m_field18 = static_cast<unsigned long long>(buffer[16]) << 56 |
                static_cast<unsigned long long>(buffer[17]) << 48 |
                static_cast<unsigned long long>(buffer[18]) << 40 |
                static_cast<unsigned long long>(buffer[19]) << 32 |
                static_cast<unsigned long long>(buffer[20]) << 24 |
                static_cast<unsigned long long>(buffer[21]) << 16 |
                static_cast<unsigned long long>(buffer[22]) << 8 | buffer[23];
            return 0;
        }

        virtual unsigned int GetSerializedSize() const
        {
            return cHeaderSize;
        }

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

private:
    static float s_retransmissionTimeOutRate;
    static unsigned int s_rttSamplingNum;
    static int (*s_resendTimeCallback)(int, int);
};

extern unsigned short g_reliableSlidingWindowGeneration;

}}}

#endif
