#include "game/core/ByteBuffer.hpp"

#include <cstring>

#include "game/core/FloatBuffer.hpp"
#include "game/core/IntBuffer.hpp"

ByteBuffer::ByteBuffer(unsigned int capacity)
    : Buffer(capacity)
{
    mIsWrapped = false;
    mBuffer = new unsigned char[capacity];
    memset(mBuffer, 0, capacity);
    mOrder = eByteOrder_BigEndian;
}

ByteBuffer::ByteBuffer(unsigned int capacity, unsigned char* bytes)
    : Buffer(capacity)
{
    mIsWrapped = true;
    mBuffer = bytes;
    mOrder = eByteOrder_BigEndian;
}

ByteBuffer::~ByteBuffer()
{
    if (!mIsWrapped)
    {
        delete[] mBuffer;
    }
}

ByteBuffer* ByteBuffer::flip()
{
    mLimit = mPosition;
    mPosition = 0;
    return this;
}

ByteBuffer* ByteBuffer::allocate(unsigned int capacity)
{
    return new ByteBuffer(capacity);
}

ByteBuffer* ByteBuffer::allocateDirect(int capacity)
{
    return new ByteBuffer(capacity);
}

ByteBuffer* ByteBuffer::wrap(arrayWithLength<unsigned char>& bytes)
{
    return new ByteBuffer(bytes.length, bytes.data);
}

void ByteBuffer::order(ByteOrder byteOrder)
{
    mOrder = byteOrder;
}

unsigned int ByteBuffer::getSize()
{
    return mCapacity;
}

int ByteBuffer::getInt()
{
    int value = getInt(mPosition);
    mPosition += 4;
    return value;
}

unsigned char ByteBuffer::get(int index)
{
    return mBuffer[index];
}

short ByteBuffer::getShort()
{
    unsigned int index = mPosition;
    short value;

    if (mOrder == eByteOrder_BigEndian)
    {
        value = (short)((mBuffer[index] << 8) | mBuffer[index + 1]);
    }
    else if (mOrder == eByteOrder_LittleEndian)
    {
        value = (short)((mBuffer[index + 1] << 8) | mBuffer[index]);
    }
    else
    {
        value = 0;
    }

    mPosition += 2;
    return value;
}

long long ByteBuffer::getLong()
{
    unsigned long long value = 0;
    unsigned int i;

    if (mOrder == eByteOrder_BigEndian)
    {
        for (i = 0; i < 8; ++i)
        {
            value = (value << 8) | mBuffer[mPosition + i];
        }
    }
    else if (mOrder == eByteOrder_LittleEndian)
    {
        for (i = 0; i < 8; ++i)
        {
            value |= ((unsigned long long)mBuffer[mPosition + i]) << (i * 8);
        }
    }

    mPosition += 8;
    return (long long)value;
}

void ByteBuffer::getShortArray(arrayWithLength<short>& values)
{
    unsigned int i;
    values.resize((mLimit - mPosition) >> 1);
    for (i = 0; i < values.length; ++i)
    {
        values.data[i] = getShort();
    }
}

ByteBuffer* ByteBuffer::put(int index, unsigned char value)
{
    mBuffer[index] = value;
    return this;
}

ByteBuffer* ByteBuffer::putInt(int value)
{
    putInt(mPosition, value);
    mPosition += 4;
    return this;
}

ByteBuffer* ByteBuffer::putShort(short value)
{
    unsigned int index = mPosition;
    if (mOrder == eByteOrder_BigEndian)
    {
        mBuffer[index] = (unsigned char)(value >> 8);
        mBuffer[index + 1] = (unsigned char)value;
    }
    else if (mOrder == eByteOrder_LittleEndian)
    {
        mBuffer[index] = (unsigned char)value;
        mBuffer[index + 1] = (unsigned char)(value >> 8);
    }
    mPosition += 2;
    return this;
}

void ByteBuffer::putShortArray(arrayWithLength<short>& values)
{
    unsigned int i;
    for (i = 0; i < values.length; ++i)
    {
        putShort(values.data[i]);
    }
}

ByteBuffer* ByteBuffer::putLong(long long value)
{
    unsigned int i;
    if (mOrder == eByteOrder_BigEndian)
    {
        for (i = 0; i < 8; ++i)
        {
            mBuffer[mPosition + i] = (unsigned char)(value >> ((7 - i) * 8));
        }
    }
    else if (mOrder == eByteOrder_LittleEndian)
    {
        for (i = 0; i < 8; ++i)
        {
            mBuffer[mPosition + i] = (unsigned char)(value >> (i * 8));
        }
    }
    mPosition += 8;
    return this;
}

arrayWithLength<unsigned char> ByteBuffer::array()
{
    arrayWithLength<unsigned char> result;
    result.data = mBuffer;
    result.length = mCapacity;
    return result;
}

unsigned char* ByteBuffer::getBuffer()
{
    return mBuffer;
}

int ByteBuffer::getInt(unsigned int index)
{
    unsigned char* bytes = mBuffer + index;

    if (mOrder == eByteOrder_BigEndian)
    {
        return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
    }

    if (mOrder == eByteOrder_LittleEndian)
    {
        return (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
    }

    return 0;
}

void ByteBuffer::putInt(unsigned int index, int value)
{
    if (mOrder == eByteOrder_BigEndian)
    {
        mBuffer[index] = (unsigned char)(value >> 24);
        mBuffer[index + 1] = (unsigned char)(value >> 16);
        mBuffer[index + 2] = (unsigned char)(value >> 8);
        mBuffer[index + 3] = (unsigned char)value;
        return;
    }

    if (mOrder == eByteOrder_LittleEndian)
    {
        mBuffer[index] = (unsigned char)value;
        mBuffer[index + 1] = (unsigned char)(value >> 8);
        mBuffer[index + 2] = (unsigned char)(value >> 16);
        mBuffer[index + 3] = (unsigned char)(value >> 24);
    }
}

ByteBuffer* ByteBuffer::put(arrayWithLength<unsigned char> bytes)
{
    memcpy(mBuffer + mPosition, bytes.data, bytes.length);
    mPosition += bytes.length;

    return this;
}

IntBuffer* ByteBuffer::asIntBuffer()
{
    return new IntBuffer((mLimit - mPosition) >> 2, (int*)(mBuffer + mPosition));
}

FloatBuffer* ByteBuffer::asFloatBuffer()
{
    return new FloatBuffer((mLimit - mPosition) >> 2, (float*)(mBuffer + mPosition));
}
