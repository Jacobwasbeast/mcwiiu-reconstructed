#ifndef GAME_CORE_INTBUFFER_HPP
#define GAME_CORE_INTBUFFER_HPP

#include "game/core/Buffer.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class IntBuffer : public Buffer
{
public:
    IntBuffer(unsigned int capacity);
    IntBuffer(unsigned int capacity, int* data);
    virtual ~IntBuffer();

    virtual IntBuffer* flip();

    void put(int value);
    void put(int index, int value);
    IntBuffer* put(arrayWithLength<int> values);
    IntBuffer* put(arrayWithLength<int>* values, unsigned int offset, unsigned int length);
    int get(unsigned int index);
    int* getBuffer();

protected:
    int* mBuffer;
};

#endif
