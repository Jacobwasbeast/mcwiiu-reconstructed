#ifndef SOURCE_VENDOR_MILES_IO_HPP
#define SOURCE_VENDOR_MILES_IO_HPP

struct MilesAsyncRead;
struct SoundBank;
class EventSystem;

struct _MILESBANKSOUNDINFO
{
    unsigned char m_reserved0[0xc];
    int m_dataSize;
    int m_param;
    int m_unbuffered;
    unsigned char m_reserved1[0x28 - 0x18];
    int m_extraParam;
};

typedef int (*MilesContainerGetSoundCallback)(const char* name, char* path, _MILESBANKSOUNDINFO* info);
typedef void* (*MilesContainerGetPresetCallback)(const char* name);
typedef void (*MilesAuditionAsyncQueuedCallback)(unsigned long long eventId, int auditionId, const char* name);
typedef void (*MilesAuditionAsyncLoadCallback)(int auditionId, int size);
typedef void (*MilesAuditionAsyncCancelCallback)(int auditionId);
typedef void (*MilesAuditionSetPersistCallback)(unsigned long long eventId, const char* name,
                                                const char* preset);
typedef void (*MilesAuditionRampStateCallback)(const char* name, unsigned long long eventId, int target);

typedef unsigned int (*AIL_FILE_OPEN_CALLBACK)(const char* pFileName, unsigned int* puFileHandle);
typedef void (*AIL_FILE_CLOSE_CALLBACK)(unsigned int uFileHandle);
typedef int (*AIL_FILE_SEEK_CALLBACK)(unsigned int uFileHandle, int iOffset, unsigned int uType);
typedef int (*AIL_FILE_READ_CALLBACK)(unsigned int uFileHandle, void* pBuffer, unsigned int uBytes);

extern "C" AIL_FILE_OPEN_CALLBACK AIL_file_open;
extern "C" AIL_FILE_CLOSE_CALLBACK AIL_file_close;
extern "C" AIL_FILE_SEEK_CALLBACK AIL_file_seek;
extern "C" AIL_FILE_READ_CALLBACK AIL_file_read;

unsigned int DefaultFileOpen(const char* pFileName, unsigned int* puFileHandle);
void DefaultFileClose(unsigned int uFileHandle);
int DefaultFileSeek(unsigned int uFileHandle, int iOffset, unsigned int uType);
int DefaultFileRead(unsigned int uFileHandle, void* pBuffer, unsigned int uBytes);

void CloseAsync(MilesAsyncRead* pRead);

extern MilesContainerGetSoundCallback g_milesContainerGetSoundCallback;
extern MilesContainerGetPresetCallback g_milesContainerGetPresetCallback;
extern MilesAuditionAsyncQueuedCallback g_milesAuditionAsyncQueuedCallback;
extern MilesAuditionAsyncLoadCallback g_milesAuditionAsyncLoadCallback;
extern MilesAuditionAsyncCancelCallback g_milesAuditionAsyncCancelCallback;
extern MilesAuditionSetPersistCallback g_milesAuditionSetPersistCallback;
extern MilesAuditionRampStateCallback g_milesAuditionRampStateCallback;
extern EventSystem* g_eventSystem;

SoundBank* GetContainer(const char* name, const char** assetName);
int Default_GetSound(const char* name, char* path, _MILESBANKSOUNDINFO* info);
void* Default_GetPreset(const char* name);
void EvalFunc(void* instance, const char* name, int index, void* value, int flags);

extern "C" int Container_GetSound(const char* name, char* path, _MILESBANKSOUNDINFO* info);
extern "C" void* Container_GetPreset(const char* name);
extern "C" void Audition_AsyncQueued(unsigned long long eventId, int auditionId, const char* name);
extern "C" void Audition_AsyncLoad(int auditionId, int size);
extern "C" void Audition_AsyncCancel(int auditionId);
extern "C" void Audition_SetPersist(unsigned long long eventId, const char* name, const char* preset);
extern "C" void Audition_RampState(const char* name, unsigned long long eventId, int target);

#endif
