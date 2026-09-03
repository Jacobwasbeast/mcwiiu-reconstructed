#ifndef SOURCE_VENDOR___N_22_CHAT_VOICEPROTOCOL_CPP_BF22A0FE_HPP
#define SOURCE_VENDOR___N_22_CHAT_VOICEPROTOCOL_CPP_BF22A0FE_HPP

#include <stdint.h>

namespace nn
{
namespace pia
{
typedef int Result;
typedef short StationIndex;
namespace chat
{
enum Codec
{
    CODEC_0 = 0,
    CODEC_1 = 1,
    CODEC_2 = 2,
    CODEC_3 = 3,
    CODEC_4 = 4,
    CODEC_5 = 5,
    CODEC_6 = 6
};

enum EcMode
{
    EC_MODE_0 = 0,
    EC_MODE_1 = 1,
    EC_MODE_2 = 2,
    EC_MODE_3 = 3
};
}
namespace transport
{
typedef unsigned short ProtocolId;
class ProtocolManager;
class UnreliableProtocol
{
public:
    UnreliableProtocol();
    void Finalize();
};
class ProtocolManager
{
public:
    UnreliableProtocol *AllocProtocol(unsigned int size);
    unsigned short CreateProtocolImpl(UnreliableProtocol *protocol, ProtocolId id);
    int SearchProtocol(ProtocolId id, unsigned char type);
    void DestroyProtocol(unsigned short id);
};
}
}
}

enum VCTCodec
{
    VCT_CODEC_0 = 0,
    VCT_CODEC_1 = 1,
    VCT_CODEC_2 = 2,
    VCT_CODEC_3 = 3,
    VCT_CODEC_4 = 4,
    VCT_CODEC_5 = 5,
    VCT_CODEC_6 = 6
};

enum TMC_HF_EC_MODE
{
    TMC_HF_EC_MODE_0 = 0,
    TMC_HF_EC_MODE_1 = 1,
    TMC_HF_EC_MODE_2 = 2
};

class __N_22_chat_VoiceProtocol_cpp_bf22a0fe
{
public:
    static void lock_();
    static void unlock_();
    static bool isOpus(nn::pia::chat::Codec codec);
    static TMC_HF_EC_MODE convertEcMode(nn::pia::chat::EcMode mode);
    static int convertCid(unsigned short cid);
    static nn::pia::StationIndex convertStationIndex(nn::pia::StationIndex index);
    static nn::pia::Result createProtocol(nn::pia::transport::UnreliableProtocol **protocol, unsigned short *protocolId, unsigned char channel);
    static void destroyProtocol(nn::pia::transport::UnreliableProtocol **protocol, unsigned short *protocolId);
    static VCTCodec convertCodec(nn::pia::chat::Codec codec);
    static nn::pia::chat::Codec convertCodec(VCTCodec codec);
    static nn::pia::chat::EcMode convertEcMode(TMC_HF_EC_MODE mode);
};

#endif
