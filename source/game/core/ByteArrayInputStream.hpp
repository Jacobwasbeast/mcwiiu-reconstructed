#ifndef GAME_CORE_BYTEARRAYINPUTSTREAM_HPP
#define GAME_CORE_BYTEARRAYINPUTSTREAM_HPP

#include "game/world/biome/arrayWithLength.hpp"
#include "game/core/InputStream.hpp"

class ByteArrayInputStream : public InputStream
{
public:
    ByteArrayInputStream(arrayWithLength<unsigned char> data);
    ByteArrayInputStream(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length);
    virtual ~ByteArrayInputStream();
    virtual int read();
    virtual int read(arrayWithLength<unsigned char> data);
    virtual int read(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length);
    virtual void close();
    virtual long long skip(long long count);
public:
    arrayWithLength<unsigned char> mBuffer;
    unsigned int mCount;
    unsigned int mMark;
    unsigned int mPosition;
};

#endif
