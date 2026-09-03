#include "game/core/ByteArrayInputStream.hpp"

void* XMemCpy(void* destination, const void* source, unsigned int size);

ByteArrayInputStream::ByteArrayInputStream(arrayWithLength<unsigned char> data)
    : mCount(data.length), mMark(0), mPosition(0)
{
    mBuffer.data = data.data;
    mBuffer.length = data.length;
}

ByteArrayInputStream::ByteArrayInputStream(arrayWithLength<unsigned char> data,
                                           unsigned int offset,
                                           unsigned int length)
    : mCount(offset + length < data.length ? offset + length : data.length),
      mMark(offset), mPosition(offset)
{
    mBuffer.data = data.data;
    mBuffer.length = data.length;
}

ByteArrayInputStream::~ByteArrayInputStream()
{
    if (mBuffer.data != NULL)
    {
        delete[] mBuffer.data;
    }
}

int ByteArrayInputStream::read()
{
    if (mCount <= mPosition)
    {
        return -1;
    }
    int position = mPosition;
    mPosition = position + 1;
    return (unsigned int)mBuffer.data[position];
}

int ByteArrayInputStream::read(arrayWithLength<unsigned char> data)
{
    return read(data, 0, data.length);
}

int ByteArrayInputStream::read(arrayWithLength<unsigned char> data,
                              unsigned int offset,
                              unsigned int length)
{
    int position = mPosition;
    if (position == (int)mCount)
    {
        return -1;
    }
    unsigned int available = mCount - position;
    if (available < length)
    {
        length = available;
    }
    XMemCpy(data.data + offset, mBuffer.data + position, length);
    mPosition = mPosition + length;
    return length;
}

void ByteArrayInputStream::close()
{
}

long long ByteArrayInputStream::skip(long long count)
{
    unsigned int position = mPosition;
    unsigned int newPosition = position + (int)count;
    if (mCount < newPosition)
    {
        newPosition = mCount;
    }
    mPosition = newPosition;
    return (int)((int)(newPosition - position) >> 31);
}
