#include "platform/wiiu/NintendoVoiceChat_WiiU.hpp"

#include <deque>
#include <new>
#include <vector>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "platform/wiiu/NQRNetworkManager_WiiU.hpp"
#include "platform/wiiu/NQRNetworkPlayer.hpp"
#include "platform/wiiu/SndMicDevice.hpp"
#include "vendor/__N_22_chat_VoiceProtocol_cpp_bf22a0fe.hpp"

namespace nn
{
namespace pia
{
namespace chat
{
class VoiceProtocol : public nn::pia::transport::UnreliableProtocol
{
public:
    struct Setting
    {
        unsigned int bufferSize;
        unsigned int maxPlayers;
        unsigned int dataSize;
        unsigned char codec;
        unsigned char reserved[3];
        unsigned int frameLength;
        unsigned short sampleRate;
        unsigned short settingTailPadding;
    };

    VoiceProtocol();
    int GetAudioFrameLength() const;
    unsigned int GetAudioDataSize() const;
    bool IsInCommunication() const;
    unsigned int GetVadActivity() const;
    bool IsClientInConference(unsigned long long stationId) const;
    nn::pia::Result AddConferenceClient(unsigned long long stationId);
    nn::pia::Result RemoveConferenceClient(unsigned long long stationId);
    int Initialize(const Setting& setting);
    int Finalize();
    void ClearStreaming();
};

int Initialize();
int BeginSetup();
int EndSetup();
int Finalize();
}
}
}

extern nn::pia::transport::ProtocolManager* g_piaProtocolManager;
nn::pia::chat::VoiceProtocol* Chat_GetVoiceProtocol();
void ProcUIDrawDoneRelease();
void Assert(const char* expression, const char* file, unsigned int line, const char* function, ...);

void checkPiaCoreToUse()
{
}

void AssertPiaError(nn::pia::Result& result)
{
}

struct VoiceChatMessage
{
    int type;
    int reserved;
    unsigned long long stationId;
};

static bool s_initialised = false;
static bool s_voiceChatEnabled = false;
static unsigned int s_foregroundState = 0;
static NQRNetworkManager_WiiU* s_networkManager = NULL;
static nn::pia::transport::ProtocolManager* s_protocolManager = NULL;
static unsigned short s_protocolId = 0;
static SndMicDevice* s_micDevice = NULL;
static std::vector<unsigned long long> s_conferencePlayers;
static std::vector<NQRNetworkPlayer*> s_voicePlayers;
static std::deque<VoiceChatMessage> s_messageQueue;
static OSFastMutex s_voiceMutex;

nn::pia::chat::VoiceProtocol* Chat_GetVoiceProtocol()
{
    if (s_protocolManager != NULL && s_protocolId != 0)
        return (nn::pia::chat::VoiceProtocol*)s_protocolManager->SearchProtocol(s_protocolId, 0x78);
    return NULL;
}

void ReleaseCallback()
{
    CGameNetworkManager::sInstance.WiiU_ReleaseCallback();
    ProcUIDrawDoneRelease();
}

void NintendoVoiceChat_WiiU::startConference()
{
    checkPiaCoreToUse();
    if (!s_initialised)
    {
        SND_PARAM parameter;
        MIC_STATE_INFO state;
        parameter.eRateMode = eSndRateMode_8kHz;
        parameter.iFrameLengthMs = 0;
        nn::pia::chat::VoiceProtocol* protocol = Chat_GetVoiceProtocol();
        parameter.iFrameLengthMs = protocol->GetAudioFrameLength();
        s_micDevice->InitializeMicSndCommon(&parameter, protocol);
        s_micDevice->GetStateMic(&state);
        s_initialised = true;
    }
}

void NintendoVoiceChat_WiiU::AddPlayer(unsigned long long stationId)
{
    checkPiaCoreToUse();
    EnterCriticalSection(&s_voiceMutex);
    nn::pia::chat::VoiceProtocol* protocol = Chat_GetVoiceProtocol();
    if (s_protocolManager != NULL && protocol != NULL && !protocol->IsClientInConference(stationId))
    {
        startConference();
        nn::pia::Result result = protocol->AddConferenceClient(stationId);
        AssertPiaError(result);
        s_conferencePlayers.push_back(stationId);
    }
    LeaveCriticalSection(&s_voiceMutex);
}

void NintendoVoiceChat_WiiU::stopConference()
{
    checkPiaCoreToUse();
    if (s_initialised)
    {
        nn::pia::chat::VoiceProtocol* protocol = Chat_GetVoiceProtocol();
        for (std::vector<unsigned long long>::const_iterator player = s_conferencePlayers.begin(); player != s_conferencePlayers.end(); ++player)
            protocol->RemoveConferenceClient(*player);
        if (s_micDevice != NULL)
        {
            s_micDevice->StopMic();
            s_micDevice->StopSnd();
            s_micDevice->FinalizeMic();
            s_micDevice->FinalizeMicSndCommon();
        }
        s_initialised = false;
    }
}

void NintendoVoiceChat_WiiU::RemovePlayer(unsigned long long stationId)
{
    checkPiaCoreToUse();
    EnterCriticalSection(&s_voiceMutex);
    nn::pia::chat::VoiceProtocol* protocol = Chat_GetVoiceProtocol();
    for (std::vector<unsigned long long>::iterator player = s_conferencePlayers.begin(); player != s_conferencePlayers.end(); ++player)
        if (*player == stationId)
        {
            s_conferencePlayers.erase(player);
            break;
        }
    if (protocol != NULL && protocol->IsClientInConference(stationId))
    {
        nn::pia::Result result = protocol->RemoveConferenceClient(stationId);
        AssertPiaError(result);
    }
    if (s_conferencePlayers.empty())
        stopConference();
    LeaveCriticalSection(&s_voiceMutex);
}

void NintendoVoiceChat_WiiU::RemovePlayer(NQRNetworkPlayer* player)
{
    if (player == NULL)
        return;
    EnterCriticalSection(&s_voiceMutex);
    if (player->IsRemote())
    {
        PlayerUID uid = player->GetUID();
        if ((uid.flags & 0x40000000) != 0)
        {
            for (std::vector<NQRNetworkPlayer*>::iterator voicePlayer = s_voicePlayers.begin(); voicePlayer != s_voicePlayers.end(); ++voicePlayer)
                if (*voicePlayer == player)
                {
                    s_voicePlayers.erase(voicePlayer);
                    break;
                }
            VoiceChatMessage message;
            message.type = 1;
            message.reserved = 0;
            message.stationId = uid.mii.principalId;
            s_messageQueue.push_back(message);
        }
    }
    LeaveCriticalSection(&s_voiceMutex);
}

void NintendoVoiceChat_WiiU::RemoveAllPlayers()
{
    checkPiaCoreToUse();
    while (!s_voicePlayers.empty())
        RemovePlayer(s_voicePlayers.back());
}

void NintendoVoiceChat_WiiU::FlushMessageQueue()
{
    EnterCriticalSection(&s_voiceMutex);
    while (!s_messageQueue.empty())
    {
        VoiceChatMessage message = s_messageQueue.front();
        if (message.type == 0)
            AddPlayer(message.stationId);
        else
            RemovePlayer(message.stationId);
        s_messageQueue.pop_front();
    }
    LeaveCriticalSection(&s_voiceMutex);
}

void NintendoVoiceChat_WiiU::Initialise(NQRNetworkManager_WiiU* manager)
{
    s_networkManager = manager;
    s_micDevice = g_pSndMicDevice;
}

void NintendoVoiceChat_WiiU::InitialiseAfterOnline()
{
    checkPiaCoreToUse();
    nn::pia::Result result = nn::pia::chat::Initialize();
    AssertPiaError(result);
    result = nn::pia::chat::BeginSetup();
    AssertPiaError(result);
    s_protocolManager = g_piaProtocolManager;
    if (s_protocolManager != NULL)
    {
        nn::pia::transport::UnreliableProtocol* memory = s_protocolManager->AllocProtocol(0x90);
        nn::pia::chat::VoiceProtocol* protocol = NULL;
        if (memory != NULL)
            protocol = new (memory) nn::pia::chat::VoiceProtocol();
        if (protocol != NULL)
            s_protocolId = s_protocolManager->CreateProtocolImpl(protocol, 0x7801);
        else
            Assert("protocol", "transport/ProtocolManager.h", 0x56, "NintendoVoiceChat_WiiU::InitialiseAfterOnline");
    }
    nn::pia::chat::VoiceProtocol::Setting setting;
    setting.bufferSize = 0x90;
    setting.maxPlayers = 4;
    setting.dataSize = 0x90;
    setting.codec = 10;
    setting.frameLength = 0x20;
    setting.sampleRate = 8;
    nn::pia::chat::VoiceProtocol* protocol = Chat_GetVoiceProtocol();
    result = protocol->Initialize(setting);
    AssertPiaError(result);
    result = nn::pia::chat::EndSetup();
    AssertPiaError(result);
    InitializeCriticalSection(&s_voiceMutex);
    s_foregroundState = 0;
}

void NintendoVoiceChat_WiiU::Shutdown()
{
    checkPiaCoreToUse();
    RemoveAllPlayers();
    FlushMessageQueue();
    nn::pia::chat::VoiceProtocol* protocol = Chat_GetVoiceProtocol();
    if (protocol != NULL)
    {
        protocol->Finalize();
        if (s_protocolManager != NULL && s_protocolId != 0)
        {
            s_protocolManager->DestroyProtocol(s_protocolId);
            s_protocolId = 0;
        }
    }
    nn::pia::chat::Finalize();
}

void NintendoVoiceChat_WiiU::AddPlayer(NQRNetworkPlayer* player)
{
    if (s_voiceChatEnabled && player != NULL)
    {
        EnterCriticalSection(&s_voiceMutex);
        if (player->IsRemote())
        {
            PlayerUID uid = player->GetUID();
            if ((uid.flags & 0x40000000) != 0)
            {
                s_voicePlayers.push_back(player);
                VoiceChatMessage message;
                message.type = 0;
                message.reserved = 0;
                message.stationId = uid.mii.principalId;
                s_messageQueue.push_back(message);
            }
        }
        LeaveCriticalSection(&s_voiceMutex);
    }
}

void NintendoVoiceChat_WiiU::SetVoiceChatEnabled(bool enabled)
{
    if (s_voiceChatEnabled != enabled)
    {
        s_voiceChatEnabled = enabled;
        if (!enabled)
        {
            RemoveAllPlayers();
            FlushMessageQueue();
        }
        else if (s_networkManager != NULL)
        {
            for (int index = 0; index < s_networkManager->GetPlayerCount(); ++index)
                AddPlayer(s_networkManager->GetPlayerByIndex(index));
        }
    }
}

bool NintendoVoiceChat_WiiU::IsTalking(NQRNetworkPlayer* player)
{
    if (player != NULL && player->HasVoice())
    {
        if (player->IsLocal())
        {
            nn::pia::chat::VoiceProtocol* protocol = Chat_GetVoiceProtocol();
            return protocol != NULL && protocol->GetVadActivity() != 0;
        }
        if (s_micDevice != NULL)
        {
            unsigned int sequence = s_micDevice->GetPacketSquence(player->GetUID().mii.principalId);
            bool talking = sequence != player->GetCustomDataValue();
            player->SetCustomDataValue(sequence);
            return talking;
        }
    }
    return false;
}

void NintendoVoiceChat_WiiU::Tick()
{
    if (s_foregroundState == 0)
    {
        if (s_micDevice != NULL)
            s_foregroundState = 1;
        if (s_foregroundState == 0)
        {
            SetVoiceChatEnabled(CMinecraftApp::sInstance.IsVoiceChatEnabled() != 0);
            FlushMessageQueue();
            return;
        }
    }
    else if (s_foregroundState == 1 && s_micDevice == NULL)
    {
        s_foregroundState = 0;
        return;
    }
    if (s_foregroundState == 2 && s_micDevice != NULL)
    {
        SND_PARAM parameter;
        parameter.eRateMode = eSndRateMode_8kHz;
        parameter.iFrameLengthMs = Chat_GetVoiceProtocol()->GetAudioFrameLength();
        s_micDevice->Pause(&parameter);
        Chat_GetVoiceProtocol()->ClearStreaming();
    }
    s_foregroundState = 0;
}

void NintendoVoiceChat_WiiU::ReleaseCallback()
{
    if (s_micDevice != NULL)
        s_micDevice->Pause(NULL);
}

void NintendoVoiceChat_WiiU::ReleaseForeground()
{
    s_foregroundState = 1;
}

void NintendoVoiceChat_WiiU::AcquiredForeground()
{
    s_foregroundState = 2;
}
