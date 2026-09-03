#ifndef SOURCE_PLATFORM_WIIU_TLSSTORAGECAFE_HPP
#define SOURCE_PLATFORM_WIIU_TLSSTORAGECAFE_HPP

class TLSStorageCafe
{
public:
    TLSStorageCafe();
    int Alloc();
    static TLSStorageCafe* Instance();
    void Free(unsigned int index);
    unsigned int GetValue(unsigned int index);
    unsigned int SetValue(unsigned int index, void* value);

private:
    static __thread void* m_values[64];
    static TLSStorageCafe* m_instance;
};

#endif
