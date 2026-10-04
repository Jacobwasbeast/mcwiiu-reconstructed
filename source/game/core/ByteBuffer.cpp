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

unsigned char* ByteBuffer::getBuffer()
{
    return mBuffer;
}

unsigned int ByteBuffer::getInt(unsigned int index)
{
    unsigned char* bytes = mBuffer + index;
    unsigned char first = bytes[0];
    unsigned char second = bytes[1];
    unsigned char third = bytes[2];
    unsigned char fourth = bytes[3];

    if (mOrder == eByteOrder_BigEndian)
    {
        return ((unsigned int)first << 24) | ((unsigned int)second << 16) | ((unsigned int)third << 8) | (unsigned int)fourth;
    }

    if (mOrder == eByteOrder_LittleEndian)
    {
        return ((unsigned int)fourth << 24) | ((unsigned int)third << 16) | ((unsigned int)second << 8) | (unsigned int)first;
    }

    return 0;
}

void ByteBuffer::putInt(unsigned int index, int value)
{
    if (mOrder == eByteOrder_BigEndian)
    {
        mBuffer[index] = (unsigned char)((value >> 24) & 0xFF);
        mBuffer[index + 1] = (unsigned char)((value >> 16) & 0xFF);
        mBuffer[index + 2] = (unsigned char)((value >> 8) & 0xFF);
        mBuffer[index + 3] = (unsigned char)value;
        return;
    }

    if (mOrder == eByteOrder_LittleEndian)
    {
        mBuffer[index] = (unsigned char)value;
        mBuffer[index + 1] = (unsigned char)((value >> 8) & 0xFF);
        mBuffer[index + 2] = (unsigned char)((value >> 16) & 0xFF);
        mBuffer[index + 3] = value >> 24;
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
