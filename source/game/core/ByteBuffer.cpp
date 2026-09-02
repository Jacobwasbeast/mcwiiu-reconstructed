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
