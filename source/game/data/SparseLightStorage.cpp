#include "game/data/SparseLightStorage.hpp"

#include <cstdlib>
#include <cstring>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/XLockFreeStack.hpp"

void* XMemSet(void* destination, int value, unsigned int size);
void* XMemCpy(void* destination, const void* source, unsigned int size);
long long InterlockedCompareExchangeRelease64(volatile long long* destination, long long exchange, long long comparand);

static const int CHUNK_WIDTH = 16;
static const int PLANE_COUNT = 128;
static const int PLANE_BYTES = 128;
static const int PLANE_VALUES = 256;
static const int COLUMN_BYTES = 64;
static const unsigned int PLANE_ALL_DARK = 0x80;
static const unsigned int PLANE_ALL_BRIGHT = 0x81;
static const int PENDING_DELETE_LIST_COUNT = 3;

static int sAllocatedBytes = 0;

static int sPendingDeleteList;
static XLockFreeStack<unsigned char> sPendingDeleteLists[PENDING_DELETE_LIST_COUNT];

SparseLightStorage::SparseLightStorage(bool allBright)
{
    int planeCount = PLANE_COUNT - 1;
    int size = planeCount * PLANE_BYTES + PLANE_BYTES;

    unsigned char* newData = (unsigned char*)malloc(size);

    sAllocatedBytes += size;

    newData[PLANE_COUNT - 1] = (unsigned char)(allBright ? PLANE_ALL_BRIGHT : PLANE_ALL_DARK);

    for (int plane = 0; plane < planeCount; plane++)
    {
        newData[plane] = (unsigned char)plane;
    }

    XMemSet(newData + PLANE_COUNT, 0, planeCount * PLANE_BYTES);

    mPlaneCountAndData = ((long long)planeCount << 48) | (unsigned int)newData;
}

SparseLightStorage::SparseLightStorage(bool allBright, bool compressed)
{
    unsigned char* newData = (unsigned char*)malloc(PLANE_BYTES);

    sAllocatedBytes += PLANE_BYTES;

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        newData[plane] = (unsigned char)(allBright ? PLANE_ALL_BRIGHT : PLANE_ALL_DARK);
    }

    mPlaneCountAndData = (unsigned int)newData;
}

int SparseLightStorage::getDataSize(long long countAndData)
{
    return (int)((countAndData >> 48) & 0xffff) * PLANE_BYTES + PLANE_BYTES;
}

SparseLightStorage::SparseLightStorage(SparseLightStorage* other)
{
    long long countAndData = other->mPlaneCountAndData;

    unsigned char* sourceData = (unsigned char*)(unsigned int)countAndData;
    unsigned int planeCount = (unsigned int)(countAndData >> 48) & 0xffff;

    int size = getDataSize(countAndData);

    unsigned char* newData = (unsigned char*)malloc(size);

    sAllocatedBytes += size;

    mPlaneCountAndData = ((long long)planeCount << 48) | (unsigned int)newData;

    XMemCpy(newData, sourceData, size);
}

SparseLightStorage::~SparseLightStorage()
{
    unsigned char* data = (unsigned char*)(unsigned int)mPlaneCountAndData;

    int size = getDataSize(mPlaneCountAndData);

    mPlaneCountAndData = 0;

    free(data);

    sAllocatedBytes -= size;
}

void SparseLightStorage::queueForDelete(unsigned char* data, int size)
{
    sPendingDeleteLists[sPendingDeleteList].Push(data);
}

void SparseLightStorage::updateDataAndCount(long long countAndData)
{
    long long oldCountAndData;
    unsigned char* oldData;
    int oldSize;

    do
    {
        oldCountAndData = mPlaneCountAndData;
        oldData = (unsigned char*)(unsigned int)oldCountAndData;
        oldSize = getDataSize(oldCountAndData);
    }
    while (InterlockedCompareExchangeRelease64(&mPlaneCountAndData, countAndData, oldCountAndData) != oldCountAndData);

    queueForDelete(oldData, oldSize);
}

void SparseLightStorage::setData(arrayWithLength<unsigned char> data, unsigned int offset)
{
    unsigned char planeIndices[PLANE_COUNT];

    int planeCount = 0;

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        const unsigned char* column = data.data + offset + plane / 2;

        int shift = (plane & 1) * 4;

        bool allDark = true;
        bool allBright = true;

        for (int i = 0; i < PLANE_VALUES; i++)
        {
            int value = (column[i * COLUMN_BYTES] >> shift) & 0xf;

            if (value == 0)
            {
                allBright = false;
            }
            else
            {
                allDark = false;

                if (value != 0xf)
                {
                    allBright = false;
                }
            }
        }

        if (allDark)
        {
            planeIndices[plane] = (unsigned char)PLANE_ALL_DARK;
        }
        else if (allBright)
        {
            planeIndices[plane] = (unsigned char)PLANE_ALL_BRIGHT;
        }
        else
        {
            planeIndices[plane] = (unsigned char)planeCount;
            planeCount++;
        }
    }

    int size = planeCount * PLANE_BYTES + PLANE_BYTES;

    unsigned char* newData = (unsigned char*)malloc(size);

    sAllocatedBytes += size;

    unsigned char* target = newData + PLANE_COUNT;

    XMemCpy(newData, planeIndices, PLANE_COUNT);

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        if (newData[plane] < PLANE_ALL_DARK)
        {
            const unsigned char* column = data.data + offset + plane / 2;

            int shift = (plane & 1) * 4;

            for (int i = 0; i < PLANE_VALUES; i += 2)
            {
                *target = (unsigned char)((column[i * COLUMN_BYTES] >> shift) & 0xf);
                *target |= (unsigned char)(((column[(i + 1) * COLUMN_BYTES] >> shift) & 0xf) << 4);

                target++;
            }
        }
    }

    updateDataAndCount(((long long)planeCount << 48) | (unsigned int)newData);
}

void SparseLightStorage::getPlaneIndicesAndData(unsigned char** planeIndices, unsigned char** planeData)
{
    unsigned char* data = (unsigned char*)(unsigned int)mPlaneCountAndData;

    *planeIndices = data;
    *planeData = data + PLANE_COUNT;
}

void SparseLightStorage::getData(arrayWithLength<unsigned char> data, unsigned int offset)
{
    unsigned char* destination = data.data + offset;

    XMemSet(destination, 0, PLANE_COUNT * PLANE_BYTES);

    unsigned char* planeIndices;
    unsigned char* planeData;

    getPlaneIndicesAndData(&planeIndices, &planeData);

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        unsigned int index = planeIndices[plane];

        if (index == PLANE_ALL_DARK)
        {
            continue;
        }

        int shift = (plane & 1) * 4;

        unsigned char* column = destination + plane / 2;

        if (index == PLANE_ALL_BRIGHT)
        {
            for (int i = 0; i < PLANE_VALUES; i++)
            {
                column[i * COLUMN_BYTES] |= (unsigned char)(0xf << shift);
            }
        }
        else
        {
            const unsigned char* source = planeData + index * PLANE_BYTES;

            for (int i = 0; i < PLANE_VALUES; i++)
            {
                column[i * COLUMN_BYTES] |= (unsigned char)((((source[i / 2] >> ((i & 1) * 4)) & 0xf) << shift));
            }
        }
    }
}

int SparseLightStorage::get(int x, int y, int z)
{
    unsigned char* planeIndices;
    unsigned char* planeData;

    getPlaneIndicesAndData(&planeIndices, &planeData);

    unsigned int index = planeIndices[y];

    if (index == PLANE_ALL_DARK)
    {
        return 0;
    }

    if (index == PLANE_ALL_BRIGHT)
    {
        return 0xf;
    }

    int value = x * CHUNK_WIDTH + z;

    return (planeData[index * PLANE_BYTES + value / 2] >> ((value & 1) * 4)) & 0xf;
}

void SparseLightStorage::addNewPlane(std::vector<int>& planes)
{
    for (;;)
    {
        long long oldCountAndData = mPlaneCountAndData;

        unsigned char* oldData = (unsigned char*)(unsigned int)oldCountAndData;
        unsigned int oldPlaneCount = (unsigned int)(oldCountAndData >> 48) & 0xffff;

        int oldSize = getDataSize(oldCountAndData);

        unsigned int planeCount = oldPlaneCount;

        std::vector<int>::iterator it = planes.begin();

        while (it != planes.end())
        {
            if (oldData[*it] < PLANE_ALL_DARK)
            {
                it = planes.erase(it);
            }
            else
            {
                ++it;
                planeCount++;
            }
        }

        int size = planeCount * PLANE_BYTES + PLANE_BYTES;

        unsigned char* newData = (unsigned char*)malloc(size);

        sAllocatedBytes += size;

        XMemCpy(newData, oldData, oldPlaneCount * PLANE_BYTES + PLANE_BYTES);

        unsigned char nextPlane = (unsigned char)oldPlaneCount;

        for (std::vector<int>::iterator plane = planes.begin(); plane != planes.end(); ++plane)
        {
            if (oldData[*plane] == PLANE_ALL_DARK)
            {
                XMemSet(newData + PLANE_COUNT + nextPlane * PLANE_BYTES, 0, PLANE_BYTES);

                newData[*plane] = nextPlane;

                nextPlane++;
            }
            else if (oldData[*plane] == PLANE_ALL_BRIGHT)
            {
                XMemSet(newData + PLANE_COUNT + nextPlane * PLANE_BYTES, 0xff, PLANE_BYTES);

                newData[*plane] = nextPlane;

                nextPlane++;
            }
        }

        long long newCountAndData = ((long long)planeCount << 48) | (unsigned int)newData;

        if (InterlockedCompareExchangeRelease64(&mPlaneCountAndData, newCountAndData, oldCountAndData) == oldCountAndData)
        {
            queueForDelete(oldData, oldSize);
            return;
        }

        free(newData);
    }
}

void SparseLightStorage::set(int x, int y, int z, int value)
{
    unsigned char* planeIndices;
    unsigned char* planeData;

    getPlaneIndicesAndData(&planeIndices, &planeData);

    unsigned int index = planeIndices[y];

    if (index >= PLANE_ALL_DARK)
    {
        if (value == 0)
        {
            if (index == PLANE_ALL_DARK)
            {
                return;
            }
        }
        else if (value == 0xf && index == PLANE_ALL_BRIGHT)
        {
            return;
        }

        std::vector<int> planes;

        planes.push_back(y);

        addNewPlane(planes);

        getPlaneIndicesAndData(&planeIndices, &planeData);

        index = planeIndices[y];
    }

    int position = x * CHUNK_WIDTH + z;
    int shift = (position & 1) * 4;

    unsigned char* target = planeData + index * PLANE_BYTES + position / 2;

    *target = (unsigned char)((*target & (0xf0 >> shift)) | (value << shift));
}

void SparseLightStorage::setAllBright()
{
    unsigned char* newData = (unsigned char*)malloc(PLANE_BYTES);

    sAllocatedBytes += PLANE_BYTES;

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        newData[plane] = (unsigned char)PLANE_ALL_BRIGHT;
    }

    updateDataAndCount((unsigned int)newData);
}

int SparseLightStorage::setDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1, int z1, int offset)
{
    int planePairs = (y1 - y0) / 2;

    y0 = y0 & ~1;

    std::vector<int> planes;

    planes.reserve(planePairs * 2);

    for (int i = 0; i < planePairs; i++)
    {
        planes.push_back(y0 + i * 2);
        planes.push_back(y0 + i * 2 + 1);
    }

    addNewPlane(planes);

    unsigned char* source = data.data + offset;
    unsigned char* current = source;

    for (int x = x0; x < x1; x++)
    {
        for (int z = z0; z < z1; z++)
        {
            for (int i = 0; i < planePairs; i++)
            {
                set(x, y0 + i * 2, z, *current & 0xf);
                set(x, y0 + i * 2 + 1, z, *current >> 4);

                current++;
            }
        }
    }

    return (int)(current - source);
}

int SparseLightStorage::getDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1, int z1, int offset)
{
    unsigned char* destination = data.data + offset;
    unsigned char* current = destination;

    if (x0 < x1)
    {
        int planePairs = (y1 - y0) / 2;

        y0 = y0 & ~1;

        for (int x = x0; x < x1; x++)
        {
            for (int z = z0; z < z1; z++)
            {
                for (int i = 0; i < planePairs; i++)
                {
                    *current = (unsigned char)get(x, y0 + i * 2, z);
                    *current |= (unsigned char)(get(x, y0 + i * 2 + 1, z) << 4);

                    current++;
                }
            }
        }
    }

    return (int)(current - destination);
}

void SparseLightStorage::staticCtor()
{
}

unsigned int SparseLightStorage::compress()
{
    long long oldCountAndData = mPlaneCountAndData;

    unsigned char* oldData = (unsigned char*)(unsigned int)oldCountAndData;
    unsigned int oldPlaneCount = (unsigned int)(oldCountAndData >> 48) & 0xffff;

    int oldSize = getDataSize(oldCountAndData);

    const unsigned char* oldPlaneData = oldData + PLANE_COUNT;

    unsigned char planeIndices[PLANE_COUNT];

    unsigned int planeCount = 0;
    bool changed = false;

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        unsigned int index = oldData[plane];

        if (index == PLANE_ALL_DARK)
        {
            planeIndices[plane] = (unsigned char)PLANE_ALL_DARK;
        }
        else if (index == PLANE_ALL_BRIGHT)
        {
            planeIndices[plane] = (unsigned char)PLANE_ALL_BRIGHT;
        }
        else
        {
            const char* values = (const char*)(oldPlaneData + index * PLANE_BYTES);

            bool allDark = true;
            bool allBright = true;

            for (int i = 0; i < PLANE_BYTES; i++)
            {
                if (values[i] == 0)
                {
                    allBright = false;
                }
                else
                {
                    allDark = false;

                    if (values[i] != -1)
                    {
                        allBright = false;
                    }
                }
            }

            if (allDark)
            {
                changed = true;
                planeIndices[plane] = (unsigned char)PLANE_ALL_DARK;
            }
            else if (allBright)
            {
                changed = true;
                planeIndices[plane] = (unsigned char)PLANE_ALL_BRIGHT;
            }
            else
            {
                planeIndices[plane] = (unsigned char)planeCount;
                planeCount++;
            }
        }
    }

    if (!changed)
    {
        return oldPlaneCount;
    }

    int size = planeCount * PLANE_BYTES + PLANE_BYTES;

    unsigned char* newData = (unsigned char*)malloc(size);

    sAllocatedBytes += size;

    unsigned char* target = newData + PLANE_COUNT;

    XMemCpy(newData, planeIndices, PLANE_COUNT);

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        if (newData[plane] < PLANE_ALL_DARK)
        {
            XMemCpy(target, oldPlaneData + oldData[plane] * PLANE_BYTES, PLANE_BYTES);

            target += PLANE_BYTES;
        }
    }

    long long newCountAndData = ((long long)planeCount << 48) | (unsigned int)newData;

    if (InterlockedCompareExchangeRelease64(&mPlaneCountAndData, newCountAndData, oldCountAndData) != oldCountAndData)
    {
        queueForDelete(newData, size);

        return planeCount;
    }

    queueForDelete(oldData, oldSize);

    return planeCount;
}

bool SparseLightStorage::isCompressed()
{
    return (int)((mPlaneCountAndData >> 48) & 0xffff) < PLANE_COUNT - 1;
}

unsigned int SparseLightStorage::getRawDataMaxBytes()
{
    return PLANE_COUNT * PLANE_BYTES + PLANE_BYTES;
}

unsigned int SparseLightStorage::getRawData(unsigned char* buffer)
{
    unsigned int size = getDataSize(mPlaneCountAndData);

    memcpy(buffer, (unsigned char*)(unsigned int)mPlaneCountAndData, size);

    return size;
}

void SparseLightStorage::setFromRawData(unsigned char* rawData, unsigned int size)
{
    unsigned char* newData = (unsigned char*)malloc(size);

    sAllocatedBytes += size;

    memcpy(newData, rawData, size);

    updateDataAndCount(((long long)((size - PLANE_BYTES) / PLANE_BYTES) << 48) | (unsigned int)newData);
}

void SparseLightStorage::tick()
{
    XLockFreeStack<unsigned char>& pending = sPendingDeleteLists[(sPendingDeleteList + 1) % PENDING_DELETE_LIST_COUNT];

    unsigned char* data;

    do
    {
        data = pending.Pop();

        free(data);
    }
    while (data != NULL);

    sPendingDeleteList = (sPendingDeleteList + 1) % PENDING_DELETE_LIST_COUNT;
}

void SparseLightStorage::write(DataOutputStream* out)
{
    long long countAndData = mPlaneCountAndData;

    out->writeInt((int)((countAndData >> 48) & 0xffff));

    arrayWithLength<unsigned char> data;

    data.data = (unsigned char*)(unsigned int)countAndData;
    data.length = getDataSize(countAndData);

    out->write(data);
}

void SparseLightStorage::read(DataInputStream* in, unsigned char* buffer)
{
    unsigned char* rawData = buffer;

    if (rawData == NULL)
    {
        rawData = (unsigned char*)malloc(getRawDataMaxBytes());
    }

    arrayWithLength<unsigned char> raw;

    raw.data = rawData;
    raw.length = in->readInt() * PLANE_BYTES + PLANE_BYTES;

    in->readFully(raw);

    const unsigned char* rawPlaneData = rawData + PLANE_COUNT;

    unsigned char planeIndices[PLANE_COUNT];

    int planeCount = 0;

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        unsigned int index = rawData[plane];

        if (index == PLANE_ALL_DARK)
        {
            planeIndices[plane] = (unsigned char)PLANE_ALL_DARK;
        }
        else if (index == PLANE_ALL_BRIGHT)
        {
            planeIndices[plane] = (unsigned char)PLANE_ALL_BRIGHT;
        }
        else
        {
            const char* values = (const char*)(rawPlaneData + index * PLANE_BYTES);

            bool allDark = true;
            bool allBright = true;

            for (int i = 0; i < PLANE_BYTES; i++)
            {
                if (values[i] == 0)
                {
                    allBright = false;
                }
                else
                {
                    allDark = false;

                    if (values[i] != -1)
                    {
                        allBright = false;
                    }
                }
            }

            if (allDark)
            {
                planeIndices[plane] = (unsigned char)PLANE_ALL_DARK;
            }
            else if (allBright)
            {
                planeIndices[plane] = (unsigned char)PLANE_ALL_BRIGHT;
            }
            else
            {
                planeIndices[plane] = (unsigned char)planeCount;
                planeCount++;
            }
        }
    }

    int size = planeCount * PLANE_BYTES + PLANE_BYTES;

    unsigned char* newData = (unsigned char*)malloc(size);

    sAllocatedBytes += size;

    unsigned char* target = newData + PLANE_COUNT;

    XMemCpy(newData, planeIndices, PLANE_COUNT);

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        if (newData[plane] < PLANE_ALL_DARK)
        {
            XMemCpy(target, rawPlaneData + rawData[plane] * PLANE_BYTES, PLANE_BYTES);

            target += PLANE_BYTES;
        }
    }

    updateDataAndCount(((long long)planeCount << 48) | (unsigned int)newData);

    if (buffer == NULL)
    {
        free(rawData);
    }
}
