#include "game/core/RegionFile.hpp"

#include <string.h>

#include <algorithm>

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/Compression.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/File.hpp"
#include "game/core/FileEntry.hpp"
#include "game/core/System.hpp"
#include "game/data/ConsoleSaveFile.hpp"
#include "game/world/biome/arrayWithLength.hpp"

void MemSect(int section);
void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();
unsigned int TlsAlloc();
void* TlsGetValue(unsigned int index);
void TlsSetValue(unsigned int index, void* value);

static const int SECTOR_BYTES = 4096;
static const int SECTOR_INTS = 1024;
static const unsigned int CHUNK_HEADER_LENGTH_MASK = 0x3FFFFFFF;
static const unsigned int CHUNK_HEADER_LZXRLE_FLAG = 0x80000000;
static const unsigned int CHUNK_HEADER_NEW_FORMAT_FLAG = 0x40000000;
static const unsigned int THREAD_COMPRESSED_DATA_SIZE = 65536;

static arrayWithLength<unsigned char> gEmptySector(SECTOR_BYTES, true);
static unsigned int gCompressedDataStorageIndex = TlsAlloc();

RegionFile::RegionFile(ConsoleSaveFile* saveFile, File* fileName)
{
    mSaveFile = saveFile;
    mLastModified = 0;

    mOffsets = new unsigned int[SECTOR_INTS];
    memset(mOffsets, 0, SECTOR_INTS * sizeof(unsigned int));

    mChunkTimestamps = new int[SECTOR_INTS];
    memset(mChunkTimestamps, 0, SECTOR_INTS * sizeof(int));

    mFileName = fileName;
    mSizeDelta = 0;

    mFile = mSaveFile->createFile(fileName->getPath());

    mSaveFile->setFilePointer(mFile, 0, 0, 2);

    if (mFile->m_fileSize < (unsigned int)SECTOR_BYTES)
    {
        mSizeDelta += SECTOR_BYTES * 2;
        mNewFile = true;
    }
    else
    {
        mNewFile = false;
    }

    if ((mFile->m_fileSize & (SECTOR_BYTES - 1)) != 0)
    {
        unsigned int padding = SECTOR_BYTES - (mFile->m_fileSize & (SECTOR_BYTES - 1));
        unsigned int bytesWritten = 0;
        unsigned char* emptyTail = new unsigned char[padding];

        memset(emptyTail, 0, padding);
        mSaveFile->writeFile(mFile, emptyTail, padding, &bytesWritten);

        delete[] emptyTail;
    }

    int numSectors;

    if (mNewFile)
    {
        numSectors = 2;
    }
    else
    {
        numSectors = (int)mFile->m_fileSize / SECTOR_BYTES;
    }

    mSectorFree = new std::vector<bool>();
    mSectorFree->reserve(numSectors);

    for (int sector = 0; sector < numSectors; ++sector)
    {
        mSectorFree->push_back(true);
    }

    (*mSectorFree)[0] = false;
    (*mSectorFree)[1] = false;

    mSaveFile->setFilePointer(mFile, 0, 0, 0);

    for (int index = 0; index < SECTOR_INTS; ++index)
    {
        unsigned int offset = 0;
        unsigned int bytesRead = 0;

        if (!mNewFile)
        {
            mSaveFile->readFile(mFile, &offset, 4, &bytesRead);

            if (saveFile->isSaveEndianDifferent())
            {
                System::ReverseULONG(&offset);
            }
        }

        mOffsets[index] = offset;

        if (offset != 0)
        {
            unsigned int sectorNumber = offset >> 8;
            unsigned int numChunkSectors = offset & 0xFF;

            if (sectorNumber + numChunkSectors <= mSectorFree->size())
            {
                for (unsigned int sector = 0; sector < numChunkSectors; ++sector)
                {
                    (*mSectorFree)[sectorNumber + sector] = false;
                }
            }
        }
    }

    for (int index = 0; index < SECTOR_INTS; ++index)
    {
        int timestamp = 0;
        unsigned int bytesRead = 0;

        if (!mNewFile)
        {
            mSaveFile->readFile(mFile, &timestamp, 4, &bytesRead);

            if (saveFile->isSaveEndianDifferent())
            {
                System::ReverseINT(&timestamp);
            }
        }

        mChunkTimestamps[index] = timestamp;
    }
}

RegionFile::~RegionFile()
{
    delete[] mOffsets;
    delete[] mChunkTimestamps;
    delete mSectorFree;

    mSaveFile->closeHandle(mFile);
}

int RegionFile::getOffset(int x, int z)
{
    return mOffsets[x + z * 32];
}

bool RegionFile::outOfBounds(int x, int z)
{
    return x < 0 || x >= 32 || z < 0 || z >= 32;
}

DataInputStream* RegionFile::getChunkDataInputStream(int x, int z, bool* pbNewSaveFormat)
{
    if (outOfBounds(x, z))
    {
        return NULL;
    }

    int offset = getOffset(x, z);

    if (offset == 0)
    {
        return NULL;
    }

    int sectorNumber = offset >> 8;
    int numSectors = offset & 0xFF;

    if ((unsigned int)(sectorNumber + numSectors) > mSectorFree->size())
    {
        return NULL;
    }

    mSaveFile->LockSaveAccess();
    mSaveFile->setFilePointer(mFile, sectorNumber * SECTOR_BYTES, 0, 0);

    unsigned int compressedLength = 0;
    unsigned int bytesRead = 0;

    mSaveFile->readFile(mFile, &compressedLength, 4, &bytesRead);

    if (mSaveFile->isSaveEndianDifferent())
    {
        System::ReverseULONG(&compressedLength);
    }

    bool bLzxRleCompressed = (compressedLength & CHUNK_HEADER_LZXRLE_FLAG) != 0;

    if (pbNewSaveFormat != NULL)
    {
        *pbNewSaveFormat = (compressedLength & CHUNK_HEADER_NEW_FORMAT_FLAG) != 0;
    }

    compressedLength &= CHUNK_HEADER_LENGTH_MASK;

    unsigned int length = 0;

    mSaveFile->readFile(mFile, &length, 4, &bytesRead);

    if (mSaveFile->isSaveEndianDifferent())
    {
        System::ReverseULONG(&length);
    }

    if (compressedLength > (unsigned int)(numSectors * SECTOR_BYTES))
    {
        mSaveFile->ReleaseSaveAccess();
        return NULL;
    }

    MemSect(50);

    unsigned char* compressed = new unsigned char[compressedLength];
    unsigned char* chunkData = new unsigned char[length];

    MemSect(0);

    unsigned int chunkLength = length;

    mSaveFile->readFile(mFile, compressed, compressedLength, &bytesRead);
    mSaveFile->ReleaseSaveAccess();

    Compression::getCompression()->SetDecompressionType(mSaveFile->getSavePlatform());

    if (bLzxRleCompressed)
    {
        Compression::getCompression()->DecompressLZXRLE(chunkData, &chunkLength, compressed, compressedLength);
    }
    else
    {
        Compression::getCompression()->Decompress(chunkData, &chunkLength, compressed, compressedLength);
    }

    Compression::getCompression()->SetDecompressionType(eSavePlatform_WIIU);

    delete[] compressed;

    arrayWithLength<unsigned char> buffer;
    buffer.data = chunkData;
    buffer.length = chunkLength;

    return new DataInputStream(new ByteArrayInputStream(buffer));
}

DataOutputStream* RegionFile::getChunkDataOutputStream(int x, int z, bool bNewSaveFormat)
{
    return new DataOutputStream(new ChunkBuffer(this, x, z, bNewSaveFormat));
}

unsigned char* RegionFile::GetThreadCompressedDataStorage()
{
    return (unsigned char*)TlsGetValue(gCompressedDataStorageIndex);
}

void RegionFile::write(int sectorNumber, unsigned char* data, int length, unsigned int compressedLength, bool bLzxRleCompressed, bool bNewSaveFormat, bool bOwnsData)
{
    unsigned int bytesWritten = 0;

    mSaveFile->setFilePointer(mFile, sectorNumber * SECTOR_BYTES, 0, 0);

    if (bLzxRleCompressed)
    {
        compressedLength |= CHUNK_HEADER_LZXRLE_FLAG;
    }
    else
    {
        compressedLength &= ~CHUNK_HEADER_LZXRLE_FLAG;
    }

    if (bNewSaveFormat)
    {
        compressedLength |= CHUNK_HEADER_NEW_FORMAT_FLAG;
    }

    mSaveFile->writeFile(mFile, &compressedLength, 4, &bytesWritten);

    compressedLength &= CHUNK_HEADER_LENGTH_MASK;

    mSaveFile->writeFile(mFile, &length, 4, &bytesWritten);
    mSaveFile->writeFile(mFile, data, compressedLength, &bytesWritten);

    if (bOwnsData)
    {
        delete[] data;
    }
}

void RegionFile::zero(int sectorNumber, int length)
{
    unsigned int bytesWritten = 0;

    mSaveFile->setFilePointer(mFile, sectorNumber * SECTOR_BYTES, 0, 0);
    mSaveFile->zeroFile(mFile, length, &bytesWritten);
}

void RegionFile::insertInitialSectors()
{
    unsigned char emptySector[SECTOR_BYTES];
    unsigned int bytesWritten = 0;

    mSaveFile->setFilePointer(mFile, 0, 0, 0);

    memset(emptySector, 0, sizeof(emptySector));

    mSaveFile->writeFile(mFile, emptySector, SECTOR_BYTES, &bytesWritten);
    mSaveFile->writeFile(mFile, emptySector, SECTOR_BYTES, &bytesWritten);

    mNewFile = false;
}

void RegionFile::setOffset(int x, int z, int offset)
{
    if (mNewFile)
    {
        insertInitialSectors();
    }

    unsigned int bytesWritten = 0;
    int index = (x + z * 32) * 4;

    mOffsets[x + z * 32] = offset;

    mSaveFile->setFilePointer(mFile, index, 0, 0);
    mSaveFile->writeFile(mFile, &offset, 4, &bytesWritten);
}

void RegionFile::setTimestamp(int x, int z, int timestamp)
{
    if (mNewFile)
    {
        insertInitialSectors();
    }

    unsigned int bytesWritten = 0;
    int index = (x + z * 32) * 4;

    mChunkTimestamps[x + z * 32] = timestamp;

    mSaveFile->setFilePointer(mFile, index + SECTOR_BYTES, 0, 0);
    mSaveFile->writeFile(mFile, &timestamp, 4, &bytesWritten);
}

void RegionFile::write(int x, int z, unsigned char* data, int length, bool bNewSaveFormat)
{
    unsigned int compressedLength = length + 2048;
    bool bOwnsBuffer = false;
    unsigned char* compressed = NULL;

    if (compressedLength < THREAD_COMPRESSED_DATA_SIZE)
    {
        compressed = GetThreadCompressedDataStorage();
    }

    if (compressed == NULL)
    {
        bOwnsBuffer = true;
        compressed = new unsigned char[compressedLength];
    }

    bool bLzxRleCompressed = true;

    if (Compression::getCompression()->CompressLZXRLE(compressed, &compressedLength, data, length, NULL) != 0)
    {
        Compression::getCompression()->Compress(compressed, &compressedLength, data, length);
        bLzxRleCompressed = false;
    }

    int sectorsNeeded = ((compressedLength + 8) >> 12) + 1;

    if (sectorsNeeded >= 256)
    {
        return;
    }

    mSaveFile->LockSaveAccess();

    int offset = getOffset(x, z);
    int sectorNumber = offset >> 8;
    int sectorsAllocated = offset & 0xFF;

    if (sectorNumber == 0 || sectorsAllocated != sectorsNeeded)
    {
        for (int sector = 0; sector < sectorsAllocated; ++sector)
        {
            (*mSectorFree)[sectorNumber + sector] = true;
        }

        zero(sectorNumber, sectorsAllocated * SECTOR_BYTES);

        PIXBeginNamedEvent(0.0f, "Scanning for free space\n");

        std::vector<bool>::iterator firstFree = std::find(mSectorFree->begin(), mSectorFree->end(), true);
        int runStart = firstFree - mSectorFree->begin();
        int runLength = 0;

        if (runStart != -1)
        {
            for (int sector = runStart; sector < mSectorFree->size(); ++sector)
            {
                if (runLength != 0)
                {
                    if ((*mSectorFree)[sector])
                    {
                        ++runLength;
                    }
                    else
                    {
                        runLength = 0;
                    }
                }
                else if ((*mSectorFree)[sector])
                {
                    runStart = sector;
                    runLength = 1;
                }

                if (runLength >= sectorsNeeded)
                {
                    break;
                }
            }
        }

        PIXEndNamedEvent();

        if (runLength < sectorsNeeded)
        {
            PIXBeginNamedEvent(0.0f, "Expanding storage for %d sectors\n", sectorsNeeded);

            mSaveFile->setFilePointer(mFile, 0, 0, 2);

            int appendedSector = mSectorFree->size();
            unsigned int bytesWritten = 0;

            for (int sector = 0; sector < sectorsNeeded; ++sector)
            {
                mSaveFile->writeFile(mFile, gEmptySector.data, SECTOR_BYTES, &bytesWritten);
                mSectorFree->push_back(false);
            }

            mSizeDelta += sectorsNeeded * SECTOR_BYTES;

            write(appendedSector, compressed, length, compressedLength, bLzxRleCompressed, bNewSaveFormat, bOwnsBuffer);
            setOffset(x, z, appendedSector << 8 | sectorsNeeded);

            PIXEndNamedEvent();

            setTimestamp(x, z, (int)System::processTimeInSeconds());
            mSaveFile->ReleaseSaveAccess();
            return;
        }

        sectorNumber = runStart;
        setOffset(x, z, sectorNumber << 8 | sectorsNeeded);

        for (int sector = 0; sector < sectorsNeeded; ++sector)
        {
            (*mSectorFree)[sectorNumber + sector] = false;
        }
    }

    write(sectorNumber, compressed, length, compressedLength, bLzxRleCompressed, bNewSaveFormat, bOwnsBuffer);

    setTimestamp(x, z, (int)System::processTimeInSeconds());
    mSaveFile->ReleaseSaveAccess();
}

int RegionFile::fjCountSavedChunks() const
{
    int count = 0;

    for (int index = 0; index < SECTOR_INTS; ++index)
    {
        if (mOffsets[index] != 0)
        {
            ++count;
        }
    }

    return count;
}

void RegionFile::writeAllOffsets()
{
    if (mNewFile)
    {
        return;
    }

    mSaveFile->LockSaveAccess();

    unsigned int bytesWritten = 0;

    mSaveFile->setFilePointer(mFile, 0, 0, 0);
    mSaveFile->writeFile(mFile, mOffsets, SECTOR_BYTES, &bytesWritten);

    bytesWritten = 0;

    mSaveFile->setFilePointer(mFile, SECTOR_BYTES, 0, 0);
    mSaveFile->writeFile(mFile, mChunkTimestamps, SECTOR_BYTES, &bytesWritten);

    mSaveFile->ReleaseSaveAccess();
}

void RegionFile::close()
{
    mSaveFile->closeHandle(mFile);
}

void RegionFile::CreateNewThreadStorage()
{
    TlsSetValue(gCompressedDataStorageIndex, new unsigned char[THREAD_COMPRESSED_DATA_SIZE]);
}

RegionFile::ChunkBuffer::~ChunkBuffer()
{
}

void RegionFile::ChunkBuffer::close()
{
    mRegionFile->write(mX, mZ, mBuffer.data, mCount, mNewSaveFormat);
}
