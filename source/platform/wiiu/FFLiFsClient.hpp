#ifndef SOURCE_PLATFORM_WIIU_FFLIFSCLIENT_HPP
#define SOURCE_PLATFORM_WIIU_FFLIFSCLIENT_HPP

#include <stdint.h>

struct FSClient
{
    uint8_t storage[5888];
};

enum FSVolumeState
{
    FS_VOLUME_STATE_UNKNOWN = 0
};

struct FSStateChangeParams
{
    void (*callback)(FSClient *, FSVolumeState, void *);
    void *context;
    uint32_t queue;
    uint32_t reserved;
};

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
