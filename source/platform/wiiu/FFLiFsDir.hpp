#ifndef SOURCE_PLATFORM_WIIU_FFLIFSDIR_HPP
#define SOURCE_PLATFORM_WIIU_FFLIFSDIR_HPP

class FFLiFsCommand;
struct FSDirEntry;

class FFLiFsDir
{
public:
    FFLiFsDir(FFLiFsCommand *command);
    ~FFLiFsDir();
    int Open(const char *path, unsigned int error);
    int Close(unsigned int error);
    int Read(FSDirEntry *entry, unsigned int error);
    int Rewind(unsigned int error);
    bool IsOpened() const;
    static int MakeDir(FFLiFsCommand *command, const char *path, unsigned int error);
};

#endif
