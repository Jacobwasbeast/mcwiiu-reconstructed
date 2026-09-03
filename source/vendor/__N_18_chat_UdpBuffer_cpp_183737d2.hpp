#ifndef SOURCE_VENDOR___N_18_CHAT_UDPBUFFER_CPP_183737D2_HPP
#define SOURCE_VENDOR___N_18_CHAT_UDPBUFFER_CPP_183737D2_HPP

struct _udp_packet;

class __N_18_chat_UdpBuffer_cpp_183737d2
{
public:
    static void initImpl();
    static void trace_udp_packet(unsigned long long timestamp, const _udp_packet& packet);
};

#endif
