#ifndef GAME_CORE_BUFFER_HPP
#define GAME_CORE_BUFFER_HPP

class Buffer
{
public:
    Buffer(unsigned int capacity);
    virtual ~Buffer();

    virtual void flip() = 0;

    void clear();
    void limit(unsigned int newLimit);
    unsigned int limit();
    void position(unsigned int newPosition);
    unsigned int position();
    int remaining();

protected:
    unsigned int mCapacity;
    unsigned int mPosition;
    unsigned int mLimit;
    int mMark;
    bool mIsWrapped;
};

#endif
