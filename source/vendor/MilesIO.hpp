#ifndef SOURCE_VENDOR_MILES_IO_HPP
#define SOURCE_VENDOR_MILES_IO_HPP

struct MilesAsyncRead;

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

#endif
