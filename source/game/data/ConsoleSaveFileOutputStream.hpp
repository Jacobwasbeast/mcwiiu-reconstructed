#ifndef GAME_DATA_CONSOLESAVEFILEOUTPUTSTREAM_HPP
#define GAME_DATA_CONSOLESAVEFILEOUTPUTSTREAM_HPP

#include "game/core/FileEntry.hpp"
#include "game/core/OutputStream.hpp"
#include "game/data/ConsoleSaveFile.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class ConsoleSaveFileOutputStream : public OutputStream
{
public:
    ConsoleSaveFileOutputStream(ConsoleSaveFile* saveFile, const ConsoleSavePath& path);
    ConsoleSaveFileOutputStream(ConsoleSaveFile* saveFile, FileEntry* fileEntry);
    virtual ~ConsoleSaveFileOutputStream();

    virtual void write(unsigned int value);
    virtual void write(arrayWithLength<unsigned char> data);
    virtual void write(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length);
    virtual void close();
    virtual void flush();

private:
    ConsoleSaveFile* mSaveFile;
    FileEntry* mFileEntry;
};

#endif
