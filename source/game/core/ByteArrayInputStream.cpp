#include "game/core/ByteArrayInputStream.hpp"

void* XMemCpy(void* destination, const void* source, unsigned int size);

ByteArrayInputStream::ByteArrayInputStream(arrayWithLength<unsigned char> data)
{
    mBuffer = data;
    mPosition = 0;
    mCount = data.length;
    mMark = 0;
}

ByteArrayInputStream::ByteArrayInputStream(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length)
{
    unsigned int end = offset + length;

    if (data.length < end)
    {
        end = data.length;
    }

    mBuffer = data;
    mPosition = offset;
    mCount = end;
    mMark = offset;
}

ByteArrayInputStream::~ByteArrayInputStream()
{
    delete[] mBuffer.data;
}

int ByteArrayInputStream::read()
{
    if (mPosition >= mCount)
    {
        return -1;
    }

    return mBuffer.data[mPosition++];
}

int ByteArrayInputStream::read(arrayWithLength<unsigned char> data)
{
    return read(data, 0, data.length);
}

int ByteArrayInputStream::read(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length)
{
    if (mPosition == mCount)
    {
        return -1;
    }

    unsigned int available = mCount - mPosition;

    if (available < length)
    {
        length = available;
    }

    XMemCpy(data.data + offset, mBuffer.data + mPosition, length);

    mPosition += length;

    return length;
}

void ByteArrayInputStream::close()
{
}

long long ByteArrayInputStream::skip(long long count)
{
    unsigned int newPosition = mPosition + (unsigned int)count;

    if (newPosition > mCount)
    {
        newPosition = mCount;
    }

    int skipped = (int)(newPosition - mPosition);

    mPosition = newPosition;

    return skipped;
}
