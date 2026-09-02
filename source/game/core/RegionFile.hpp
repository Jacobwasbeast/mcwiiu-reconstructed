#ifndef GAME_CORE_REGIONFILE_HPP
#define GAME_CORE_REGIONFILE_HPP

#include <vector>

#include "game/core/ByteArrayOutputStream.hpp"

class ConsoleSaveFile;
class DataInputStream;
class DataOutputStream;
class File;
class FileEntry;

class RegionFile
{
public:
    class ChunkBuffer : public ByteArrayOutputStream
    {
    public:
        ChunkBuffer(RegionFile* regionFile, int x, int z, bool bNewSaveFormat)
            : ByteArrayOutputStream(65536)
        {
            mRegionFile = regionFile;
            mX = x;
            mZ = z;
            mNewSaveFormat = bNewSaveFormat;
        }

        virtual ~ChunkBuffer();

        virtual void close();

    private:
        RegionFile* mRegionFile;
        int mX;
        int mZ;
        bool mNewSaveFormat;
    };

    RegionFile(ConsoleSaveFile* saveFile, File* fileName);
    ~RegionFile();

    DataInputStream* getChunkDataInputStream(int x, int z, bool* pbNewSaveFormat);
    DataOutputStream* getChunkDataOutputStream(int x, int z, bool bNewSaveFormat);

    void write(int x, int z, unsigned char* data, int length, bool bNewSaveFormat);

    bool hasChunk(int x, int z);
    void writeAllOffsets();
    void close();

    int fjCountSavedChunks() const;
    long long lastModified();
    int getSizeDelta();

    static void CreateNewThreadStorage();
    static void ReleaseThreadStorage();

private:
    int getOffset(int x, int z);
    bool outOfBounds(int x, int z);

    void setOffset(int x, int z, int offset);
    void setTimestamp(int x, int z, int timestamp);

    void insertInitialSectors();
    void zero(int sectorNumber, int length);

    void write(int sectorNumber, unsigned char* data, int length, unsigned int compressedLength, bool bLzxRleCompressed, bool bNewSaveFormat, bool bOwnsData);

    unsigned char* GetThreadCompressedDataStorage();

    FileEntry* mFile;
    File* mFileName;
    ConsoleSaveFile* mSaveFile;
    unsigned int* mOffsets;
    int* mChunkTimestamps;
    std::vector<bool>* mSectorFree;
    int mSizeDelta;
    long long mLastModified;
    bool mNewFile;
};

#endif
