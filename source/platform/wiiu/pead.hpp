#ifndef PLATFORM_WIIU_PEAD_HPP
#define PLATFORM_WIIU_PEAD_HPP

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

struct OSThread;

namespace nn
{
    namespace pia
    {
        namespace common { class BackgroundScheduler; }
        namespace transport
        {
            class PacketHandler;
            class StationPacketHandler;
            class TransportThreadStream;
        }
    }
}

namespace pead
{
class Arena;
class Heap;
class StackSymbolResolver;
class StreamFormat;
class WriteStream;

struct Endian
{
    enum Types
    {
        cBig = 0,
        cLittle = 1,
        cHost = cBig
    };
};

template <typename T>
class SafeStringBase
{
public:
    class iterator
    {
    public:
        iterator(const T* current = 0) : mCurrent(current) {}
        const T& operator*() const { return *mCurrent; }
        iterator& operator++() { ++mCurrent; return *this; }
        bool operator==(const iterator& other) const { return mCurrent == other.mCurrent; }
        bool operator!=(const iterator& other) const { return mCurrent != other.mCurrent; }
        const T* get() const { return mCurrent; }
    private:
        const T* mCurrent;
    };

    SafeStringBase(const T* text = 0) : mStringTop(text ? text : emptyString()) {}
    virtual ~SafeStringBase() {}
    virtual SafeStringBase& operator=(const SafeStringBase& other)
    {
        mStringTop = other.mStringTop;
        return *this;
    }
    SafeStringBase& __as__S(const SafeStringBase& other);
    virtual void assureTerminationImpl_() const {}
    const T* cstr() const { assureTerminationImpl_(); return mStringTop; }
    iterator begin() const { return iterator(mStringTop); }
    iterator end() const
    {
        const T* end = mStringTop;
        while (*end) ++end;
        return iterator(end);
    }
    int calcLength() const
    {
        const T* end = mStringTop;
        while (*end) ++end;
        return static_cast<int>(end - mStringTop);
    }
protected:
    static const T* emptyString()
    {
        static const T empty[1] = { 0 };
        return empty;
    }
    const T* mStringTop;
};

template <typename T>
class Buffer
{
public:
    Buffer(T* buffer = 0, int size = 0) : mBuffer(buffer), mSize(size) {}
    T* begin() const { return mBuffer; }
    T* end() const { return mBuffer + mSize; }
    int size() const { return mSize; }
    const T& operator[](int index) const { return mBuffer[index]; }
private:
    T* mBuffer;
    int mSize;
};

template <typename T>
class BitFlag
{
public:
    BitFlag(T value = 0) : mValue(value) {}
    T getDirect() const { return mValue; }
private:
    T mValue;
};

template <typename T>
class BufferedSafeStringBase : public SafeStringBase<T>
{
public:
    BufferedSafeStringBase(T* buffer = 0, int size = 0)
        : SafeStringBase<T>(buffer), mBuffer(buffer), mBufferSize(size)
    {
        if (mBuffer && mBufferSize > 0) mBuffer[0] = 0;
    }
    virtual ~BufferedSafeStringBase() {}
    virtual BufferedSafeStringBase& operator=(const SafeStringBase<T>& other)
    {
        copy(other.cstr());
        return *this;
    }
    BufferedSafeStringBase& __as__S(const SafeStringBase<T>& other);
    virtual void assureTerminationImpl_() const
    {
        if (mBuffer && mBufferSize > 0) mBuffer[mBufferSize - 1] = 0;
    }
    static int formatImpl_(T* buffer, int size, const T* format, va_list args);
    static int formatImpl___S(T* buffer, int size, const T* format, va_list args);
    int formatV(const T* format, va_list args) { return formatImpl_(mBuffer, mBufferSize, format, args); }
    int formatV__S(const T* format, va_list args);
    int format(const T* format, ...);
    int format__S(const T* format, ...);
    int appendWithFormatV(const T* format, va_list args);
    int appendWithFormatV__S(const T* format, va_list args);
    int appendWithFormat(const T* format, ...);
    int appendWithFormat__S(const T* format, ...);
    void assureTerminationImpl___S() const;
    int getBufferSize() const { return mBufferSize; }
    T* getBuffer() { return mBuffer; }
    const T* getBuffer() const { return mBuffer; }
    void clear() { if (mBuffer && mBufferSize > 0) mBuffer[0] = 0; }
    void copy(const T* text);
protected:
    T* mBuffer;
    int mBufferSize;
};

template <typename T>
class StringBuilderBase : public BufferedSafeStringBase<T>
{
public:
    StringBuilderBase(T* buffer, int size) : BufferedSafeStringBase<T>(buffer, size) {}
};

template <typename T>
class HeapSafeStringBase : public BufferedSafeStringBase<T>
{
public:
    HeapSafeStringBase(T* buffer = 0, int size = 0)
        : BufferedSafeStringBase<T>(buffer, size) {}
    virtual ~HeapSafeStringBase();
    virtual HeapSafeStringBase& operator=(const SafeStringBase<T>& other)
    {
        BufferedSafeStringBase<T>::operator=(other);
        return *this;
    }
    HeapSafeStringBase& __as__S(const SafeStringBase<T>& other);
};

template <int N>
class FixedSafeStringBase : public BufferedSafeStringBase<char>
{
public:
    FixedSafeStringBase() : BufferedSafeStringBase<char>(mStorage, N) {}
    FixedSafeStringBase(const SafeStringBase<char>& text)
        : BufferedSafeStringBase<char>(mStorage, N) { this->copy(text.cstr()); }
    virtual ~FixedSafeStringBase() {}
    virtual FixedSafeStringBase& operator=(const SafeStringBase<char>& text)
    {
        this->copy(text.cstr());
        return *this;
    }
private:
    char mStorage[N];
};

template <int N>
class FixedSafeString : public FixedSafeStringBase<N>
{
public:
    FixedSafeString() {}
    FixedSafeString(const SafeStringBase<char>& text) : FixedSafeStringBase<N>(text) {}
    virtual ~FixedSafeString() {}
    virtual FixedSafeString& operator=(const SafeStringBase<char>& text)
    {
        FixedSafeStringBase<N>::operator=(text);
        return *this;
    }
};

typedef FixedSafeString<32> FixedSafeString32;
typedef FixedSafeString<128> FixedSafeString128;
typedef FixedSafeString<256> FixedSafeString256;
typedef FixedSafeString<1024> FixedSafeString1024;

namespace RuntimeTypeInfo
{
    class Interface
    {
    public:
        virtual ~Interface() {}
        virtual bool isDerived(const Interface*) const = 0;
    };

    class Root : public Interface
    {
    public:
        virtual bool isDerived(const Interface* type) const { return type == this; }
    };

    template <typename T>
    class Derive : public Root
    {
    public:
        virtual bool isDerived(const Interface* type) const
        {
            return type == this || Root::isDerived(type);
        }
    };
}

class ListNode
{
public:
    ListNode() : mPrev(this), mNext(this) {}
    void insertBack_(ListNode* node);
    void insertFront_(ListNode* node);
    void erase_();
    ListNode* prev() const { return mPrev; }
    ListNode* next() const { return mNext; }
private:
    ListNode* mPrev;
    ListNode* mNext;
};

class Random;

class ListImpl
{
public:
    ListImpl() : mStart(), mCount(0) {}
    ListNode* nth(int index) const;
    void sort(int offset, int (*compare)(const void*, const void*));
    void clear();
    void reverse();
    void swap(ListNode*, ListNode*);
    void shuffle(Random*);
    static ListNode* mergeSortImpl(ListNode*, ListNode*, int, int, int (*)(const void*, const void*));
    void mergeSort(int, int (*)(const void*, const void*));
    void popBack();
    void popFront();
    int indexOf(const ListNode*) const;
    void moveAfter(ListNode*, ListNode*);
    void moveBefore(ListNode*, ListNode*);
    ListNode* find(const void*, int, int (*)(const void*, const void*)) const;
    void uniq(int, int (*)(const void*, const void*));
    bool checkLinks() const;
private:
    ListNode mStart;
    int mCount;
};

class PtrArrayImpl
{
public:
    PtrArrayImpl() : mBuffer(0), mSize(0), mCapacity(0), mAllocated(false) {}
    void setBuffer(int, void*);
    bool tryAllocBuffer(int, Heap*, int);
    void allocBuffer(int, Heap*, int);
    void freeBuffer();
    void erase(int, int);
    bool checkInsert(int, int);
    void insert(int, void*);
    int binarySearch(const void*, int (*)(const void*, const void*)) const;
    void reverse();
    void shuffle(Random*);
    void insertArray(int, void*, int, int);
    void sort(int (*)(const void*, const void*));
    void heapSort(int (*)(const void*, const void*));
    int compare(const PtrArrayImpl&, int (*)(const void*, const void*)) const;
    void uniq(int (*)(const void*, const void*));
private:
    void** mBuffer;
    int mSize;
    int mCapacity;
    bool mAllocated;
};

class Random
{
public:
    Random() { init(); }
    void init(unsigned int);
    void init();
    void init(unsigned int, unsigned int, unsigned int, unsigned int);
    unsigned int getU32();
    unsigned long long getU64();
    void getContext(unsigned int*, unsigned int*, unsigned int*, unsigned int*) const;
private:
    unsigned int mContext[4];
};

class StreamSrc
{
public:
    virtual ~StreamSrc() {}
    virtual int skip(int) = 0;
    virtual unsigned int read(void*, unsigned int) = 0;
    virtual unsigned int write(const void*, unsigned int) = 0;
    virtual void rewind() = 0;
    virtual bool isEOF() = 0;
    virtual bool flush() { return true; }
};

class BufferReadStreamSrc : public StreamSrc
{
public:
    BufferReadStreamSrc(StreamSrc*, void*, unsigned int);
    virtual ~BufferReadStreamSrc();
    virtual int skip(int);
    virtual unsigned int read(void*, unsigned int);
    virtual unsigned int write(const void*, unsigned int);
    virtual void rewind();
    virtual bool isEOF();
private:
    const unsigned char* mBuffer;
    unsigned int mSize;
    unsigned int mPosition;
    StreamSrc* mOriginalSrc;
};

class BufferWriteStreamSrc : public StreamSrc
{
public:
    BufferWriteStreamSrc(StreamSrc*, void*, unsigned int);
    virtual ~BufferWriteStreamSrc();
    virtual int skip(int);
    virtual unsigned int read(void*, unsigned int);
    virtual unsigned int write(const void*, unsigned int);
    virtual void rewind();
    virtual bool isEOF();
    virtual bool flush();
protected:
    unsigned char* mBuffer;
    unsigned int mBufferSize;
    unsigned int mPosition;
    StreamSrc* mOriginalSrc;
};

class BufferMultiByteTextWriteStreamSrc : public BufferWriteStreamSrc
{
public:
    BufferMultiByteTextWriteStreamSrc(StreamSrc*, void*, unsigned int);
    virtual ~BufferMultiByteTextWriteStreamSrc();
    virtual unsigned int write(const void*, unsigned int);
};

class BufferMultiByteNullTerminatedTextWriteStreamSrc : public BufferMultiByteTextWriteStreamSrc
{
public:
    virtual ~BufferMultiByteNullTerminatedTextWriteStreamSrc();
    virtual bool flush();
};

class StreamFormat
{
public:
    virtual ~StreamFormat() {}
    virtual unsigned char readU8(StreamSrc*, Endian::Types) = 0;
    virtual unsigned short readU16(StreamSrc*, Endian::Types) = 0;
    virtual unsigned int readU32(StreamSrc*, Endian::Types) = 0;
    virtual unsigned long long readU64(StreamSrc*, Endian::Types) = 0;
    virtual signed char readS8(StreamSrc*, Endian::Types) = 0;
    virtual short readS16(StreamSrc*, Endian::Types) = 0;
    virtual int readS32(StreamSrc*, Endian::Types) = 0;
    virtual long long readS64(StreamSrc*, Endian::Types) = 0;
    virtual float readF32(StreamSrc*, Endian::Types) = 0;
    virtual void readBit(StreamSrc*, void*, unsigned int) = 0;
    virtual void readString(StreamSrc*, BufferedSafeStringBase<char>*, unsigned int) = 0;
    virtual void readMemBlock(StreamSrc*, void*, unsigned int) = 0;
    virtual void writeU8(StreamSrc*, Endian::Types, unsigned char) = 0;
    virtual void writeU16(StreamSrc*, Endian::Types, unsigned short) = 0;
    virtual void writeU32(StreamSrc*, Endian::Types, unsigned int) = 0;
    virtual void writeU64(StreamSrc*, Endian::Types, unsigned long long) = 0;
    virtual void writeS8(StreamSrc*, Endian::Types, signed char) = 0;
    virtual void writeS16(StreamSrc*, Endian::Types, short) = 0;
    virtual void writeS32(StreamSrc*, Endian::Types, int) = 0;
    virtual void writeS64(StreamSrc*, Endian::Types, long long) = 0;
    virtual void writeF32(StreamSrc*, Endian::Types, float) = 0;
    virtual void writeBit(StreamSrc*, const void*, unsigned int) = 0;
    virtual void writeString(StreamSrc*, const SafeStringBase<char>&, unsigned int) = 0;
    virtual void writeMemBlock(StreamSrc*, const void*, unsigned int) = 0;
    virtual void writeDecorationText(StreamSrc*, const SafeStringBase<char>&) = 0;
    virtual void writeNullChar(StreamSrc*) = 0;
    virtual void skip(StreamSrc*, unsigned int) = 0;
    virtual void flush(StreamSrc*) = 0;
    virtual void rewind(StreamSrc*) = 0;
};

class BinaryStreamFormat : public StreamFormat
{
public:
    virtual unsigned char readU8(StreamSrc*, Endian::Types);
    virtual unsigned short readU16(StreamSrc*, Endian::Types);
    virtual unsigned int readU32(StreamSrc*, Endian::Types);
    virtual unsigned long long readU64(StreamSrc*, Endian::Types);
    virtual signed char readS8(StreamSrc*, Endian::Types);
    virtual short readS16(StreamSrc*, Endian::Types);
    virtual int readS32(StreamSrc*, Endian::Types);
    virtual long long readS64(StreamSrc*, Endian::Types);
    virtual float readF32(StreamSrc*, Endian::Types);
    virtual void readBit(StreamSrc*, void*, unsigned int);
    virtual void readString(StreamSrc*, BufferedSafeStringBase<char>*, unsigned int);
    virtual void readMemBlock(StreamSrc*, void*, unsigned int);
    virtual void writeU8(StreamSrc*, Endian::Types, unsigned char);
    virtual void writeU16(StreamSrc*, Endian::Types, unsigned short);
    virtual void writeU32(StreamSrc*, Endian::Types, unsigned int);
    virtual void writeU64(StreamSrc*, Endian::Types, unsigned long long);
    virtual void writeS8(StreamSrc*, Endian::Types, signed char);
    virtual void writeS16(StreamSrc*, Endian::Types, short);
    virtual void writeS32(StreamSrc*, Endian::Types, int);
    virtual void writeS64(StreamSrc*, Endian::Types, long long);
    virtual void writeF32(StreamSrc*, Endian::Types, float);
    virtual void writeBit(StreamSrc*, const void*, unsigned int);
    virtual void writeString(StreamSrc*, const SafeStringBase<char>&, unsigned int);
    virtual void writeMemBlock(StreamSrc*, const void*, unsigned int);
    virtual void writeDecorationText(StreamSrc*, const SafeStringBase<char>&);
    virtual void writeNullChar(StreamSrc*);
    virtual void skip(StreamSrc*, unsigned int);
    virtual void flush(StreamSrc*);
    virtual void rewind(StreamSrc*);
};

class TextStreamFormat : public BinaryStreamFormat
{
public:
    TextStreamFormat();
    virtual ~TextStreamFormat();
    virtual unsigned char readU8(StreamSrc*, Endian::Types);
    virtual unsigned short readU16(StreamSrc*, Endian::Types);
    virtual unsigned int readU32(StreamSrc*, Endian::Types);
    virtual unsigned long long readU64(StreamSrc*, Endian::Types);
    virtual signed char readS8(StreamSrc*, Endian::Types);
    virtual short readS16(StreamSrc*, Endian::Types);
    virtual int readS32(StreamSrc*, Endian::Types);
    virtual long long readS64(StreamSrc*, Endian::Types);
    virtual float readF32(StreamSrc*, Endian::Types);
    virtual void readBit(StreamSrc*, void*, unsigned int);
    virtual void readString(StreamSrc*, BufferedSafeStringBase<char>*, unsigned int);
    virtual void readMemBlock(StreamSrc*, void*, unsigned int);
    virtual void writeU8(StreamSrc*, Endian::Types, unsigned char);
    virtual void writeU16(StreamSrc*, Endian::Types, unsigned short);
    virtual void writeU32(StreamSrc*, Endian::Types, unsigned int);
    virtual void writeU64(StreamSrc*, Endian::Types, unsigned long long);
    virtual void writeS8(StreamSrc*, Endian::Types, signed char);
    virtual void writeS16(StreamSrc*, Endian::Types, short);
    virtual void writeS32(StreamSrc*, Endian::Types, int);
    virtual void writeS64(StreamSrc*, Endian::Types, long long);
    virtual void writeF32(StreamSrc*, Endian::Types, float);
    virtual void writeBit(StreamSrc*, const void*, unsigned int);
    virtual void writeString(StreamSrc*, const SafeStringBase<char>&, unsigned int);
    virtual void writeMemBlock(StreamSrc*, const void*, unsigned int);
    virtual void writeDecorationText(StreamSrc*, const SafeStringBase<char>&);
    virtual void writeNullChar(StreamSrc*);
    virtual void skip(StreamSrc*, unsigned int);
    virtual void rewind(StreamSrc*);
    virtual void flush(StreamSrc*);
private:
    void getNextData_(StreamSrc*);
    FixedSafeString128 mTextBuffer;
};

class Stream
{
public:
    enum Modes { cBinary = 0, cText = 1 };
    Stream();
    Stream(StreamSrc*, Modes);
    Stream(StreamSrc*, StreamFormat*);
    virtual ~Stream();
    void skip(unsigned int);
    void skip(unsigned int, unsigned int);
    void rewind();
    bool isEOF();
    void setUserFormat(StreamFormat*);
    void setMode(Modes);
    void setBinaryEndian(Endian::Types);
    StreamSrc* getStreamSrc() const { return mSrc; }
    StreamFormat* getStreamFormat() const { return mFormat; }
protected:
    StreamSrc* mSrc;
    StreamFormat* mFormat;
    Endian::Types mEndian;
    Modes mMode;
};

class ReadStream : public Stream
{
public:
    ReadStream();
    ReadStream(StreamSrc*, Modes);
    ReadStream(StreamSrc*, StreamFormat*);
    virtual ~ReadStream();
    unsigned char readU8();
    unsigned short readU16();
    unsigned int readU32();
    unsigned long long readU64();
    signed char readS8();
    short readS16();
    int readS32();
    long long readS64();
    float readF32();
    void readBit(void*, unsigned int);
    void readString(BufferedSafeStringBase<char>*, unsigned int);
    void readMemBlock(void*, unsigned int);
    void readU8(unsigned char&);
    void readU16(unsigned short&);
    void readU32(unsigned int&);
    void readU64(unsigned long long&);
    void readS8(signed char&);
    void readS16(short&);
    void readS32(int&);
    void readS64(long long&);
    void readF32(float&);
    float readF32BitImpl_(unsigned int, unsigned int);
    double readF64BitImpl_(unsigned int, unsigned int);
};

class WriteStream : public Stream
{
public:
    WriteStream();
    WriteStream(StreamSrc*, Modes);
    WriteStream(StreamSrc*, StreamFormat*);
    virtual ~WriteStream();
    void flush();
    void writeU8(unsigned char);
    void writeU16(unsigned short);
    void writeU32(unsigned int);
    void writeU64(unsigned long long);
    void writeS8(signed char);
    void writeS16(short);
    void writeS32(int);
    void writeS64(long long);
    void writeF32(float);
    void writeBit(const void*, unsigned int);
    void writeString(const SafeStringBase<char>&, unsigned int);
    void writeMemBlock(const void*, unsigned int);
    void writeComment(const SafeStringBase<char>&);
    void writeLineComment(const SafeStringBase<char>&);
    void writeDecorationText(const SafeStringBase<char>&);
    void writeNullChar();
    void writeF32BitImpl_(float, unsigned int, unsigned int);
    void writeF64BitImpl_(double, unsigned int, unsigned int);
};

class PrintStreamSrc : public StreamSrc
{
public:
    virtual unsigned int read(void*, unsigned int);
    virtual unsigned int write(const void*, unsigned int);
    virtual int skip(int);
    virtual void rewind();
    virtual bool isEOF();
};

class PrintWriteStream : public WriteStream
{
public:
    PrintWriteStream(Modes);
    PrintWriteStream(StreamFormat*);
    virtual ~PrintWriteStream();
private:
    PrintStreamSrc mPrintSrc;
};

class BufferReadStream : public ReadStream
{
public:
    BufferReadStream(ReadStream*, const void*, unsigned int);
    virtual ~BufferReadStream();
private:
    BufferReadStreamSrc mBufferSrc;
};

class BufferWriteStream : public WriteStream
{
public:
    BufferWriteStream(WriteStream*, void*, unsigned int);
    virtual ~BufferWriteStream();
private:
    BufferWriteStreamSrc mBufferSrc;
};

class BufferMultiByteTextWriteStream : public WriteStream
{
public:
    BufferMultiByteTextWriteStream(WriteStream*, void*, unsigned int);
    virtual ~BufferMultiByteTextWriteStream();
private:
    BufferMultiByteTextWriteStreamSrc mBufferSrc;
};

class Base64
{
public:
    static void encode(char*, const void*, unsigned int, bool);
    static bool decode(void*, unsigned int, const char*, unsigned int, unsigned int*);
};

class BitFlagUtil
{
public:
    static unsigned int countOnBit(unsigned int);
    static int findOnBitFromRight(unsigned int, int);
    static unsigned int countRightOnBit(unsigned int, int);
    static unsigned int countRightOnBit64(unsigned long long, int);
    static int findOnBitFromRight64(unsigned long long, int);
};

class CoreId
{
public:
    CoreId();
    explicit CoreId(int id) : mId(id) {}
    int value() const { return mId; }
private:
    int mId;
};

class CoreIdMask
{
public:
    CoreIdMask(unsigned int mask = 0) : mMask(mask) {}
    int countOnBits() const;
    unsigned int value() const { return mMask; }
private:
    unsigned int mMask;
};

class CoreInfo
{
public:
    static void configure();
    static int getPlatformMask(CoreId);
    static void dump();
};

class IDisposer
{
public:
    enum HeapNullOption { cHeapNullOption_Assert = 0, cHeapNullOption_Allow = 1 };
    IDisposer();
    IDisposer(Heap*, HeapNullOption);
    virtual ~IDisposer();
private:
    friend class Heap;
    ListNode mDisposerNode;
    Heap* mHeap;
};

template <typename T>
class IDelegate1
{
public:
    virtual ~IDelegate1() {}
    virtual void invoke(T) = 0;
    virtual IDelegate1* clone(Heap*) const { return 0; }
};

template <typename T1, typename T2>
class IDelegate2
{
public:
    virtual ~IDelegate2() {}
    virtual void invoke(T1, T2) = 0;
    virtual IDelegate2* clone(Heap*) const { return 0; }
};

template <typename Owner, typename T1, typename T2>
class Delegate2 : public IDelegate2<T1, T2>
{
public:
    typedef void (Owner::*Method)(T1, T2);
    Delegate2(Owner* owner, Method method) : mOwner(owner), mMethod(method) {}
    virtual void invoke(T1 first, T2 second) { (mOwner->*mMethod)(first, second); }
    virtual IDelegate2<T1, T2>* clone(Heap*) const { return new Delegate2(*this); }
private:
    Owner* mOwner;
    Method mMethod;
};

template <typename T>
class DelegateEvent
{
public:
    class Slot : public ListNode
    {
    public:
        Slot() : mDelegate(0) {}
        IDelegate1<T>* mDelegate;
    };
    DelegateEvent() : mEnabled(true) {}
    ~DelegateEvent() {}
    void invoke(T argument)
    {
        for (ListNode* node = mSlots.next(); node != &mSlots; node = node->next())
            static_cast<Slot*>(node)->mDelegate->invoke(argument);
    }
private:
    ListNode mSlots;
    bool mEnabled;
};

class Heap : public IDisposer
{
    friend class HeapMgr;
public:
    enum HeapDirection { cHeapDirection_Forward = 1, cHeapDirection_Reverse = -1 };
    Heap(const SafeStringBase<char>&, Heap*, void*, unsigned int, HeapDirection, bool);
    virtual ~Heap();
    virtual bool checkDerivedRuntimeTypeInfo(const RuntimeTypeInfo::Interface*) const;
    virtual const RuntimeTypeInfo::Interface* getRuntimeTypeInfo() const;
    virtual void destroy() = 0;
    virtual unsigned int adjust() = 0;
    virtual void* tryAlloc(unsigned int, int) = 0;
    virtual void free(void*) = 0;
    virtual unsigned int resizeFront(void*, unsigned int) = 0;
    virtual unsigned int resizeBack(void*, unsigned int) = 0;
    virtual void* tryRealloc(void*, unsigned int, int);
    virtual void freeAll() = 0;
    virtual void* getStartAddress() const = 0;
    virtual void* getEndAddress() const = 0;
    virtual unsigned int getSize() const = 0;
    virtual unsigned int getFreeSize() const = 0;
    virtual unsigned int getMaxAllocatableSize(int) const = 0;
    virtual bool isInclude(const void*) const = 0;
    virtual bool isEmpty() const = 0;
    virtual bool isFreeable() const = 0;
    virtual bool isResizable() const = 0;
    virtual bool isAdjustable() const = 0;
    virtual void dump() const;
    virtual void dumpYAML(WriteStream&, int) const;
    virtual void pushBackChild_(Heap*);
    void dumpTreeYAML(WriteStream&, int) const;
    Heap* findContainHeap_(const void*);
    void dispose_(const void*, const void*);
    void eraseChild_(Heap*);
    void destruct_();
    void appendDisposer_(IDisposer*);
    void removeDisposer_(IDisposer*);
    void checkAccessThread_() const;
protected:
    SafeStringBase<char> mName;
    Heap* mParent;
    ListNode mChildren;
    ListNode mDisposers;
    void* mStart;
    unsigned int mSize;
    HeapDirection mDirection;
    bool mEnableLock;
};

class MemBlock
{
public:
    MemBlock* previous;
    MemBlock* next;
    unsigned char* address;
    void* payload;
    unsigned int span;
    unsigned int size;
    int alignment;
};

class ExpHeap : public Heap
{
public:
    enum FindMode { cFindMode_FirstFit = 0, cFindMode_BestFit = 1 };
    enum AllocMode { cAllocMode_FirstFit = 0, cAllocMode_BestFit = 1 };
    ExpHeap(const SafeStringBase<char>&, Heap*, void*, unsigned int, HeapDirection, bool);
    virtual ~ExpHeap();
    static ExpHeap* tryCreate(unsigned int, const SafeStringBase<char>&, Heap*, HeapDirection, bool);
    static ExpHeap* create(unsigned int, const SafeStringBase<char>&, Heap*, HeapDirection, bool);
    static ExpHeap* tryCreate(void*, unsigned int, const SafeStringBase<char>&, bool);
    static int getManagementAreaSize(int);
    virtual void destroy();
    virtual unsigned int adjust();
    virtual void* tryAlloc(unsigned int, int);
    virtual void free(void*);
    virtual unsigned int resizeFront(void*, unsigned int);
    virtual unsigned int resizeBack(void*, unsigned int);
    virtual void* tryRealloc(void*, unsigned int, int);
    virtual void freeAll();
    virtual void* getStartAddress() const;
    virtual void* getEndAddress() const;
    virtual unsigned int getSize() const;
    virtual unsigned int getFreeSize() const;
    virtual unsigned int getMaxAllocatableSize(int) const;
    virtual bool isInclude(const void*) const;
    virtual bool isEmpty() const;
    virtual bool isFreeable() const;
    virtual bool isResizable() const;
    virtual bool isAdjustable() const;
    virtual void dump() const;
    virtual void dumpYAML(WriteStream&, int) const;
    void setAllocMode(AllocMode);
    AllocMode getAllocMode() const;
    unsigned int getAllocatedSize(void*);
    void dumpUseList() const;
    void dumpFreeList() const;
    bool tryCheckFreeList() const;
    bool tryCheckUseList() const;
private:
    void pushToFreeList_(MemBlock*);
    static void createMaxSizeFreeMemBlock_(ExpHeap*);
    static void doCreate(ExpHeap*, Heap*);
    static bool compareMemBlockAddr_(const MemBlock*, const MemBlock*);
    MemBlock* findLastMemBlockIfFree_();
    int adjustBack_();
    MemBlock* findFirstMemBlockIfFree_();
    int adjustFront_();
    MemBlock* findFreeMemBlockFromHead_(unsigned int, FindMode) const;
    MemBlock* findFreeMemBlockFromHead_(unsigned int, int, FindMode) const;
    void pushToUseList_(MemBlock*);
    void* allocFromHead_(unsigned int);
    void* allocFromHead_(unsigned int, int);
    MemBlock* findFreeMemBlockFromTail_(unsigned int, FindMode) const;
    MemBlock* findFreeMemBlockFromTail_(unsigned int, int, FindMode) const;
    void* allocFromTail_(unsigned int);
    void* allocFromTail_(unsigned int, int);
    void* realloc_(void*, unsigned char*, unsigned int, unsigned int, int);
    void removeFreeBlock_(MemBlock*);
    void removeUseBlock_(MemBlock*);
    AllocMode mAllocMode;
    unsigned int mFreeSize;
    MemBlock* mFreeHead;
    MemBlock* mFreeTail;
    MemBlock* mUseHead;
    MemBlock* mUseTail;
    bool mOwnsMemory;
};

class FindContainHeapCache
{
public:
    FindContainHeapCache();
    bool tryRemoveHeap(Heap*);
private:
    Heap* mHeap;
};

class HeapMgr
{
public:
    struct AllocCallbackArg { Heap* heap; void* address; unsigned int size; };
    struct FreeCallbackArg { Heap* heap; void* address; };
    struct AllocFailedCallbackArg { Heap* heap; unsigned int size; int alignment; };
    HeapMgr();
    ~HeapMgr();
    static void initialize(Arena*);
    static void destroy();
    Heap* findContainHeap(const void*) const;
    Heap* findHeapByName(const SafeStringBase<char>&, int) const;
    Heap* getCurrentHeap() const;
    static bool isContainedInAnyHeap(const void*);
    static void dumpTreeYAML(WriteStream&);
    static void setAllocFailedCallback(IDelegate1<const AllocFailedCallbackArg*>*);
    void setCurrentHeap_(Heap*);
    static HeapMgr* instance();
private:
    static void createRootHeap_();
    static void initializeImpl_();
    static Heap* findHeapByName_(Heap*, const SafeStringBase<char>&, int*);
    static void removeFromFindContainHeapCache_(Heap*);
    Heap* mRootHeap;
    Heap* mCurrentHeap;
};

void* allocateFromSeadHeap(unsigned int, Heap*);
void* allocateFromSeadHeap(unsigned int);
void* allocateFromSeadHeap(unsigned int, Heap*, int);
void freeToSeadHeap(void*);
unsigned int getArraySize(void*);

template <typename T>
T* NewArray(unsigned int count, Heap* heap, int alignment)
{
    T* array = static_cast<T*>(allocateFromSeadHeap(sizeof(T) * count, heap, alignment));
    if (!array) return 0;
    for (unsigned int index = 0; index < count; ++index) new (array + index) T();
    return array;
}

template <typename T>
void DeleteArray(T* array)
{
    if (!array) return;
    unsigned int count = getArraySize(array) / sizeof(T);
    while (count) array[--count].~T();
    freeToSeadHeap(array);
}

class CriticalSection : public IDisposer
{
public:
    CriticalSection();
    CriticalSection(Heap*);
    virtual ~CriticalSection();
    void lock();
    void unlock();
    bool tryLock();
    void dump(StackSymbolResolver*);
private:
    unsigned int mStorage[11];
};

class Mutex : public IDisposer
{
public:
    Mutex();
    Mutex(Heap*);
    virtual ~Mutex();
    void lock();
    void unlock();
    bool tryLock();
private:
    unsigned int mStorage[11];
};

template <typename Lock>
class ScopedLock
{
public:
    ScopedLock(Lock* lock) : mLock(lock) { if (mLock) mLock->lock(); }
    ~ScopedLock() { if (mLock) mLock->unlock(); }
private:
    Lock* mLock;
};

template <typename Lock>
class ConditionalScopedLock
{
public:
    ConditionalScopedLock(Lock* lock, bool condition) : mLock(condition ? lock : 0)
    {
        if (mLock) mLock->lock();
    }
    ~ConditionalScopedLock() { if (mLock) mLock->unlock(); }
private:
    Lock* mLock;
};

class TickSpan
{
public:
    TickSpan(long long ticks = 0) : mTicks(ticks) {}
    long long toNanoSeconds() const;
    void setNanoSeconds(long long);
    long long getTicks() const { return mTicks; }
private:
    long long mTicks;
};

class Event : public IDisposer
{
public:
    Event();
    Event(Heap*);
    Event(bool);
    Event(Heap*, bool);
    virtual ~Event();
    void initialize(bool);
    void wait();
    bool wait(TickSpan);
    void setSignal();
    void resetSignal();
private:
    unsigned int mStorage[9];
};

class MessageQueue
{
public:
    enum BlockType { cBlock = 0, cNoBlock = 1 };
    MessageQueue();
    ~MessageQueue();
    void allocate(int, Heap*);
    void free();
    bool push(int, BlockType);
    int pop(BlockType);
    bool jam(int, BlockType);
    int peek(BlockType) const;
private:
    int* mMessages;
    int mCapacity;
    int mCount;
    int mRead;
    int mWrite;
    Heap* mHeap;
    unsigned int mStorage[10];
};

class Thread : public IDisposer
{
public:
    Thread(Heap*, OSThread*);
    Thread(const SafeStringBase<char>&, Heap*, int, MessageQueue::BlockType, int, int, int);
    virtual ~Thread();
    virtual void destroy();
    virtual bool sendMessage(int, MessageQueue::BlockType);
    virtual int recvMessage(MessageQueue::BlockType);
    virtual MessageQueue* getMessageQueue() const;
    virtual void start();
    virtual void quit(bool);
    virtual void waitDone();
    virtual void quitAndDestroySingleThread(bool);
    virtual void quitAndWaitDoneSingleThread(bool);
    virtual void setPriority(int);
    virtual int getPriority() const;
    virtual MessageQueue::BlockType getBlockType() const;
    virtual int getStackSize() const;
    virtual int calcStackUsedSizePeak() const;
    virtual void calc_(int) = 0;
    virtual void* getStackCheckStartAddress_() const;
    void setAffinity(const CoreIdMask&);
    static int cafeThreadFunc_(int, void*);
    static void sleep(TickSpan);
    static void yield();
    void run_();
    void checkStackOverFlow(const char*, int) const;
    void checkStackEndCorruption(const char*, int) const;
    void checkStackPointerOverFlow(const char*, int) const;
    void setStackOverflowExceptionEnable(bool);
    void initStackCheck_();
    void initStackCheckWithCurrentStackPointer_();
protected:
    OSThread* mThread;
    MessageQueue mMessageQueue;
    void* mStack;
    int mStackSize;
    int mPriority;
    MessageQueue::BlockType mBlockType;
    bool mQuitRequested;
};

class MainThread : public Thread
{
public:
    MainThread(Heap*, OSThread*);
    virtual ~MainThread();
    virtual void destroy();
    virtual void quit(bool);
    virtual void waitDone();
    virtual void quitAndDestroySingleThread(bool);
    virtual void setPriority(int);
    virtual void calc_(int);
};

class DelegateThread : public Thread
{
public:
    DelegateThread(const SafeStringBase<char>&, IDelegate2<Thread*, int>*, Heap*, int,
                   MessageQueue::BlockType, int, int, int);
    virtual ~DelegateThread();
    virtual void calc_(int);
private:
    IDelegate2<Thread*, int>* mDelegate;
};

class ThreadMgr
{
public:
    class SingletonDisposer_ : public IDisposer
    {
    public:
        virtual ~SingletonDisposer_();
    };
    ThreadMgr();
    ~ThreadMgr();
    static void createInstance(Heap*);
    static void deleteInstance();
    void initialize(Heap*);
    void destroy();
    Thread* getCurrentThread() const;
    bool isMainThread() const;
    static void waitDoneMultipleThread(const Thread**, int);
    static void quitAndWaitDoneMultipleThread(Thread**, int, bool);
    static void checkCurrentThreadStackOverFlow(const char*, int);
    static void checkCurrentThreadStackEndCorruption(const char*, int);
    static void checkCurrentThreadStackPointerOverFlow(const char*, int);
private:
    void initMainThread_(Heap*);
    void destroyMainThread_();
    static unsigned int getCurrentThreadID_();
    MainThread* mMainThread;
};

class PrintFormatter;

class PrintOutput
{
public:
    virtual ~PrintOutput();
    virtual void write(const char*, int) = 0;
    void writeLineBreak();
    void operator<<(PrintFormatter&);
    void __ls(PrintFormatter&);
};

class BufferingPrintOutput : public PrintOutput
{
public:
    virtual ~BufferingPrintOutput();
    virtual void write(const char*, int);
};

class StringPrintOutput : public PrintOutput
{
public:
    StringPrintOutput(BufferedSafeStringBase<char>* string) : mString(string) {}
    virtual ~StringPrintOutput();
    virtual void write(const char*, int);
protected:
    BufferedSafeStringBase<char>* mString;
};

class StringCutOffPrintOutput : public StringPrintOutput
{
public:
    StringCutOffPrintOutput(BufferedSafeStringBase<char>* string) : StringPrintOutput(string) {}
    virtual ~StringCutOffPrintOutput();
    virtual void write(const char*, int);
};

class StreamPrintOutput : public PrintOutput
{
public:
    StreamPrintOutput(StreamSrc* src) : mSrc(src) {}
    virtual ~StreamPrintOutput();
    virtual void write(const char*, int);
private:
    StreamSrc* mSrc;
};

class PrintFormatter
{
public:
    template <typename T, template <typename> class Wrapper>
    class OutImpl
    {
    public:
        static void out(const Wrapper<T>&, const char*, PrintOutput*);
        static void out__S(const Wrapper<T>&, const char*, PrintOutput*);
    };

    PrintFormatter(const char*, PrintOutput*);
    void flush();
    void flushWithLineBreak();
    PrintFormatter& operator<<(const char*);
    PrintFormatter& __ls(const char*);
    static void outputString_(const char*, PrintOutput*, const char*, int);
    static void outputPtr_(const char*, PrintOutput*, unsigned int);
    template <typename T> static void out(const T&, const char*, PrintOutput*);
    static void out(const char*, const char*, PrintOutput*);
    static void out(const wchar_t*, const char*, PrintOutput*);
    static void out(const Heap&, const char*, PrintOutput*);
    static void out(const ExpHeap&, const char*, PrintOutput*);
    static void out(const TickSpan&, const char*, PrintOutput*);
private:
    void proceedToFormatMark_(char*);
    const char* mFormat;
    PrintOutput* mOutput;
    int mPosition;
    int mLength;
    bool mPassThrough;
};

class BufferingPrintFormatter : public PrintFormatter
{
public:
    BufferingPrintFormatter();
    BufferingPrintFormatter(const char*);
};

class StringPrintFormatter : public PrintFormatter
{
public:
    StringPrintFormatter(BufferedSafeStringBase<char>*);
    StringPrintFormatter(BufferedSafeStringBase<char>*, const char*);
private:
    StringPrintOutput mStringOutput;
};

class StringCutOffPrintFormatter : public PrintFormatter
{
public:
    StringCutOffPrintFormatter(BufferedSafeStringBase<char>*);
    StringCutOffPrintFormatter(BufferedSafeStringBase<char>*, const char*);
private:
    StringCutOffPrintOutput mStringOutput;
};

class StreamPrintFormatter : public PrintFormatter
{
public:
    StreamPrintFormatter(StreamSrc*);
    StreamPrintFormatter(StreamSrc*, const char*);
    void flushAndWriteNullChar();
private:
    StreamPrintOutput mStreamOutput;
};

class PrintConfig
{
public:
    struct PrintEventArg
    {
        PrintEventArg(const char* value = 0, int printLevel = 0) : text(value), level(printLevel) {}
        const char* text;
        int level;
    };
    static void registerFinalCallback(IDelegate1<const PrintEventArg&>*);
    static void execCallbacks(const PrintEventArg&);
    static void registerCallback(DelegateEvent<const PrintEventArg&>::Slot&);
    static void unregisterCallback(DelegateEvent<const PrintEventArg&>::Slot&);
};

class StringUtil
{
public:
    enum CardinalNumber { cDecimal = 0, cHexadecimal = 1, cOctal = 2, cBinary = 3 };
    struct Char16Pair { wchar_t from; wchar_t to; };
    static bool tryParseU64(unsigned long long*, const SafeStringBase<char>&, CardinalNumber);
    static bool tryParseS64(long long*, const SafeStringBase<char>&, CardinalNumber);
    static bool tryParseU32(unsigned int*, const SafeStringBase<char>&, CardinalNumber);
    static bool tryParseS32(int*, const SafeStringBase<char>&, CardinalNumber);
    static bool tryParseU16(unsigned short*, const SafeStringBase<char>&, CardinalNumber);
    static bool tryParseS16(short*, const SafeStringBase<char>&, CardinalNumber);
    static bool tryParseU8(unsigned char*, const SafeStringBase<char>&, CardinalNumber);
    static bool tryParseS8(signed char*, const SafeStringBase<char>&, CardinalNumber);
    static int parseS32(const SafeStringBase<char>&, CardinalNumber);
    static int vsw16printf(wchar_t*, unsigned int, const wchar_t*, va_list);
    static int vsnprintf(char*, unsigned int, const char*, va_list);
    static int convertUtf16ToSjis(char*, unsigned int, const wchar_t*, int);
    static int convertUtf16ToUtf8(char*, unsigned int, const wchar_t*, int);
    static int convertUtf8ToSjis(char*, unsigned int, const char*, int);
    static int compareChar16Pair(const Char16Pair&, const wchar_t&);
    static wchar_t replace(wchar_t, const Buffer<const Char16Pair>&);
    static wchar_t toUpperCapital(wchar_t);
    static void toUpperCapitalFirstCharactor(BufferedSafeStringBase<wchar_t>*);
    static wchar_t toLowerCapital(wchar_t);
    static void toLowerCapitalFirstCharactor(BufferedSafeStringBase<wchar_t>*);
    static bool tryParseF32(float*, const SafeStringBase<char>&);
    static bool tryParseF64(double*, const SafeStringBase<char>&);
    static unsigned long long parseU64(const SafeStringBase<char>&, CardinalNumber);
    static long long parseS64(const SafeStringBase<char>&, CardinalNumber);
    static unsigned int parseU32(const SafeStringBase<char>&, CardinalNumber);
    static unsigned short parseU16(const SafeStringBase<char>&, CardinalNumber);
    static short parseS16(const SafeStringBase<char>&, CardinalNumber);
    static unsigned char parseU8(const SafeStringBase<char>&, CardinalNumber);
    static signed char parseS8(const SafeStringBase<char>&, CardinalNumber);
    static float parseF32(const SafeStringBase<char>&);
    static double parseF64(const SafeStringBase<char>&);
    static void wcs16cpy(wchar_t*, unsigned int, const wchar_t*);
    static int sw16printf(wchar_t*, unsigned int, const wchar_t*, ...);
    static int snprintf(char*, unsigned int, const char*, ...);
    static int convertSjisToUtf16(wchar_t*, unsigned int, const char*, int);
    static int convertUtf8ToUtf16(wchar_t*, unsigned int, const char*, int);
    static int convertSjisToUtf8(char*, unsigned int, const char*, int);

    class __N_18_peadStringUtil_cpp_6714fef4
    {
    public:
        struct OutBuffer16
        {
            wchar_t* buffer;
            int capacity;
            int length;
        };

        class FormatParser16
        {
        public:
            FormatParser16(va_list*);
            void clear_();
            const wchar_t* parseFlag_(const wchar_t*);
            const wchar_t* parseWidth_(const wchar_t*);
            const wchar_t* parsePrecision_(const wchar_t*);
            const wchar_t* parseSize_(const wchar_t*);
            void printInteger_(OutBuffer16*, unsigned int, bool, bool);
            void printCharacter_(OutBuffer16*);
            void printString_(OutBuffer16*);
            void printFloat_(OutBuffer16*, wchar_t, bool);
            int execute(OutBuffer16*, const wchar_t*);
            template <typename T> static void printIntegerInverseString_(OutBuffer16*, unsigned int, bool, T);
            template <typename T> void printIntegerInverseStringFromArg_(OutBuffer16*, bool*, bool, unsigned int, bool);
            template <typename T> void printStringCore_(OutBuffer16*, const T*);
        private:
            va_list* mArgs;
            bool mLeftJustify;
            bool mForceSign;
            bool mBlankSign;
            bool mAlternate;
            bool mZeroPad;
            int mWidth;
            int mPrecision;
            wchar_t mSize;
        };

        static int checkAndConvertDecChar_(char);
        static int checkAndConvertHexChar_(char);
        static void tryParseSignImpl_(int*, SafeStringBase<char>::iterator&);
        static bool tryParsePrefixImpl_(int*, SafeStringBase<char>::iterator&, const SafeStringBase<char>&);
        template <typename T> static bool tryParseNumberImpl_(T*, SafeStringBase<char>::iterator&, int);
        template <typename T> static bool tryParseDecImpl_(T*, SafeStringBase<char>::iterator&, int);
        template <typename T> static bool tryParseImpl_(T*, const SafeStringBase<char>&, int);
        template <typename T> static void tryParseUnderDecimalPointImpl_(T*, SafeStringBase<char>::iterator&, T, T);
        template <typename T> static bool tryParseDecimalFractionImpl_(T*, const SafeStringBase<char>&);
        template <typename T> static T parseImpl_(const SafeStringBase<char>&, CardinalNumber);
        template <typename T> static T parseDecimalFractionImpl_(const SafeStringBase<char>&);
    };
};

class __N_16_peadCoreInfo_cpp_b178e7d4
{
public:
    static int countNumBits(unsigned int);
};

class __N_24_peadTextStreamFormat_cpp_8d174ff4
{
public:
    static bool isSJISCode_(unsigned char);
};

namespace system
{
    void PrintStringImpl(const char*, int);
    void PrintString(const char*, int);
    void PrintV(const char*, va_list);
    void Print(const char*, ...);
}

int replaceStringImpl(char*, int*, int, const char*, int,
                      const SafeStringBase<char>&, const SafeStringBase<char>&, bool*);
int replaceStringImpl(wchar_t*, int*, int, const wchar_t*, int,
                      const SafeStringBase<wchar_t>&, const SafeStringBase<wchar_t>&, bool*);
template <typename T>
int replaceStringImpl_(T*, int*, int, const T*, int,
                       const SafeStringBase<T>&, const SafeStringBase<T>&, bool*);
}

#endif
