#ifndef SOURCE_PLATFORM_WIIU_WIIUMILESFILESYSTEM_HPP
#define SOURCE_PLATFORM_WIIU_WIIUMILESFILESYSTEM_HPP

struct CFileHandle;

void InvalidePositionsForOtherHandles(CFileHandle* handle);
int file_open_callback(const char* filename, unsigned int* handleOut);
void file_close_callback(unsigned int handle);
int file_seek_callback(unsigned int handle, int offset, int origin);
int file_read_callback(unsigned int handle, void* buffer, int bytes);

#endif
