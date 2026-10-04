#include "vendor/__N_22_chat_VoiceProtocol_cpp_bf22a0fe.hpp"
#include <new>
#include "game/core/CriticalSection.hpp"

extern OSFastMutex g_voiceProtocolMutex;
void __N_22_chat_VoiceProtocol_cpp_bf22a0fe::lock_()
{
    EnterCriticalSection(&g_voiceProtocolMutex);
}

void __N_22_chat_VoiceProtocol_cpp_bf22a0fe::unlock_()
{
    LeaveCriticalSection(&g_voiceProtocolMutex);
}

bool __N_22_chat_VoiceProtocol_cpp_bf22a0fe::isOpus(nn::pia::chat::Codec codec)
{
    return codec == nn::pia::chat::CODEC_5 || codec == nn::pia::chat::CODEC_6;
}

TMC_HF_EC_MODE __N_22_chat_VoiceProtocol_cpp_bf22a0fe::convertEcMode(nn::pia::chat::EcMode mode)
{
    static const unsigned int conversion[4] = { 0, 1, 2, 0 };
    return (TMC_HF_EC_MODE)conversion[(unsigned int)mode];
}

int __N_22_chat_VoiceProtocol_cpp_bf22a0fe::convertCid(unsigned short cid)
{
    if (cid == 0)
        return 0xfd;
    return (int)cid - 1;
}

nn::pia::StationIndex __N_22_chat_VoiceProtocol_cpp_bf22a0fe::convertStationIndex(nn::pia::StationIndex index)
{
    return static_cast<nn::pia::StationIndex>(static_cast<unsigned short>(index + 1));
}

nn::pia::Result __N_22_chat_VoiceProtocol_cpp_bf22a0fe::createProtocol(nn::pia::transport::UnreliableProtocol **protocol, unsigned short *protocolId, unsigned char channel)
{
    if (*protocolId != 0)
    {
        if (protocol != 0)
        {
            *protocol = 0;
        }
        return 0x10c08;
    }
    if (*protocol != 0)
        return 0x10c08;
    if (nn::pia::transport::g_piaProtocolManager == 0)
        return 0x10c08;
    nn::pia::transport::UnreliableProtocol *allocated = nn::pia::transport::g_piaProtocolManager->AllocProtocol(0x20);
    if (allocated != 0)
        new (allocated) nn::pia::transport::UnreliableProtocol();
    nn::pia::transport::ProtocolId id = (unsigned short)(0x6800 | channel);
    unsigned short created = nn::pia::transport::g_piaProtocolManager->CreateProtocolImpl(allocated, id);
    *protocolId = created;
    if (created != 0)
    {
        *protocol = (nn::pia::transport::UnreliableProtocol *)nn::pia::transport::g_piaProtocolManager->SearchProtocol(created, 0x68);
        return 0;
    }
    *protocol = 0;
    return 0x10c07;
}

void __N_22_chat_VoiceProtocol_cpp_bf22a0fe::destroyProtocol(nn::pia::transport::UnreliableProtocol **protocol, unsigned short *protocolId)
{
    if (*protocol != 0)
        (*protocol)->Finalize();
    if (nn::pia::transport::g_piaProtocolManager != 0 && *protocolId != 0)
    {
        nn::pia::transport::g_piaProtocolManager->DestroyProtocol(*protocolId);
        *protocolId = 0;
        *protocol = 0;
    }
}

VCTCodec __N_22_chat_VoiceProtocol_cpp_bf22a0fe::convertCodec(nn::pia::chat::Codec codec)
{
    return (VCTCodec)(unsigned int)codec;
}

nn::pia::chat::Codec __N_22_chat_VoiceProtocol_cpp_bf22a0fe::convertCodec(VCTCodec codec)
{
    return (nn::pia::chat::Codec)(unsigned int)codec;
}

nn::pia::chat::EcMode __N_22_chat_VoiceProtocol_cpp_bf22a0fe::convertEcMode(TMC_HF_EC_MODE mode)
{
    static const unsigned int conversion[3] = { 0, 1, 2 };
    return (nn::pia::chat::EcMode)conversion[(unsigned int)mode];
}
