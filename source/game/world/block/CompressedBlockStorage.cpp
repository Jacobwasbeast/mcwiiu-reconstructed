#include "game/world/block/CompressedBlockStorage.hpp"

#include <string.h>

#include "game/core/CriticalSection.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/System.hpp"
#include "game/core/XLockFreeStack.hpp"

struct _MEMORYSTATUS
{
    unsigned long dwLength;
    unsigned long dwMemoryLoad;
    unsigned long dwTotalPhys;
    unsigned long dwAvailPhys;
    unsigned long dwTotalPageFile;
    unsigned long dwAvailPageFile;
    unsigned long dwTotalVirtual;
    unsigned long dwAvailVirtual;
};

void* XPhysicalAlloc(unsigned long size, unsigned long upperAddress, unsigned long alignment, unsigned int protect);
void XPhysicalFree(void* address);
void* XMemSet(void* destination, int value, unsigned int count);
void* XMemCpy(void* destination, const void* source, unsigned int count);
void GlobalMemoryStatus(_MEMORYSTATUS* status);
void __debugbreak();

static int sPendingDeleteList;
static XLockFreeStack<unsigned char> sPendingDeletes[3];
static OSFastMutex sCompressLock;
static int sAllocatedBytes;

CompressedBlockStorage::CompressedBlockStorage()
{
    mData = NULL;
    mSize = 0;
}

CompressedBlockStorage::~CompressedBlockStorage()
{
    if (mData != NULL)
    {
        int size = mSize;

        XPhysicalFree(mData);

        sAllocatedBytes -= size;
    }
}

void CompressedBlockStorage::staticCtor()
{
    InitializeCriticalSectionAndSpinCount(&sCompressLock, 5120);
}

void CompressedBlockStorage::tick()
{
    int list = (sPendingDeleteList + 1) % 3;

    unsigned char* data = sPendingDeletes[list].Pop();

    while (data != NULL)
    {
        XPhysicalFree(data);

        data = sPendingDeletes[list].Pop();
    }

    sPendingDeleteList = (sPendingDeleteList + 1) % 3;
}

void CompressedBlockStorage::queueForDelete(unsigned char* data, int size)
{
    if (data != NULL)
    {
        sPendingDeletes[sPendingDeleteList].Push(data);

        sAllocatedBytes -= size;
    }
}

void CompressedBlockStorage::reverseIndices(unsigned char* data)
{
    unsigned short* headers = (unsigned short*)data;

    for (int i = 0; i < 512; i++)
    {
        System::ReverseUSHORT(&headers[i]);
    }
}

void CompressedBlockStorage::getDataSection(unsigned char* blocks, int x, int y, int z)
{
    const unsigned short* headers = (const unsigned short*)mData;
    const unsigned char* sections = mData + 1024;

    unsigned short header = headers[(x << 7) | (z << 5) | y];
    unsigned int format = header & 3;

    if (format != 3)
    {
        int shift = 3 - format;
        unsigned int bitsPerBlock = 1 << format;
        unsigned int paletteSize = 1 << bitsPerBlock;
        unsigned int slotMask = 7 >> format;
        unsigned int indexMask = 0x3E >> shift;
        unsigned int valueMask = paletteSize - 1;
        const unsigned char* palette = sections + ((header >> 1) & 0x7FFE);
        const unsigned char* indices = palette + paletteSize;

        for (int i = 0; i < 64; i++)
        {
            blocks[i] = palette[(indices[(i >> shift) & indexMask] >> ((i & slotMask) * bitsPerBlock)) & valueMask];
        }
    }
    else if ((header & 4) != 0)
    {
        for (int i = 0; i < 64; i++)
        {
            blocks[i] = (unsigned char)(header >> 8);
        }
    }
    else
    {
        const unsigned char* section = sections + ((header >> 1) & 0x7FFE);

        for (int i = 0; i < 64; i++)
        {
            blocks[i] = section[i];
        }
    }
}

void CompressedBlockStorage::compress(int sectionIndex)
{
    if (mData == NULL)
    {
        return;
    }

    if (mSize == 1024 && sectionIndex == -1)
    {
        return;
    }

    unsigned short newHeaders[512];
    unsigned char paletteIndices[256];
    unsigned char blocks[64];

    bool rebuild = sectionIndex >= 0;

    EnterCriticalSection(&sCompressLock);

    const unsigned short* oldHeaders = (const unsigned short*)mData;
    const unsigned char* oldSections = mData + 1024;

    int newSize = 0;
    int i;
    int j;

    if (sectionIndex == -1)
    {
        for (i = 0; i < 512; i++)
        {
            unsigned short header = oldHeaders[i];
            unsigned int format = header & 3;
            const unsigned char* section;

            if (format == 3)
            {
                section = oldSections + ((header >> 1) & 0x7FFE);

                if ((header & 4) != 0 || section == NULL)
                {
                    newHeaders[i] = header;
                    continue;
                }
            }
            else
            {
                int shift = 3 - format;
                unsigned int bitsPerBlock = 1 << format;
                unsigned int paletteSize = 1 << bitsPerBlock;
                unsigned int slotMask = 7 >> format;
                unsigned int indexMask = 0x3E >> shift;
                unsigned int valueMask = paletteSize - 1;
                const unsigned char* palette = oldSections + ((header >> 1) & 0x7FFE);
                const unsigned char* indices = palette + paletteSize;

                for (j = 0; j < 64; j++)
                {
                    blocks[j] = palette[(indices[(j >> shift) & indexMask] >> ((j & slotMask) * bitsPerBlock)) & valueMask];
                }

                section = blocks;
            }

            unsigned long long usedBlocks[4] = { 0, 0, 0, 0 };

            for (j = 0; j < 64; j++)
            {
                usedBlocks[section[j] & 3] |= 1ULL << (section[j] >> 2);
            }

            int usedCount = 0;

            for (j = 0; j < 256; j++)
            {
                if ((usedBlocks[j & 3] & (1ULL << (j >> 2))) != 0)
                {
                    usedCount++;
                }
            }

            if (usedCount == 1)
            {
                newHeaders[i] = 7;
                rebuild = true;
            }
            else if (usedCount == 2)
            {
                newHeaders[i] = 0;

                if (format != 0)
                {
                    rebuild = true;
                }

                newSize += 10;
            }
            else if (usedCount < 5)
            {
                newHeaders[i] = 1;

                if (format != 1)
                {
                    rebuild = true;
                }

                newSize += 20;
            }
            else if (usedCount < 17)
            {
                newHeaders[i] = 2;

                if (format != 2)
                {
                    rebuild = true;
                }

                newSize += 48;
            }
            else
            {
                newHeaders[i] = 3;
                newSize = ((newSize + 3) & ~3) + 64;
            }
        }
    }
    else
    {
        for (i = 0; i < 512; i++)
        {
            unsigned int format = oldHeaders[i] & 3;

            if (i == sectionIndex)
            {
                format = (format + 1) & 3;

                newHeaders[i] = (unsigned short)format;
            }
            else
            {
                newHeaders[i] = (unsigned short)format;

                if (format == 3)
                {
                    format = (oldHeaders[i] & 4) | 3;

                    newHeaders[i] = (unsigned short)format;
                }
            }

            if (format == 0)
            {
                newSize += 10;
            }
            else if (format == 1)
            {
                newSize += 20;
            }
            else if (format == 2)
            {
                newSize += 48;
            }
            else if (format == 3)
            {
                newSize = ((newSize + 3) & ~3) + 64;
            }
        }
    }

    if (rebuild)
    {
        newSize += 1024;

        unsigned char* newData = (unsigned char*)XPhysicalAlloc(newSize, 0xFFFFFFFF, 0x1000, 4);

        sAllocatedBytes += newSize;

        if (newData == NULL)
        {
            _MEMORYSTATUS memoryStatus;

            GlobalMemoryStatus(&memoryStatus);
            __debugbreak();
        }

        unsigned short* headers = (unsigned short*)newData;
        unsigned char* newSections = newData + 1024;
        unsigned short writeOffset = 0;

        for (i = 0; i < 512; i++)
        {
            unsigned int newFormat = newHeaders[i] & 3;
            unsigned int oldFormat = oldHeaders[i] & 3;
            unsigned short header = oldHeaders[i];

            headers[i] = (unsigned short)newFormat;

            if (oldFormat == newFormat && (oldFormat != 3 || ((header ^ newHeaders[i]) & 4) == 0))
            {
                if (oldFormat == 3)
                {
                    if ((header & 4) == 0)
                    {
                        writeOffset = (unsigned short)((writeOffset + 3) & 0xFFFC);

                        XMemCpy(newSections + writeOffset, oldSections + ((header >> 1) & 0x7FFE), 64);

                        headers[i] |= (unsigned short)((writeOffset << 1) & 0xFFFC);

                        writeOffset = (unsigned short)(writeOffset + 64);
                    }
                    else
                    {
                        headers[i] = header;
                    }
                }
                else
                {
                    unsigned int length = (8 << oldFormat) + (1 << (1 << oldFormat));

                    headers[i] = (unsigned short)(newFormat | ((writeOffset << 1) & 0xFFFC));

                    XMemCpy(newSections + writeOffset, oldSections + ((header >> 1) & 0x7FFE), length);

                    writeOffset = (unsigned short)(writeOffset + length);
                }

                continue;
            }

            const unsigned char* section;

            if (oldFormat == 3)
            {
                if ((header & 4) == 0)
                {
                    section = oldSections + ((header >> 1) & 0x7FFE);
                }
                else
                {
                    XMemSet(blocks, header >> 8, 64);

                    section = blocks;
                }
            }
            else
            {
                int shift = 3 - oldFormat;
                unsigned int bitsPerBlock = 1 << oldFormat;
                unsigned int paletteSize = 1 << bitsPerBlock;
                unsigned int slotMask = 7 >> oldFormat;
                unsigned int indexMask = 0x3E >> shift;
                unsigned int valueMask = paletteSize - 1;
                const unsigned char* palette = oldSections + ((header >> 1) & 0x7FFE);
                const unsigned char* indices = palette + paletteSize;

                for (j = 0; j < 64; j++)
                {
                    blocks[j] = palette[(indices[(j >> shift) & indexMask] >> ((j & slotMask) * bitsPerBlock)) & valueMask];
                }

                section = blocks;
            }

            XMemSet(paletteIndices, 0xFF, 256);

            if (newFormat == 3)
            {
                if ((newHeaders[i] & 4) == 0)
                {
                    writeOffset = (unsigned short)((writeOffset + 3) & 0xFFFC);

                    XMemCpy(newSections + writeOffset, section, 64);

                    headers[i] |= (unsigned short)((writeOffset << 1) & 0xFFFC);

                    writeOffset = (unsigned short)(writeOffset + 64);
                }
                else
                {
                    headers[i] = (unsigned short)((section[0] << 8) | 7);
                }
            }
            else
            {
                int shift = 3 - newFormat;
                unsigned int bitsPerBlock = 1 << newFormat;
                unsigned int paletteSize = 1 << bitsPerBlock;
                unsigned int indexBytes = 8 << newFormat;
                unsigned int slotMask = 7 >> newFormat;
                unsigned int indexMask = 0x3E >> shift;
                unsigned char* palette = newSections + writeOffset;
                unsigned char* indices = palette + paletteSize;

                XMemSet(palette, 0xFF, paletteSize);
                XMemSet(indices, 0, indexBytes);

                headers[i] |= (unsigned short)((writeOffset << 1) & 0xFFFC);

                writeOffset = (unsigned short)(writeOffset + paletteSize + indexBytes);

                int paletteCount = 0;

                for (j = 0; j < 64; j++)
                {
                    unsigned char block = section[j];

                    if (paletteIndices[block] == 0xFF)
                    {
                        paletteIndices[block] = (unsigned char)paletteCount;
                        palette[paletteCount] = block;

                        paletteCount++;
                    }

                    indices[(j >> shift) & indexMask] |= (unsigned char)(paletteIndices[block] << ((j & slotMask) * bitsPerBlock));
                }
            }
        }

        queueForDelete(mData, mSize);

        mData = newData;
        mSize = newSize;
    }

    LeaveCriticalSection(&sCompressLock);
}

void CompressedBlockStorage::write(DataOutputStream* out)
{
    out->writeInt(mSize);

    if (mData != NULL)
    {
        unsigned char* headers = new unsigned char[1024]();

        memcpy(headers, mData, 1024);

        reverseIndices(headers);

        arrayWithLength<unsigned char> headerData;

        headerData.data = headers;
        headerData.length = 1024;

        out->write(headerData);

        delete[] headers;

        if (mSize > 1024)
        {
            arrayWithLength<unsigned char> sectionData;

            sectionData.data = mData + 1024;
            sectionData.length = mSize - 1024;

            out->write(sectionData);
        }
    }
}

void CompressedBlockStorage::read(DataInputStream* in)
{
    int size = in->readInt();

    if (size > 0)
    {
        if (mData != NULL)
        {
            int oldSize = mSize;

            XPhysicalFree(mData);

            sAllocatedBytes -= oldSize;
        }

        arrayWithLength<unsigned char> data;

        data.data = (unsigned char*)XPhysicalAlloc(size, 0xFFFFFFFF, 0x1000, 4);

        sAllocatedBytes += size;

        mData = data.data;
        mSize = size;

        data.length = size;

        in->readFully(data);

        reverseIndices(mData);

        compress(-1);
    }
}
