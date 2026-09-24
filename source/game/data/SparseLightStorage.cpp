#include "game/data/SparseLightStorage.hpp"


#include <cstdlib>
#include <cstring>
#include <cstddef>

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

    mDataAndCount.data = newData;
    mDataAndCount.planeCount = (unsigned int)planeCount;
}

SparseLightStorage::SparseLightStorage(bool allBright, bool compressed)
{
    unsigned char* newData = (unsigned char*)malloc(PLANE_BYTES);

    sAllocatedBytes += PLANE_BYTES;

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        newData[plane] = (unsigned char)(allBright ? PLANE_ALL_BRIGHT : PLANE_ALL_DARK);
    }

    mDataAndCount.data = newData;
    mDataAndCount.planeCount = 0;
}

int SparseLightStorage::getDataSize(unsigned int planeCount)
{
    return (int)planeCount * PLANE_BYTES + PLANE_BYTES;
}

SparseLightStorage::SparseLightStorage(SparseLightStorage* other)
{
    unsigned char* sourceData = other->mDataAndCount.data;
    unsigned int planeCount = other->mDataAndCount.planeCount;

    int size = getDataSize(planeCount);

    unsigned char* newData = (unsigned char*)malloc(size);

    sAllocatedBytes += size;

    mDataAndCount.data = newData;
    mDataAndCount.planeCount = planeCount;

    XMemCpy(newData, sourceData, size);
}

SparseLightStorage::~SparseLightStorage()
{
    unsigned char* data = mDataAndCount.data;

    int size = getDataSize(mDataAndCount.planeCount);

    mDataAndCount.data = NULL;
    mDataAndCount.planeCount = 0;

    free(data);

    sAllocatedBytes -= size;
}

void SparseLightStorage::queueForDelete(unsigned char* data, int size)
{
    sPendingDeleteLists[sPendingDeleteList].Push(data);
}

void SparseLightStorage::updateDataAndCount(unsigned char* data, unsigned int planeCount)
{
    static_assert(sizeof(void*) == sizeof(unsigned int), "SparseLightStorage requires the target 32-bit pointer ABI");
    static_assert(sizeof(DataAndCount) == sizeof(long long), "SparseLightStorage data/count pair must be 64 bits");
    static_assert(offsetof(DataAndCount, planeCount) == 0, "plane count must lead the published pair");
    static_assert(offsetof(DataAndCount, data) == sizeof(unsigned int), "data pointer must follow plane count");

    DataAndCount replacement;
    replacement.planeCount = planeCount;
    replacement.data = data;

    long long expected;
    long long exchange;
    memcpy(&exchange, &replacement, sizeof(exchange));

    for (;;)
    {
        DataAndCount observed;
        memcpy(&observed, &mDataAndCount, sizeof(observed));
        memcpy(&expected, &observed, sizeof(expected));

        unsigned char* oldData = observed.data;
        int oldSize = getDataSize(observed.planeCount);
        long long previous = InterlockedCompareExchangeRelease64(
            reinterpret_cast<volatile long long*>(&mDataAndCount), exchange, expected);

        if (previous == expected)
        {
            queueForDelete(oldData, oldSize);
            return;
        }
    }
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

    updateDataAndCount(newData, (unsigned int)planeCount);
}

void SparseLightStorage::getPlaneIndicesAndData(unsigned char** planeIndices, unsigned char** planeData)
{
    unsigned char* data = mDataAndCount.data;

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
        DataAndCount observed;
        memcpy(&observed, &mDataAndCount, sizeof(observed));
        unsigned char* oldData = observed.data;
        unsigned int oldPlaneCount = observed.planeCount;
        int oldSize = getDataSize(oldPlaneCount);

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

        DataAndCount replacement;
        replacement.planeCount = planeCount;
        replacement.data = newData;

        long long expected;
        long long exchange;
        memcpy(&expected, &observed, sizeof(expected));
        memcpy(&exchange, &replacement, sizeof(exchange));

        long long previous = InterlockedCompareExchangeRelease64(
            reinterpret_cast<volatile long long*>(&mDataAndCount), exchange, expected);
        if (previous == expected)
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

    updateDataAndCount(newData, 0);
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
    DataAndCount observed;
    memcpy(&observed, &mDataAndCount, sizeof(observed));
    unsigned char* oldData = observed.data;
    unsigned int oldPlaneCount = observed.planeCount;
    int oldSize = getDataSize(oldPlaneCount);

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

    DataAndCount replacement;
    replacement.planeCount = planeCount;
    replacement.data = newData;

    long long expected;
    long long exchange;
    memcpy(&expected, &observed, sizeof(expected));
    memcpy(&exchange, &replacement, sizeof(exchange));

    long long previous = InterlockedCompareExchangeRelease64(
        reinterpret_cast<volatile long long*>(&mDataAndCount), exchange, expected);
    if (previous != expected)
    {
        queueForDelete(newData, size);
        return planeCount;
    }

    queueForDelete(oldData, oldSize);
    return planeCount;
}

bool SparseLightStorage::isCompressed()
{
    return mDataAndCount.planeCount < PLANE_COUNT - 1;
}

int SparseLightStorage::getAllocatedBytes()
{
    return sAllocatedBytes;
}

unsigned int SparseLightStorage::getRawDataMaxBytes()
{
    return PLANE_COUNT * PLANE_BYTES + PLANE_BYTES;
}

unsigned int SparseLightStorage::getRawData(unsigned char* buffer)
{
    unsigned int size = getDataSize(mDataAndCount.planeCount);

    memcpy(buffer, mDataAndCount.data, size);

    return size;
}

void SparseLightStorage::setFromRawData(unsigned char* rawData, unsigned int size)
{
    unsigned char* newData = (unsigned char*)malloc(size);

    sAllocatedBytes += size;

    memcpy(newData, rawData, size);

    updateDataAndCount(newData, (size - PLANE_BYTES) / PLANE_BYTES);
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
    out->writeInt((int)mDataAndCount.planeCount);

    arrayWithLength<unsigned char> data;

    data.data = mDataAndCount.data;
    data.length = getDataSize(mDataAndCount.planeCount);

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

    updateDataAndCount(newData, (unsigned int)planeCount);

    if (buffer == NULL)
    {
        free(rawData);
    }
}
