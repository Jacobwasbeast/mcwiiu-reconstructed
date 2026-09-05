#ifndef PLATFORM_WIIU_FFLIBUFFERALLOCATOR_HPP
#define PLATFORM_WIIU_FFLIBUFFERALLOCATOR_HPP

class FFLiBufferAllocator
{
public:
    void Init(void* buffer, unsigned int size);
    unsigned int Allocate(unsigned int size, unsigned int alignment);
    unsigned int Allocate(unsigned int size);
    unsigned int GetRestSize() const;
    bool CanAllocate(unsigned int size, unsigned int alignment) const;

private:
    unsigned int mBuffer;
    unsigned int mRestSize;
};

#endif
