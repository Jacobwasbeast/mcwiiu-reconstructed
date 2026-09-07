#ifndef SOURCE_PLATFORM_WIIU_FFLIFSCLIENT_HPP
#define SOURCE_PLATFORM_WIIU_FFLIFSCLIENT_HPP

#include <stdint.h>
#include <coreinit/filesystem.h>

class FFLiFsClient
{
private:
    FSClient m_fsClient;
    uint8_t m_valid;

public:
    FFLiFsClient();
    ~FFLiFsClient();
    void SetStateChangeNotification(FSStateChangeParams *params);
    int32_t Init(const FSStateChangeParams *params, unsigned int error);
    uint8_t IsValid() const;
    FSVolumeState GetVolumeState();
    void Shutdown(unsigned int error);
    uint32_t ShutdownIfValid(unsigned int error);
};

#endif
