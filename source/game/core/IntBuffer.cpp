#include "game/core/IntBuffer.hpp"

#include <cstring>

IntBuffer::IntBuffer(unsigned int capacity) : Buffer(capacity)
{
    mBuffer = new int[capacity]();
}

IntBuffer::IntBuffer(unsigned int capacity, int* data) : Buffer(capacity)
{
    mBuffer = data;
    mIsWrapped = true;
}

IntBuffer::~IntBuffer()
{
    if (!mIsWrapped)
    {
        delete[] mBuffer;
    }
}

IntBuffer* IntBuffer::flip()
{
    mLimit = mPosition;
    mPosition = 0;

    return this;
}

void IntBuffer::put(int value)
{
    mBuffer[mPosition++] = value;
}

void IntBuffer::put(int index, int value)
{
    mBuffer[index] = value;
}

IntBuffer* IntBuffer::put(arrayWithLength<int>* values, unsigned int offset, unsigned int length)
{
    memcpy(mBuffer + mPosition, values->data + offset, length * sizeof(int));
    mPosition += length;

    return this;
}

int IntBuffer::get(unsigned int index)
{
    return mBuffer[index];
}
