#include "game/core/BufferedReader.hpp"

#include <string.h>

BufferedReader::BufferedReader(Reader* reader)
{
    mReader = reader;
    mPosition = 0;
    mCount = 0;
    mCapacity = 64;
    mBuffer = new wchar_t[64];
    memset(mBuffer, 0, mCapacity * sizeof(wchar_t));

    bufferMore();
}

BufferedReader::~BufferedReader()
{
    delete[] mBuffer;
}

void BufferedReader::close()
{
    mReader->close();
}

void BufferedReader::bufferMore()
{
    if (mCount - mPosition <= 32)
    {
        if (mCount + 64 > mCapacity)
        {
            wchar_t* newBuffer = new wchar_t[mCapacity * 2];

            memset(newBuffer, 0, mCapacity * 2 * sizeof(wchar_t));
            memcpy(newBuffer, mBuffer, mCapacity * sizeof(wchar_t));

            delete[] mBuffer;

            mBuffer = newBuffer;
            mCapacity = mCapacity * 2;
        }

        unsigned int buffered = 0;
        int character;

        while ((character = mReader->read()) != -1)
        {
            mBuffer[mCount] = (wchar_t)character;
            mCount++;
            buffered++;

            if (buffered >= 64)
            {
                break;
            }
        }
    }
}

int BufferedReader::read()
{
    if (mCount == mPosition)
    {
        int character = mReader->read();

        if (character == -1)
        {
            return -1;
        }

        mBuffer[mCount] = (wchar_t)character;
        mCount++;

        bufferMore();
    }

    return mBuffer[mPosition++];
}

unsigned int BufferedReader::read(wchar_t* buffer, unsigned int offset, unsigned int length)
{
    if (mCount + length > mCapacity)
    {
        wchar_t* newBuffer = new wchar_t[mCapacity * 2];

        memset(newBuffer, 0, mCapacity * 2 * sizeof(wchar_t));
        memcpy(newBuffer, mBuffer, mCapacity * sizeof(wchar_t));

        delete[] mBuffer;

        mBuffer = newBuffer;
        mCapacity = mCapacity * 2;
    }

    unsigned int count = 0;

    while (count < length && mPosition <= mCount)
    {
        buffer[offset + count] = mBuffer[mPosition];
        mPosition++;
        count++;
    }

    while (count < length)
    {
        int character = mReader->read();

        if (character == -1)
        {
            break;
        }

        mBuffer[mCount] = (wchar_t)character;
        mCount++;

        buffer[offset + count] = (wchar_t)character;
        count++;

        mPosition++;
    }

    bufferMore();

    return count;
}

std::wstring BufferedReader::readLine()
{
    std::wstring line = L"";
    bool endOfLine = false;

    while (mPosition < mCount)
    {
        wchar_t character = mBuffer[mPosition];

        if (endOfLine)
        {
            if (character != L'\n' && character != L'\r')
            {
                break;
            }

            mPosition++;

            bufferMore();
        }
        else
        {
            mPosition++;

            if (character == L'\n' || character == L'\r')
            {
                endOfLine = true;

                bufferMore();
            }
            else
            {
                line.push_back(character);

                bufferMore();
            }
        }
    }

    return line;
}
