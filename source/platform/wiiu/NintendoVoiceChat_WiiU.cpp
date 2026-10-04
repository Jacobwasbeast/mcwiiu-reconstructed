#include "platform/wiiu/NintendoVoiceChat_WiiU.hpp"

#include <deque>
#include <new>
#include <vector>

#include <proc_ui/procui.h>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "nn/pia.h"
#include "nn/pia/Transport.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "platform/wiiu/NQRNetworkManager_WiiU.hpp"
#include "platform/wiiu/NQRNetworkPlayer.hpp"
#include "platform/wiiu/SndMicDevice.hpp"
#include "vendor/__N_22_chat_VoiceProtocol_cpp_bf22a0fe.hpp"

void checkPiaCoreToUse();

void AssertPiaError(nn::pia::Result& result);

enum VoiceChatMessageType
{
    eVoiceChatMessage_AddPlayer = 0,
    eVoiceChatMessage_RemovePlayer = 1
};

enum
{
    VOICE_STREAM_BUFFER_SIZE = 0x1000
};

struct VoiceChatMessage
{
    int type;
    unsigned long long stationId;
};

static unsigned int s_protocolId = 0;
static unsigned int s_foregroundState = 0;
static bool s_initialised = false;
static bool s_voiceChatEnabled = true;
static std::vector<NQRNetworkPlayer*> s_voicePlayers;
static std::vector<unsigned long long> s_conferencePlayers;
static unsigned int s_micGainDefault;
static unsigned int s_micGainMin;
static unsigned int s_micGainMax;
static unsigned int s_micBufferSize;
static unsigned int s_micReserved;
static SndMicDevice* s_micDevice;
static std::deque<VoiceChatMessage> s_messageQueue;
static OSFastMutex s_voiceMutex;
static NQRNetworkManager_WiiU* s_networkManager;
static SndMicDevice s_micDeviceInstance;

nn::pia::chat::VoiceProtocol* Chat_GetVoiceProtocol()
{
    if (nn::pia::transport::g_piaTransport != NULL && s_protocolId != 0)
        return (nn::pia::chat::VoiceProtocol*)nn::pia::transport::g_piaTransport->m_protocolManager.SearchProtocol(nn::pia::transport::ProtocolId(s_protocolId), 0x78);
    return NULL;
}

extern "C" void ReleaseCallback()
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
        parameter.iFrameLengthMs = Chat_GetVoiceProtocol()->GetAudioFrameLength();
        s_micDeviceInstance.InitializeMicSndCommon(&parameter, Chat_GetVoiceProtocol());
        s_micDeviceInstance.GetStateMic(&state);
        s_micReserved = 0;
        s_micGainMax = state.uiGainMax;
        s_micBufferSize = VOICE_STREAM_BUFFER_SIZE;
        s_micGainDefault = state.uiGainDefault;
        s_micDevice = &s_micDeviceInstance;
        s_micGainMin = state.uiGainMin;
        s_initialised = true;
    }
}

void NintendoVoiceChat_WiiU::AddPlayer(unsigned long long stationId)
{
    checkPiaCoreToUse();
    EnterCriticalSection(&s_voiceMutex);
    nn::pia::session::Session* session = nn::pia::session::g_piaSession;
    nn::pia::chat::VoiceProtocol* protocol = Chat_GetVoiceProtocol();
    if (session != NULL && protocol != NULL && session->IsValidStationId(stationId) && !protocol->IsClientInConference(stationId))
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
        nn::pia::transport::StationManager* stationManager = nn::pia::transport::g_piaStationManager;
        nn::pia::transport::Station** first = stationManager->m_stations.data;
        for (nn::pia::transport::Station** entry = first; entry != first + stationManager->m_stations.size; ++entry)
        {
            nn::pia::transport::Station* station = *entry;
            Chat_GetVoiceProtocol()->RemoveConferenceClient(((unsigned long long)station->m_field18 << 32) | station->m_field1c);
        }
        s_micDeviceInstance.StopMic();
        s_micDeviceInstance.StopSnd();
        s_micDeviceInstance.FinalizeMic();
        s_micDeviceInstance.FinalizeMicSndCommon();
        s_initialised = false;
        s_micDevice = NULL;
    }
}

void NintendoVoiceChat_WiiU::RemovePlayer(unsigned long long stationId)
{
    checkPiaCoreToUse();
    EnterCriticalSection(&s_voiceMutex);
    nn::pia::session::Session* session = nn::pia::session::g_piaSession;
    nn::pia::chat::VoiceProtocol* protocol = Chat_GetVoiceProtocol();
    if (session != NULL && protocol != NULL)
    {
        for (unsigned int index = 0; index < s_conferencePlayers.size(); ++index)
            if (s_conferencePlayers[index] == stationId)
            {
                s_conferencePlayers[index] = s_conferencePlayers.back();
                s_conferencePlayers.pop_back();
                break;
            }
        if (session->IsValidStationId(stationId) && protocol->IsClientInConference(stationId))
        {
            nn::pia::Result result = protocol->RemoveConferenceClient(stationId);
            AssertPiaError(result);
        }
        if (s_conferencePlayers.empty())
            stopConference();
    }
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
            message.type = eVoiceChatMessage_RemovePlayer;
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
        if (message.type == eVoiceChatMessage_AddPlayer)
            AddPlayer(message.stationId);
        else if (message.type == eVoiceChatMessage_RemovePlayer)
            RemovePlayer(message.stationId);
        s_messageQueue.pop_front();
    }
    LeaveCriticalSection(&s_voiceMutex);
}

void NintendoVoiceChat_WiiU::Initialise(NQRNetworkManager_WiiU* manager)
{
    s_networkManager = manager;
}

void NintendoVoiceChat_WiiU::InitialiseAfterOnline()
{
    checkPiaCoreToUse();
    nn::pia::Result result = nn::pia::chat::Initialize();
    AssertPiaError(result);
    result = nn::pia::chat::BeginSetup();
    AssertPiaError(result);
    nn::pia::transport::Transport* transport = nn::pia::transport::g_piaTransport;
    if (!transport->m_protocolManager.m_started)
    {
        nn::pia::transport::UnreliableProtocol* memory = transport->m_protocolManager.AllocProtocol(0x90);
        nn::pia::chat::VoiceProtocol* protocol = NULL;
        if (memory != NULL)
            protocol = new (memory) nn::pia::chat::VoiceProtocol();
        if (protocol != (nn::pia::chat::VoiceProtocol*)memory)
            nn::pia::Assert("pProtocol == memptr", "transport_ProtocolManager.h", 0x56, NULL);
        s_protocolId = transport->m_protocolManager.CreateProtocolImpl(protocol, nn::pia::transport::ProtocolId(0x7801));
    }
    else
        s_protocolId = 0;
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
        if (nn::pia::transport::g_piaTransport != NULL)
        {
            nn::pia::transport::g_piaTransport->m_protocolManager.DestroyProtocol(nn::pia::transport::ProtocolId(s_protocolId));
            s_protocolId = 0;
        }
    }
    nn::pia::chat::Finalize();
}

void NintendoVoiceChat_WiiU::AddPlayer(NQRNetworkPlayer* player)
{
    if (s_voiceChatEnabled)
    {
        EnterCriticalSection(&s_voiceMutex);
        unsigned long long stationId = player->GetStationId();
        if (player->IsRemote())
        {
            PlayerUID uid = player->GetUID();
            if ((uid.flags & 0x40000000) != 0)
            {
                s_voicePlayers.push_back(player);
                VoiceChatMessage message;
                message.type = eVoiceChatMessage_AddPlayer;
                message.stationId = stationId;
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
    if (player->HasVoice())
    {
        if (player->IsLocal())
            return Chat_GetVoiceProtocol()->GetVadActivity();
        if (s_micDevice != NULL)
        {
            unsigned int previous = player->GetCustomDataValue();
            unsigned int sequence = s_micDevice->GetPacketSquence(player->GetStationId());
            player->SetCustomDataValue(sequence);
            if (sequence != previous)
                return true;
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
            SetVoiceChatEnabled(gApp.IsVoiceChatEnabled() != 0);
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

void AssertPiaError(nn::pia::Result& result)
{
}
