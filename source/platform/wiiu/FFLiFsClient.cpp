#include "platform/wiiu/FFLiFsClient.hpp"

extern "C" int FSAddClient(FSClient *client, unsigned int error);
extern "C" void FSDelClient(FSClient *client);
extern "C" void FSSetStateChangeNotification(FSClient *client, FSStateChangeParams *params);
extern "C" FSVolumeState FSGetVolumeState(FSClient *client);
extern FSStateChangeParams g_ffliFsClientStateChangeParams;

FFLiFsClient::FFLiFsClient()
{
    m_valid = 0;
}

FFLiFsClient::~FFLiFsClient()
{
}

void FFLiFsClient::SetStateChangeNotification(FSStateChangeParams *params)
{
    FSSetStateChangeNotification(&m_fsClient, params);
}

int32_t FFLiFsClient::Init(const FSStateChangeParams *params, unsigned int error)
{
    int32_t result = FSAddClient(&m_fsClient, error);
    if (result == 0)
    {
        if (params == 0)
        {
            params = &g_ffliFsClientStateChangeParams;
        }
        SetStateChangeNotification(const_cast<FSStateChangeParams *>(params));
        m_valid = 1;
    }
    return result;
}

uint8_t FFLiFsClient::IsValid() const
{
    return m_valid;
}

FSVolumeState FFLiFsClient::GetVolumeState()
{
    return FSGetVolumeState(&m_fsClient);
}

void FFLiFsClient::Shutdown(unsigned int error)
{
    FSDelClient(&m_fsClient);
    m_valid = 0;
}

uint32_t FFLiFsClient::ShutdownIfValid(unsigned int error)
{
    uint32_t result = 0;
    if (IsValid() != 0)
    {
        Shutdown(error);
    }
    return result;
}
