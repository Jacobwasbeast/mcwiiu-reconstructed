#include "platform/wiiu/pead.hpp"

#include <algorithm>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <new>
#include <vector>

#include <coreinit/time.h>
#include <coreinit/core.h>
#include <coreinit/mutex.h>
#include <coreinit/thread.h>
#include <coreinit/event.h>

class C4JThreadImpl
{
public:
    static unsigned int OSCreateThread4J(OSThread*, int (*)(int, void*), int, void*, void*, unsigned int, unsigned int, unsigned short);
};


namespace pead
{
namespace
{
BinaryStreamFormat sBinaryFormat;
TextStreamFormat sTextFormat;
HeapMgr* sHeapMgr;
ThreadMgr* sThreadMgr;
IDelegate1<const PrintConfig::PrintEventArg&>* sFinalPrintCallback;
IDelegate1<const HeapMgr::AllocFailedCallbackArg*>* sAllocFailedCallback;
std::vector<Heap*> sHeaps;

template <typename T>
T byteSwap(T value)
{
    union Bytes
    {
        T value;
        unsigned char data[sizeof(T)];
    } source, destination;
    source.value = value;
    for (unsigned int index = 0; index < sizeof(T); ++index)
        destination.data[index] = source.data[sizeof(T) - index - 1];
    return destination.value;
}

template <typename T>
T convertEndian(T value, Endian::Types endian)
{
    return endian == Endian::cLittle ? byteSwap(value) : value;
}

template <typename T>
bool parseInteger(T* value, const SafeStringBase<char>& text, StringUtil::CardinalNumber cardinal)
{
    if (!value) return false;
    char* end = 0;
    int base = cardinal == StringUtil::cHexadecimal ? 16 :
               cardinal == StringUtil::cOctal ? 8 :
               cardinal == StringUtil::cBinary ? 2 : 10;
    unsigned long long parsed = strtoull(text.cstr(), &end, base);
    if (!end || *end) return false;
    *value = static_cast<T>(parsed);
    return true;
}

template <typename T>
bool parseSignedInteger(T* value, const SafeStringBase<char>& text, StringUtil::CardinalNumber cardinal)
{
    if (!value) return false;
    char* end = 0;
    int base = cardinal == StringUtil::cHexadecimal ? 16 :
               cardinal == StringUtil::cOctal ? 8 :
               cardinal == StringUtil::cBinary ? 2 : 10;
    long long parsed = strtoll(text.cstr(), &end, base);
    if (!end || *end) return false;
    *value = static_cast<T>(parsed);
    return true;
}

int appendText(char* destination, unsigned int capacity, const char* source, int length)
{
    if (!destination || capacity == 0) return 0;
    unsigned int current = static_cast<unsigned int>(strlen(destination));
    unsigned int requested = length < 0 ? static_cast<unsigned int>(strlen(source)) : static_cast<unsigned int>(length);
    unsigned int available = current < capacity ? capacity - current - 1 : 0;
    unsigned int copied = requested < available ? requested : available;
    if (copied) memcpy(destination + current, source, copied);
    destination[current + copied] = 0;
    return static_cast<int>(copied);
}

void appendWide(StringUtil::__N_18_peadStringUtil_cpp_6714fef4::OutBuffer16* output, wchar_t character)
{
    if (output->length + 1 < output->capacity) output->buffer[output->length] = character;
    ++output->length;
}
}

template <typename T>
void BufferedSafeStringBase<T>::copy(const T* text)
{
    if (!mBuffer || mBufferSize <= 0) return;
    int index = 0;
    if (text)
        while (index < mBufferSize - 1 && text[index])
        {
            mBuffer[index] = text[index];
            ++index;
        }
    mBuffer[index] = 0;
}

template <>
int BufferedSafeStringBase<char>::formatImpl_(char* buffer, int size, const char* format, va_list args)
{
    if (!buffer || size <= 0) return 0;
    int result = ::vsnprintf(buffer, static_cast<size_t>(size), format, args);
    buffer[size - 1] = 0;
    return result;
}

template <>
int BufferedSafeStringBase<wchar_t>::formatImpl_(wchar_t* buffer, int size, const wchar_t* format, va_list args)
{
    if (!buffer || size <= 0) return 0;
    int result = ::vswprintf(buffer, static_cast<size_t>(size), format, args);
    buffer[size - 1] = 0;
    return result;
}

template <typename T>
int BufferedSafeStringBase<T>::format(const T* formatText, ...)
{
    va_list args;
    va_start(args, formatText);
    int result = formatV(formatText, args);
    va_end(args);
    return result;
}

template <typename T>
int BufferedSafeStringBase<T>::appendWithFormatV(const T* formatText, va_list args)
{
    int length = this->calcLength();
    if (length >= mBufferSize) return 0;
    return formatImpl_(mBuffer + length, mBufferSize - length, formatText, args);
}

template <typename T>
int BufferedSafeStringBase<T>::appendWithFormat(const T* formatText, ...)
{
    va_list args;
    va_start(args, formatText);
    int result = appendWithFormatV(formatText, args);
    va_end(args);
    return result;
}

template <typename T>
HeapSafeStringBase<T>::~HeapSafeStringBase()
{
    if (this->mBuffer) freeToSeadHeap(this->mBuffer);
}

template <>
SafeStringBase<char>& SafeStringBase<char>::__as__S(const SafeStringBase<char>& other)
{
    return operator=(other);
}

template <>
SafeStringBase<wchar_t>& SafeStringBase<wchar_t>::__as__S(const SafeStringBase<wchar_t>& other)
{
    return operator=(other);
}

template <>
BufferedSafeStringBase<char>& BufferedSafeStringBase<char>::__as__S(const SafeStringBase<char>& other)
{
    return operator=(other);
}

template <>
BufferedSafeStringBase<wchar_t>& BufferedSafeStringBase<wchar_t>::__as__S(const SafeStringBase<wchar_t>& other)
{
    return operator=(other);
}

template <>
int BufferedSafeStringBase<char>::formatImpl___S(char* buffer, int size, const char* formatText, va_list args)
{
    return formatImpl_(buffer, size, formatText, args);
}

template <>
int BufferedSafeStringBase<wchar_t>::formatImpl___S(wchar_t* buffer, int size, const wchar_t* formatText, va_list args)
{
    return formatImpl_(buffer, size, formatText, args);
}

template <>
int BufferedSafeStringBase<char>::formatV__S(const char* formatText, va_list args)
{
    return formatV(formatText, args);
}

template <>
int BufferedSafeStringBase<char>::format__S(const char* formatText, ...)
{
    va_list args;
    va_start(args, formatText);
    int result = formatV__S(formatText, args);
    va_end(args);
    return result;
}

template <>
int BufferedSafeStringBase<wchar_t>::formatV__S(const wchar_t* formatText, va_list args)
{
    return formatV(formatText, args);
}

template <>
int BufferedSafeStringBase<wchar_t>::format__S(const wchar_t* formatText, ...)
{
    va_list args;
    va_start(args, formatText);
    int result = formatV__S(formatText, args);
    va_end(args);
    return result;
}

template <>
int BufferedSafeStringBase<char>::appendWithFormatV__S(const char* formatText, va_list args)
{
    return appendWithFormatV(formatText, args);
}

template <>
int BufferedSafeStringBase<wchar_t>::appendWithFormatV__S(const wchar_t* formatText, va_list args)
{
    return appendWithFormatV(formatText, args);
}

template <>
int BufferedSafeStringBase<char>::appendWithFormat__S(const char* formatText, ...)
{
    va_list args;
    va_start(args, formatText);
    int result = appendWithFormatV__S(formatText, args);
    va_end(args);
    return result;
}

template <>
int BufferedSafeStringBase<wchar_t>::appendWithFormat__S(const wchar_t* formatText, ...)
{
    va_list args;
    va_start(args, formatText);
    int result = appendWithFormatV__S(formatText, args);
    va_end(args);
    return result;
}

template <>
void BufferedSafeStringBase<char>::assureTerminationImpl___S() const
{
    assureTerminationImpl_();
}

template <>
void BufferedSafeStringBase<wchar_t>::assureTerminationImpl___S() const
{
    assureTerminationImpl_();
}

template <>
HeapSafeStringBase<char>& HeapSafeStringBase<char>::__as__S(const SafeStringBase<char>& other)
{
    return operator=(other);
}

template <>
HeapSafeStringBase<wchar_t>& HeapSafeStringBase<wchar_t>::__as__S(const SafeStringBase<wchar_t>& other)
{
    return operator=(other);
}

template class SafeStringBase<char>;
template class SafeStringBase<wchar_t>;
template class BufferedSafeStringBase<char>;
template class BufferedSafeStringBase<wchar_t>;
template class HeapSafeStringBase<char>;
template class HeapSafeStringBase<wchar_t>;

void ListNode::insertBack_(ListNode* node)
{
    node->mPrev = mPrev;
    node->mNext = this;
    mPrev->mNext = node;
    mPrev = node;
}

void ListNode::insertFront_(ListNode* node)
{
    node->mPrev = this;
    node->mNext = mNext;
    mNext->mPrev = node;
    mNext = node;
}

void ListNode::erase_()
{
    mPrev->mNext = mNext;
    mNext->mPrev = mPrev;
    mPrev = this;
    mNext = this;
}

ListNode* ListImpl::nth(int index) const
{
    if (index < 0 || index >= mCount) return 0;
    ListNode* node = mStart.next();
    while (index--) node = node->next();
    return node;
}

void ListImpl::sort(int offset, int (*compare)(const void*, const void*))
{
    if (mCount <= 1)
        return;

    ListNode* node = nth(1);
    while (node != &mStart)
    {
        ListNode* next = node->next();
        if (compare(reinterpret_cast<const char*>(node->prev()) - offset,
                    reinterpret_cast<const char*>(node) - offset) > 0)
        {
            ListNode* insertion = node->prev();
            while (insertion != &mStart &&
                   compare(reinterpret_cast<const char*>(insertion) - offset,
                           reinterpret_cast<const char*>(node) - offset) > 0)
                insertion = insertion->prev();
            node->erase_();
            insertion->insertBack_(node);
        }
        node = next;
    }
}

void ListImpl::clear()
{
    while (mStart.next() != &mStart) mStart.next()->erase_();
    mCount = 0;
}

void PtrArrayImpl::setBuffer(int capacity, void* buffer)
{
    mBuffer = static_cast<void**>(buffer);
    mSize = 0;
    mCapacity = capacity;
    mAllocated = false;
}

bool PtrArrayImpl::tryAllocBuffer(int capacity, Heap* heap, int alignment)
{
    freeBuffer();
    mBuffer = static_cast<void**>(allocateFromSeadHeap(sizeof(void*) * capacity, heap, alignment));
    if (!mBuffer) return false;
    mSize = 0;
    mCapacity = capacity;
    mAllocated = true;
    return true;
}

void PtrArrayImpl::allocBuffer(int capacity, Heap* heap, int alignment)
{
    tryAllocBuffer(capacity, heap, alignment);
}

void PtrArrayImpl::freeBuffer()
{
    if (mAllocated && mBuffer) freeToSeadHeap(mBuffer);
    mBuffer = 0;
    mSize = 0;
    mCapacity = 0;
    mAllocated = false;
}

void PtrArrayImpl::erase(int index, int count)
{
    if (index < 0 || count <= 0 || index >= mSize) return;
    if (index + count > mSize) count = mSize - index;
    memmove(mBuffer + index, mBuffer + index + count, sizeof(void*) * (mSize - index - count));
    mSize -= count;
}

bool PtrArrayImpl::checkInsert(int index, int count)
{
    return index >= 0 && index <= mSize && count >= 0 && mSize + count <= mCapacity;
}

void PtrArrayImpl::insert(int index, void* value)
{
    if (!checkInsert(index, 1)) return;
    memmove(mBuffer + index + 1, mBuffer + index, sizeof(void*) * (mSize - index));
    mBuffer[index] = value;
    ++mSize;
}

int PtrArrayImpl::binarySearch(const void* value, int (*compare)(const void*, const void*)) const
{
    int low = 0;
    int high = mSize;
    while (low < high)
    {
        int middle = (low + high) / 2;
        int result = compare(value, mBuffer[middle]);
        if (result == 0) return middle;
        if (result < 0) high = middle;
        else low = middle + 1;
    }
    return -low - 1;
}

void pead::Random::init(unsigned int seed)
{
    mContext[0] = (seed ^ (seed >> 30)) * 0x6c078965u + 1;
    mContext[1] = (mContext[0] ^ (mContext[0] >> 30)) * 0x6c078965u + 2;
    mContext[2] = (mContext[1] ^ (mContext[1] >> 30)) * 0x6c078965u + 3;
    mContext[3] = (mContext[2] ^ (mContext[2] >> 30)) * 0x6c078965u + 4;
}

void pead::Random::init()
{
    init(static_cast<unsigned int>(OSGetSystemTime()));
}

unsigned int Random::getU32()
{
    unsigned int temporary = mContext[0] ^ (mContext[0] << 11);
    mContext[0] = mContext[1];
    mContext[1] = mContext[2];
    mContext[2] = mContext[3];
    mContext[3] = mContext[3] ^ (mContext[3] >> 19) ^ temporary ^ (temporary >> 8);
    return mContext[3];
}

unsigned long long Random::getU64()
{
    return static_cast<unsigned long long>(getU32()) << 32 | getU32();
}

BufferReadStreamSrc::~BufferReadStreamSrc() {}

int BufferReadStreamSrc::skip(int amount)
{
    int next = static_cast<int>(mPosition) + amount;
    if (next < 0) next = 0;
    if (static_cast<unsigned int>(next) > mSize) next = static_cast<int>(mSize);
    mPosition = static_cast<unsigned int>(next);
    return next;
}

unsigned int BufferReadStreamSrc::read(void* destination, unsigned int size)
{
    unsigned int remaining = mSize - mPosition;
    unsigned int readSize = size < remaining ? size : remaining;
    if (readSize) memcpy(destination, mBuffer + mPosition, readSize);
    mPosition += readSize;
    return readSize;
}

unsigned int BufferReadStreamSrc::write(const void*, unsigned int) { return 0; }
void BufferReadStreamSrc::rewind() { mPosition = 0; }
bool BufferReadStreamSrc::isEOF() { return mPosition >= mSize; }

BufferWriteStreamSrc::~BufferWriteStreamSrc() {}
int BufferWriteStreamSrc::skip(int) { return 0; }
unsigned int BufferWriteStreamSrc::read(void*, unsigned int) { return 0; }

unsigned int BufferWriteStreamSrc::write(const void* source, unsigned int size)
{
    unsigned int remaining = mBufferSize - mPosition;
    unsigned int writeSize = size < remaining ? size : remaining;
    if (writeSize) memcpy(mBuffer + mPosition, source, writeSize);
    mPosition += writeSize;
    return writeSize;
}

void BufferWriteStreamSrc::rewind() { mPosition = 0; }
bool BufferWriteStreamSrc::isEOF() { return mPosition >= mBufferSize; }

bool BufferWriteStreamSrc::flush()
{
    if (!mOriginalSrc) return true;
    unsigned int written = mOriginalSrc->write(mBuffer, mPosition);
    if (written != mPosition) return false;
    mPosition = 0;
    return mOriginalSrc->flush();
}

BufferMultiByteTextWriteStreamSrc::~BufferMultiByteTextWriteStreamSrc() {}

unsigned int BufferMultiByteTextWriteStreamSrc::write(const void* source, unsigned int size)
{
    const unsigned char* bytes = static_cast<const unsigned char*>(source);
    unsigned int total = 0;
    while (total < size)
    {
        if (mPosition == mBufferSize && !flush()) break;
        unsigned int remaining = mBufferSize - mPosition;
        unsigned int part = size - total < remaining ? size - total : remaining;
        memcpy(mBuffer + mPosition, bytes + total, part);
        mPosition += part;
        total += part;
    }
    return total;
}

BufferMultiByteNullTerminatedTextWriteStreamSrc::~BufferMultiByteNullTerminatedTextWriteStreamSrc() {}

bool BufferMultiByteNullTerminatedTextWriteStreamSrc::flush()
{
    mBuffer[mPosition] = 0;
    return BufferWriteStreamSrc::flush();
}

template <typename T>
T readBinary(StreamSrc* source, Endian::Types endian)
{
    T value = 0;
    source->read(&value, sizeof(value));
    return convertEndian(value, endian);
}

template <typename T>
void writeBinary(StreamSrc* source, Endian::Types endian, T value)
{
    value = convertEndian(value, endian);
    source->write(&value, sizeof(value));
}

unsigned char BinaryStreamFormat::readU8(StreamSrc* src, Endian::Types e) { return readBinary<unsigned char>(src, e); }
unsigned short BinaryStreamFormat::readU16(StreamSrc* src, Endian::Types e) { return readBinary<unsigned short>(src, e); }
unsigned int BinaryStreamFormat::readU32(StreamSrc* src, Endian::Types e) { return readBinary<unsigned int>(src, e); }
unsigned long long BinaryStreamFormat::readU64(StreamSrc* src, Endian::Types e) { return readBinary<unsigned long long>(src, e); }
signed char BinaryStreamFormat::readS8(StreamSrc* src, Endian::Types e) { return readBinary<signed char>(src, e); }
short BinaryStreamFormat::readS16(StreamSrc* src, Endian::Types e) { return readBinary<short>(src, e); }
int BinaryStreamFormat::readS32(StreamSrc* src, Endian::Types e) { return readBinary<int>(src, e); }
long long BinaryStreamFormat::readS64(StreamSrc* src, Endian::Types e) { return readBinary<long long>(src, e); }
float BinaryStreamFormat::readF32(StreamSrc* src, Endian::Types e) { return readBinary<float>(src, e); }

void BinaryStreamFormat::readBit(StreamSrc* src, void* data, unsigned int bits)
{
    unsigned int bytes = bits / 8;
    src->read(data, bytes);
    unsigned int remaining = bits & 7;
    if (remaining)
    {
        unsigned char tail = 0;
        src->read(&tail, 1);
        unsigned char mask = static_cast<unsigned char>(0xffu << remaining);
        static_cast<unsigned char*>(data)[bytes] = static_cast<unsigned char>((static_cast<unsigned char*>(data)[bytes] & mask) | (tail & ~mask));
    }
}

void BinaryStreamFormat::readString(StreamSrc* src, BufferedSafeStringBase<char>* string, unsigned int size)
{
    unsigned int length = readU32(src, Endian::cHost);
    unsigned int capacity = static_cast<unsigned int>(string->getBufferSize());
    unsigned int copySize = length < size ? length : size;
    if (capacity) copySize = copySize < capacity - 1 ? copySize : capacity - 1;
    src->read(string->getBuffer(), copySize);
    if (capacity) string->getBuffer()[copySize] = 0;
    if (length > copySize) src->skip(static_cast<int>(length - copySize));
}

void BinaryStreamFormat::readMemBlock(StreamSrc* src, void* data, unsigned int size) { src->read(data, size); }
void BinaryStreamFormat::writeU8(StreamSrc* src, Endian::Types e, unsigned char v) { writeBinary(src, e, v); }
void BinaryStreamFormat::writeU16(StreamSrc* src, Endian::Types e, unsigned short v) { writeBinary(src, e, v); }
void BinaryStreamFormat::writeU32(StreamSrc* src, Endian::Types e, unsigned int v) { writeBinary(src, e, v); }
void BinaryStreamFormat::writeU64(StreamSrc* src, Endian::Types e, unsigned long long v) { writeBinary(src, e, v); }
void BinaryStreamFormat::writeS8(StreamSrc* src, Endian::Types e, signed char v) { writeBinary(src, e, v); }
void BinaryStreamFormat::writeS16(StreamSrc* src, Endian::Types e, short v) { writeBinary(src, e, v); }
void BinaryStreamFormat::writeS32(StreamSrc* src, Endian::Types e, int v) { writeBinary(src, e, v); }
void BinaryStreamFormat::writeS64(StreamSrc* src, Endian::Types e, long long v) { writeBinary(src, e, v); }
void BinaryStreamFormat::writeF32(StreamSrc* src, Endian::Types e, float v) { writeBinary(src, e, v); }

void BinaryStreamFormat::writeBit(StreamSrc* src, const void* data, unsigned int bits)
{
    unsigned int bytes = (bits + 7) / 8;
    src->write(data, bytes);
}

void BinaryStreamFormat::writeString(StreamSrc* src, const SafeStringBase<char>& string, unsigned int size)
{
    unsigned int length = static_cast<unsigned int>(string.calcLength());
    if (length > size) length = size;
    writeU32(src, Endian::cHost, length);
    src->write(string.cstr(), length);
}

void BinaryStreamFormat::writeMemBlock(StreamSrc* src, const void* data, unsigned int size) { src->write(data, size); }
void BinaryStreamFormat::writeDecorationText(StreamSrc*, const SafeStringBase<char>&) {}
void BinaryStreamFormat::writeNullChar(StreamSrc*) {}
void BinaryStreamFormat::skip(StreamSrc* src, unsigned int size) { src->skip(static_cast<int>(size)); }
void BinaryStreamFormat::flush(StreamSrc*) {}
void BinaryStreamFormat::rewind(StreamSrc* src) { src->rewind(); }

TextStreamFormat::~TextStreamFormat() {}
TextStreamFormat::TextStreamFormat() : mTextBuffer() {}
void TextStreamFormat::getNextData_(StreamSrc* source)
{
    unsigned int length = 0;
    bool quoted = false;
    bool blockComment = false;
    char character = 0;
    while (source->read(&character, 1) == 1)
    {
        if (blockComment)
        {
            if (character == '*')
            {
                char next = 0;
                if (source->read(&next, 1) == 1 && next == '/') blockComment = false;
            }
            continue;
        }
        if (!quoted && (character == ' ' || character == '\t' || character == '\r' || character == '\n'))
        {
            if (length) break;
            continue;
        }
        if (!quoted && character == '#')
        {
            while (source->read(&character, 1) == 1 && character != '\n') {}
            if (length) break;
            continue;
        }
        if (!quoted && character == '/')
        {
            char next = 0;
            if (source->read(&next, 1) == 1)
            {
                if (next == '/')
                {
                    while (source->read(&character, 1) == 1 && character != '\n') {}
                    if (length) break;
                    continue;
                }
                if (next == '*')
                {
                    blockComment = true;
                    continue;
                }
                if (length < static_cast<unsigned int>(mTextBuffer.getBufferSize() - 1)) mTextBuffer.getBuffer()[length++] = character;
                character = next;
            }
        }
        if (character == '"')
        {
            quoted = !quoted;
            if (!quoted) break;
            continue;
        }
        if (length < static_cast<unsigned int>(mTextBuffer.getBufferSize() - 1)) mTextBuffer.getBuffer()[length++] = character;
        if (__N_24_peadTextStreamFormat_cpp_8d174ff4::isSJISCode_(static_cast<unsigned char>(character)))
        {
            if (source->read(&character, 1) == 1 && length < static_cast<unsigned int>(mTextBuffer.getBufferSize() - 1))
                mTextBuffer.getBuffer()[length++] = character;
        }
    }
    mTextBuffer.getBuffer()[length] = 0;
}
unsigned char TextStreamFormat::readU8(StreamSrc* s, Endian::Types) { return static_cast<unsigned char>(readU32(s, Endian::cHost)); }
unsigned short TextStreamFormat::readU16(StreamSrc* s, Endian::Types) { return static_cast<unsigned short>(readU32(s, Endian::cHost)); }
unsigned int TextStreamFormat::readU32(StreamSrc* s, Endian::Types) { getNextData_(s); return static_cast<unsigned int>(strtoul(mTextBuffer.cstr(), 0, 0)); }
unsigned long long TextStreamFormat::readU64(StreamSrc* s, Endian::Types) { getNextData_(s); return strtoull(mTextBuffer.cstr(), 0, 0); }
signed char TextStreamFormat::readS8(StreamSrc* s, Endian::Types) { return static_cast<signed char>(readS32(s, Endian::cHost)); }
short TextStreamFormat::readS16(StreamSrc* s, Endian::Types) { return static_cast<short>(readS32(s, Endian::cHost)); }
int TextStreamFormat::readS32(StreamSrc* s, Endian::Types) { getNextData_(s); return static_cast<int>(strtol(mTextBuffer.cstr(), 0, 0)); }
long long TextStreamFormat::readS64(StreamSrc* s, Endian::Types) { getNextData_(s); return strtoll(mTextBuffer.cstr(), 0, 0); }
float TextStreamFormat::readF32(StreamSrc* s, Endian::Types) { getNextData_(s); return static_cast<float>(strtod(mTextBuffer.cstr(), 0)); }
void TextStreamFormat::readBit(StreamSrc* source, void* destination, unsigned int bitCount)
{
    getNextData_(source);
    unsigned char* bytes = static_cast<unsigned char*>(destination);
    memset(bytes, 0, (bitCount + 7) / 8);
    for (unsigned int bit = 0; bit < bitCount && mTextBuffer.cstr()[bit]; ++bit)
        if (mTextBuffer.cstr()[bit] == '1') bytes[bit / 8] |= static_cast<unsigned char>(1u << (bit & 7));
}
void TextStreamFormat::readString(StreamSrc* source, BufferedSafeStringBase<char>* destination, unsigned int size)
{
    getNextData_(source);
    int length = mTextBuffer.calcLength();
    if (length > static_cast<int>(size)) mTextBuffer.getBuffer()[size] = 0;
    destination->copy(mTextBuffer.cstr());
}
void TextStreamFormat::readMemBlock(StreamSrc* source, void* destination, unsigned int size)
{
    getNextData_(source);
    unsigned int decodedSize = 0;
    Base64::decode(destination, size, mTextBuffer.cstr(), mTextBuffer.calcLength(), &decodedSize);
}

void TextStreamFormat::writeU8(StreamSrc* s, Endian::Types, unsigned char v) { char b[16]; ::snprintf(b, sizeof(b), "%u", v); s->write(b, strlen(b)); }
void TextStreamFormat::writeU16(StreamSrc* s, Endian::Types, unsigned short v) { char b[16]; ::snprintf(b, sizeof(b), "%u", v); s->write(b, strlen(b)); }
void TextStreamFormat::writeU32(StreamSrc* s, Endian::Types, unsigned int v) { char b[16]; ::snprintf(b, sizeof(b), "%u", v); s->write(b, strlen(b)); }
void TextStreamFormat::writeU64(StreamSrc* s, Endian::Types, unsigned long long v) { char b[32]; ::snprintf(b, sizeof(b), "%llu", v); s->write(b, strlen(b)); }
void TextStreamFormat::writeS8(StreamSrc* s, Endian::Types, signed char v) { char b[16]; ::snprintf(b, sizeof(b), "%d", v); s->write(b, strlen(b)); }
void TextStreamFormat::writeS16(StreamSrc* s, Endian::Types, short v) { char b[16]; ::snprintf(b, sizeof(b), "%d", v); s->write(b, strlen(b)); }
void TextStreamFormat::writeS32(StreamSrc* s, Endian::Types, int v) { char b[16]; ::snprintf(b, sizeof(b), "%d", v); s->write(b, strlen(b)); }
void TextStreamFormat::writeS64(StreamSrc* s, Endian::Types, long long v) { char b[32]; ::snprintf(b, sizeof(b), "%lld", v); s->write(b, strlen(b)); }
void TextStreamFormat::writeF32(StreamSrc* s, Endian::Types, float v) { char b[32]; ::snprintf(b, sizeof(b), "%g", v); s->write(b, strlen(b)); }
void TextStreamFormat::writeBit(StreamSrc* source, const void* data, unsigned int bitCount)
{
    const unsigned char* bytes = static_cast<const unsigned char*>(data);
    for (unsigned int bit = 0; bit < bitCount; ++bit)
    {
        char value = bytes[bit / 8] & (1u << (bit & 7)) ? '1' : '0';
        source->write(&value, 1);
    }
}
void TextStreamFormat::writeString(StreamSrc* s, const SafeStringBase<char>& v, unsigned int n) { unsigned int l = v.calcLength(); if (l > n) l = n; s->write(v.cstr(), l); }
void TextStreamFormat::writeMemBlock(StreamSrc* s, const void* d, unsigned int n) { Base64::encode(mTextBuffer.getBuffer(), d, n, false); s->write(mTextBuffer.cstr(), ((n + 2) / 3) * 4); }
void TextStreamFormat::writeDecorationText(StreamSrc* s, const SafeStringBase<char>& v) { s->write(v.cstr(), v.calcLength()); }
void TextStreamFormat::writeNullChar(StreamSrc* s) { static const char zero = 0; s->write(&zero, 1); }
void TextStreamFormat::skip(StreamSrc* s, unsigned int n) { s->skip(n); }
void TextStreamFormat::rewind(StreamSrc* s) { s->rewind(); }
void TextStreamFormat::flush(StreamSrc* s) { s->flush(); }

Stream::~Stream() {}
void Stream::skip(unsigned int size) { mFormat->skip(mSrc, size); }
void Stream::skip(unsigned int count, unsigned int size) { mFormat->skip(mSrc, count * size); }
void Stream::rewind() { mFormat->rewind(mSrc); }
bool Stream::isEOF() { return mSrc->isEOF(); }
ReadStream::~ReadStream() {}
unsigned char pead::ReadStream::readU8() { return mFormat->readU8(mSrc, mEndian); }
unsigned short pead::ReadStream::readU16() { return mFormat->readU16(mSrc, mEndian); }
unsigned int pead::ReadStream::readU32() { return mFormat->readU32(mSrc, mEndian); }
unsigned long long pead::ReadStream::readU64() { return mFormat->readU64(mSrc, mEndian); }
signed char pead::ReadStream::readS8() { return mFormat->readS8(mSrc, mEndian); }
short pead::ReadStream::readS16() { return mFormat->readS16(mSrc, mEndian); }
int pead::ReadStream::readS32() { return mFormat->readS32(mSrc, mEndian); }
long long pead::ReadStream::readS64() { return mFormat->readS64(mSrc, mEndian); }
float pead::ReadStream::readF32() { return mFormat->readF32(mSrc, mEndian); }
void ReadStream::readBit(void* d, unsigned int n) { mFormat->readBit(mSrc, d, n); }
void ReadStream::readString(BufferedSafeStringBase<char>* s, unsigned int n) { mFormat->readString(mSrc, s, n); }
void ReadStream::readMemBlock(void* d, unsigned int n) { mFormat->readMemBlock(mSrc, d, n); }

WriteStream::~WriteStream() {}
void WriteStream::flush() { mFormat->flush(mSrc); mSrc->flush(); }
void WriteStream::writeU8(unsigned char v) { mFormat->writeU8(mSrc, mEndian, v); }
void WriteStream::writeU16(unsigned short v) { mFormat->writeU16(mSrc, mEndian, v); }
void WriteStream::writeU32(unsigned int v) { mFormat->writeU32(mSrc, mEndian, v); }
void WriteStream::writeU64(unsigned long long v) { mFormat->writeU64(mSrc, mEndian, v); }
void WriteStream::writeS8(signed char v) { mFormat->writeS8(mSrc, mEndian, v); }
void WriteStream::writeS16(short v) { mFormat->writeS16(mSrc, mEndian, v); }
void WriteStream::writeS32(int v) { mFormat->writeS32(mSrc, mEndian, v); }
void WriteStream::writeS64(long long v) { mFormat->writeS64(mSrc, mEndian, v); }
void WriteStream::writeF32(float v) { mFormat->writeF32(mSrc, mEndian, v); }
void WriteStream::writeBit(const void* d, unsigned int n) { mFormat->writeBit(mSrc, d, n); }
void WriteStream::writeString(const SafeStringBase<char>& s, unsigned int n) { mFormat->writeString(mSrc, s, n); }
void WriteStream::writeMemBlock(const void* d, unsigned int n) { mFormat->writeMemBlock(mSrc, d, n); }
void WriteStream::writeComment(const SafeStringBase<char>& s) { mFormat->writeDecorationText(mSrc, SafeStringBase<char>("/* ")); mFormat->writeDecorationText(mSrc, s); mFormat->writeDecorationText(mSrc, SafeStringBase<char>(" */")); }
void WriteStream::writeLineComment(const SafeStringBase<char>& s) { mFormat->writeDecorationText(mSrc, SafeStringBase<char>("// ")); mFormat->writeDecorationText(mSrc, s); }
void WriteStream::writeDecorationText(const SafeStringBase<char>& s) { mFormat->writeDecorationText(mSrc, s); }
void WriteStream::writeNullChar() { mFormat->writeNullChar(mSrc); }

BufferReadStream::~BufferReadStream() {}
BufferWriteStream::~BufferWriteStream() { mBufferSrc.flush(); }
BufferMultiByteTextWriteStream::~BufferMultiByteTextWriteStream() { mBufferSrc.flush(); }

unsigned int PrintStreamSrc::read(void*, unsigned int) { return 0; }
unsigned int PrintStreamSrc::write(const void*, unsigned int size) { return size; }
int PrintStreamSrc::skip(int) { return 0; }
void PrintStreamSrc::rewind() {}
bool PrintStreamSrc::isEOF() { return false; }
PrintWriteStream::~PrintWriteStream() {}

void Base64::encode(char* destination, const void* source, unsigned int size, bool urlSafe)
{
    const char* alphabet = urlSafe ? "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_" : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const unsigned char* input = static_cast<const unsigned char*>(source);
    unsigned int inputIndex = 0;
    unsigned int outputIndex = 0;
    while (inputIndex + 2 < size)
    {
        unsigned int value = input[inputIndex] << 16 | input[inputIndex + 1] << 8 | input[inputIndex + 2];
        destination[outputIndex++] = alphabet[(value >> 18) & 63];
        destination[outputIndex++] = alphabet[(value >> 12) & 63];
        destination[outputIndex++] = alphabet[(value >> 6) & 63];
        destination[outputIndex++] = alphabet[value & 63];
        inputIndex += 3;
    }
    if (inputIndex < size)
    {
        unsigned int value = input[inputIndex] << 16;
        if (inputIndex + 1 < size) value |= input[inputIndex + 1] << 8;
        destination[outputIndex++] = alphabet[(value >> 18) & 63];
        destination[outputIndex++] = alphabet[(value >> 12) & 63];
        destination[outputIndex++] = inputIndex + 1 < size ? alphabet[(value >> 6) & 63] : '=';
        destination[outputIndex++] = '=';
    }
}

bool Base64::decode(void* destination, unsigned int capacity, const char* source, unsigned int size, unsigned int* resultSize)
{
    unsigned char* output = static_cast<unsigned char*>(destination);
    unsigned int written = 0;
    unsigned int accumulator = 0;
    int bits = 0;
    for (unsigned int index = 0; index < size; ++index)
    {
        unsigned char character = static_cast<unsigned char>(source[index]);
        if (character <= 0x20) continue;
        if (character == '=') break;
        int value = character >= 'A' && character <= 'Z' ? character - 'A' :
                    character >= 'a' && character <= 'z' ? character - 'a' + 26 :
                    character >= '0' && character <= '9' ? character - '0' + 52 :
                    character == '+' || character == '-' ? 62 :
                    character == '/' || character == '_' ? 63 : -1;
        if (value < 0) return false;
        accumulator = accumulator << 6 | static_cast<unsigned int>(value);
        bits += 6;
        if (bits >= 8)
        {
            bits -= 8;
            if (written >= capacity) return false;
            output[written++] = static_cast<unsigned char>(accumulator >> bits);
        }
    }
    if (resultSize) *resultSize = written;
    return true;
}

unsigned int BitFlagUtil::countOnBit(unsigned int value)
{
    value = value - ((value >> 1) & 0x55555555u);
    value = (value & 0x33333333u) + ((value >> 2) & 0x33333333u);
    return (((value + (value >> 4)) & 0x0f0f0f0fu) * 0x01010101u) >> 24;
}

int BitFlagUtil::findOnBitFromRight(unsigned int value, int start)
{
    for (int bit = start; bit < 32; ++bit) if (value & (1u << bit)) return bit;
    return -1;
}

CoreId::CoreId() : mId(OSGetCoreId()) {}
static unsigned int sCoreCount;
static unsigned int sCoreIdMap[3];
static unsigned int sCoreIndexTable[3];

void CoreInfo::configure()
{
    sCoreCount = 3;
    sCoreIdMap[0] = 1;
    sCoreIdMap[1] = 0;
    sCoreIdMap[2] = 2;
    sCoreIndexTable[sCoreIdMap[0]] = 0;
    sCoreIndexTable[sCoreIdMap[1]] = 1;
    sCoreIndexTable[sCoreIdMap[2]] = 2;
}

int CoreInfo::getPlatformMask(CoreId id)
{
    return 1 << (sCoreIdMap[id.value()] & 0x3f);
}

bool __N_24_peadTextStreamFormat_cpp_8d174ff4::isSJISCode_(unsigned char)
{
    return false;
}

IDisposer::IDisposer() : mDisposerNode(), mHeap(HeapMgr::instance() ? HeapMgr::instance()->getCurrentHeap() : 0)
{
    if (mHeap) mHeap->appendDisposer_(this);
}

IDisposer::IDisposer(Heap* heap, HeapNullOption option) : mDisposerNode(), mHeap(heap)
{
    if (!mHeap && option == cHeapNullOption_Assert && HeapMgr::instance()) mHeap = HeapMgr::instance()->getCurrentHeap();
    if (mHeap) mHeap->appendDisposer_(this);
}

IDisposer::~IDisposer()
{
    if (mHeap) mHeap->removeDisposer_(this);
}

Heap::Heap(const SafeStringBase<char>& name, Heap* parent, void* start, unsigned int size, HeapDirection direction, bool enableLock)
    : IDisposer(parent, cHeapNullOption_Allow), mName(name), mParent(parent), mChildren(), mDisposers(), mStart(start), mSize(size), mDirection(direction), mEnableLock(enableLock)
{
    sHeaps.push_back(this);
    if (mParent) mParent->pushBackChild_(this);
}

Heap::~Heap() { for (std::vector<Heap*>::iterator it = sHeaps.begin(); it != sHeaps.end(); ++it) if (*it == this) { sHeaps.erase(it); break; } }
bool Heap::checkDerivedRuntimeTypeInfo(const RuntimeTypeInfo::Interface* type) const { return getRuntimeTypeInfo()->isDerived(type); }
const RuntimeTypeInfo::Interface* Heap::getRuntimeTypeInfo() const { static RuntimeTypeInfo::Derive<Heap> type; return &type; }
void* Heap::tryRealloc(void*, unsigned int, int) { return 0; }
void Heap::dump() const {}
void Heap::dumpYAML(WriteStream& stream, int indent) const
{
    for (int index = 0; index < indent; ++index) stream.writeDecorationText(SafeStringBase<char>("  "));
    stream.writeDecorationText(SafeStringBase<char>("name: "));
    stream.writeDecorationText(mName);
    stream.writeDecorationText(SafeStringBase<char>("\n"));
    for (int index = 0; index < indent; ++index) stream.writeDecorationText(SafeStringBase<char>("  "));
    stream.writeDecorationText(SafeStringBase<char>("size: "));
    stream.writeU32(mSize);
    stream.writeDecorationText(SafeStringBase<char>("\n"));
}
void Heap::pushBackChild_(Heap* child) { if (child) child->mParent = this; }
void Heap::dumpTreeYAML(WriteStream& stream, int indent) const
{
    dumpYAML(stream, indent);
    for (std::vector<Heap*>::iterator it = sHeaps.begin(); it != sHeaps.end(); ++it)
        if ((*it)->mParent == this) (*it)->dumpTreeYAML(stream, indent + 1);
}
Heap* Heap::findContainHeap_(const void* address) { for (std::vector<Heap*>::iterator it = sHeaps.begin(); it != sHeaps.end(); ++it) if ((*it)->isInclude(address)) return *it; return isInclude(address) ? this : 0; }
void Heap::dispose_(const void*, const void*) { while (mDisposers.next() != &mDisposers) mDisposers.next()->erase_(); }
void Heap::eraseChild_(Heap* child) { if (child && child->mParent == this) child->mParent = 0; }
void Heap::destruct_() { dispose_(mStart, static_cast<unsigned char*>(mStart) + mSize); if (mParent) mParent->eraseChild_(this); }
void Heap::appendDisposer_(IDisposer* disposer) { if (disposer) mDisposers.next()->insertFront_(&disposer->mDisposerNode); }
void Heap::removeDisposer_(IDisposer* disposer) { if (disposer) disposer->mDisposerNode.erase_(); }

ExpHeap::ExpHeap(const SafeStringBase<char>& name, Heap* parent, void* start, unsigned int size, HeapDirection direction, bool enableLock)
    : Heap(name, parent, start, size, direction, enableLock), mAllocMode(cAllocMode_FirstFit), mFreeSize(size), mFreeHead(0), mFreeTail(0), mUseHead(0), mUseTail(0), mOwnsMemory(false)
{
    createMaxSizeFreeMemBlock_(this);
}
ExpHeap::~ExpHeap() {}
ExpHeap* ExpHeap::tryCreate(unsigned int size, const SafeStringBase<char>& name, Heap* parent, HeapDirection direction, bool lock)
{
    unsigned int total = static_cast<unsigned int>(sizeof(ExpHeap)) + size;
    void* storage = parent ? parent->tryAlloc(total, 4) : malloc(total);
    if (!storage) return 0;
    unsigned char* arena = static_cast<unsigned char*>(storage) + sizeof(ExpHeap);
    ExpHeap* heap = new (storage) ExpHeap(name, parent, arena, size, direction, lock);
    heap->mOwnsMemory = true;
    return heap;
}
ExpHeap* ExpHeap::create(unsigned int size, const SafeStringBase<char>& name, Heap* parent, HeapDirection direction, bool lock) { return tryCreate(size, name, parent, direction, lock); }
ExpHeap* ExpHeap::tryCreate(void* storage, unsigned int size, const SafeStringBase<char>& name, bool lock)
{
    if (!storage || size <= sizeof(ExpHeap) + sizeof(MemBlock)) return 0;
    unsigned char* arena = static_cast<unsigned char*>(storage) + sizeof(ExpHeap);
    return new (storage) ExpHeap(name, 0, arena, size - sizeof(ExpHeap), cHeapDirection_Forward, lock);
}
void ExpHeap::pushToFreeList_(MemBlock* block)
{
    block->previous = 0;
    block->next = mFreeHead;
    if (mFreeHead) mFreeHead->previous = block;
    else mFreeTail = block;
    mFreeHead = block;
}
void ExpHeap::createMaxSizeFreeMemBlock_(ExpHeap* heap)
{
    if (heap->mSize < sizeof(MemBlock)) return;
    MemBlock* block = new (heap->mStart) MemBlock();
    block->address = static_cast<unsigned char*>(heap->mStart);
    block->payload = 0;
    block->span = heap->mSize;
    block->size = heap->mSize - sizeof(MemBlock);
    block->alignment = 4;
    heap->pushToFreeList_(block);
}
void ExpHeap::doCreate(ExpHeap* heap, Heap* parent) { if (parent) parent->pushBackChild_(heap); }
bool ExpHeap::compareMemBlockAddr_(const MemBlock* first, const MemBlock* second) { return first < second; }
MemBlock* ExpHeap::findLastMemBlockIfFree_() { for (MemBlock* block = mFreeHead; block; block = block->next) if (block->address + block->span == static_cast<unsigned char*>(mStart) + mSize) return block; return 0; }
int ExpHeap::adjustBack_() { MemBlock* block = findLastMemBlockIfFree_(); if (!block) return 0; int size = static_cast<int>(block->span); removeFreeBlock_(block); mSize -= size; mFreeSize -= size; return size; }
MemBlock* ExpHeap::findFirstMemBlockIfFree_() { for (MemBlock* block = mFreeHead; block; block = block->next) if (block->address == mStart) return block; return 0; }
int ExpHeap::adjustFront_()
{
    MemBlock* block = findFirstMemBlockIfFree_();
    if (!block) return 0;
    int size = static_cast<int>(block->span);
    removeFreeBlock_(block);
    mStart = block->address + block->span;
    mSize -= size;
    mFreeSize -= size;
    return size;
}
MemBlock* ExpHeap::findFreeMemBlockFromHead_(unsigned int size, FindMode mode) const { return findFreeMemBlockFromHead_(size, 4, mode); }
MemBlock* ExpHeap::findFreeMemBlockFromHead_(unsigned int size, int alignment, FindMode mode) const
{
    MemBlock* found = 0;
    unsigned int best = 0xffffffffu;
    unsigned int boundary = alignment < 0 ? static_cast<unsigned int>(-alignment) : static_cast<unsigned int>(alignment);
    if (boundary < 4) boundary = 4;
    for (MemBlock* block = mFreeHead; block; block = block->next)
    {
        size_t payload = ((size_t)(block->address + sizeof(MemBlock)) + boundary - 1) & ~(boundary - 1);
        unsigned int needed = static_cast<unsigned int>(payload - (size_t)block->address) + size;
        if (needed <= block->span && (mode == cFindMode_FirstFit || block->span < best)) { found = block; best = block->span; if (mode == cFindMode_FirstFit) break; }
    }
    return found;
}
void ExpHeap::pushToUseList_(MemBlock* block)
{
    block->previous = 0;
    block->next = mUseHead;
    if (mUseHead) mUseHead->previous = block;
    else mUseTail = block;
    mUseHead = block;
}
void* ExpHeap::allocFromHead_(unsigned int size) { return allocFromHead_(size, 4); }
void* ExpHeap::allocFromHead_(unsigned int size, int alignment)
{
    MemBlock* block = findFreeMemBlockFromHead_(size, alignment, mAllocMode == cAllocMode_FirstFit ? cFindMode_FirstFit : cFindMode_BestFit);
    if (!block) return 0;
    removeFreeBlock_(block);
    unsigned int boundary = alignment < 0 ? static_cast<unsigned int>(-alignment) : static_cast<unsigned int>(alignment);
    if (boundary < 4) boundary = 4;
    unsigned char* payload = (unsigned char*)(((size_t)(block->address + sizeof(MemBlock)) + boundary - 1) & ~(boundary - 1));
    unsigned int used = static_cast<unsigned int>(payload + size - block->address);
    unsigned int remainder = block->span - used;
    if (remainder > sizeof(MemBlock) + 4)
    {
        MemBlock* freeBlock = new (block->address + used) MemBlock();
        freeBlock->address = block->address + used;
        freeBlock->payload = 0;
        freeBlock->span = remainder;
        freeBlock->size = remainder - sizeof(MemBlock);
        freeBlock->alignment = 4;
        pushToFreeList_(freeBlock);
        block->span = used;
    }
    block->payload = payload;
    block->size = size;
    block->alignment = alignment;
    pushToUseList_(block);
    mFreeSize -= block->span;
    return payload;
}
MemBlock* ExpHeap::findFreeMemBlockFromTail_(unsigned int size, FindMode mode) const { return findFreeMemBlockFromTail_(size, -4, mode); }
MemBlock* ExpHeap::findFreeMemBlockFromTail_(unsigned int size, int alignment, FindMode mode) const
{
    MemBlock* found = 0;
    unsigned int best = 0xffffffffu;
    unsigned int boundary = alignment < 0 ? static_cast<unsigned int>(-alignment) : static_cast<unsigned int>(alignment);
    if (boundary < 4) boundary = 4;
    for (MemBlock* block = mFreeTail; block; block = block->previous)
    {
        size_t payload = ((size_t)(block->address + block->span - size)) & ~(boundary - 1);
        if (payload >= (size_t)(block->address + sizeof(MemBlock)) && (mode == cFindMode_FirstFit || block->span < best)) { found = block; best = block->span; if (mode == cFindMode_FirstFit) break; }
    }
    return found;
}
void* ExpHeap::allocFromTail_(unsigned int size) { return allocFromTail_(size, -4); }
void* ExpHeap::allocFromTail_(unsigned int size, int alignment) { return allocFromHead_(size, alignment); }
void* ExpHeap::realloc_(void* address, unsigned char*, unsigned int oldSize, unsigned int newSize, int alignment)
{
    void* replacement = tryAlloc(newSize, alignment);
    if (!replacement) return 0;
    memcpy(replacement, address, oldSize < newSize ? oldSize : newSize);
    free(address);
    return replacement;
}
void ExpHeap::destroy()
{
    Heap* parent = mParent;
    bool owned = mOwnsMemory;
    void* storage = this;
    this->~ExpHeap();
    if (owned) { if (parent) parent->free(storage); else ::free(storage); }
}
unsigned int ExpHeap::adjust() { int adjusted = mDirection == cHeapDirection_Forward ? adjustBack_() : adjustFront_(); return adjusted < 0 ? 0 : static_cast<unsigned int>(adjusted); }
void* ExpHeap::tryAlloc(unsigned int size, int alignment)
{
    unsigned int allocSize = size < 4 ? 4 : size;
    if (allocSize >= mSize)
    {
        if (sAllocFailedCallback)
        {
            HeapMgr::AllocFailedCallbackArg arg = { this, size, alignment };
            sAllocFailedCallback->invoke(&arg);
        }
        return 0;
    }
    unsigned int absAlignment = alignment < 0 ? static_cast<unsigned int>(-alignment) : static_cast<unsigned int>(alignment);
    if ((absAlignment - 1) & absAlignment)
    {
        if (sAllocFailedCallback)
        {
            HeapMgr::AllocFailedCallbackArg arg = { this, size, alignment };
            sAllocFailedCallback->invoke(&arg);
        }
        return 0;
    }
    unsigned int roundedSize = (allocSize + 3) & 0xfffffffcu;
    int effective = alignment * static_cast<int>(mDirection);
    void* result;
    if (effective < 0)
        result = -effective < 5 ? allocFromTail_(roundedSize) : allocFromTail_(roundedSize, effective);
    else
        result = effective < 5 ? allocFromHead_(roundedSize) : allocFromHead_(roundedSize, effective);
    if (!result && sAllocFailedCallback)
    {
        HeapMgr::AllocFailedCallbackArg arg = { this, size, alignment };
        sAllocFailedCallback->invoke(&arg);
    }
    return result;
}
void ExpHeap::removeFreeBlock_(MemBlock* block) { if (block->previous) block->previous->next = block->next; else mFreeHead = block->next; if (block->next) block->next->previous = block->previous; else mFreeTail = block->previous; }
void ExpHeap::removeUseBlock_(MemBlock* block) { if (block->previous) block->previous->next = block->next; else mUseHead = block->next; if (block->next) block->next->previous = block->previous; else mUseTail = block->previous; }
void ExpHeap::free(void* address)
{
    for (MemBlock* block = mUseHead; block; block = block->next)
    {
        if (block->payload != address) continue;
        removeUseBlock_(block);
        mFreeSize += block->span;
        block->payload = 0;
        pushToFreeList_(block);
        return;
    }
}
unsigned int ExpHeap::resizeFront(void* address, unsigned int size)
{
    for (MemBlock* block = mUseHead; block; block = block->next)
    {
        if (block->payload == address && size <= block->size)
        {
            mFreeSize += block->size - size;
            block->size = size;
            return size;
        }
    }
    return 0;
}
unsigned int ExpHeap::resizeBack(void* address, unsigned int size) { return resizeFront(address, size); }
void* ExpHeap::tryRealloc(void* address, unsigned int size, int alignment)
{
    if (!address) return tryAlloc(size, alignment);
    unsigned int oldSize = getAllocatedSize(address);
    return realloc_(address, static_cast<unsigned char*>(address), oldSize, size, alignment);
}
void ExpHeap::freeAll() { mFreeHead = mFreeTail = mUseHead = mUseTail = 0; mFreeSize = mSize; createMaxSizeFreeMemBlock_(this); }
void* ExpHeap::getStartAddress() const { return mStart; }
void* ExpHeap::getEndAddress() const { return static_cast<unsigned char*>(mStart) + mSize; }
unsigned int ExpHeap::getSize() const { return mSize; }
unsigned int ExpHeap::getFreeSize() const { return mFreeSize; }
unsigned int ExpHeap::getMaxAllocatableSize(int alignment) const { MemBlock* block = findFreeMemBlockFromHead_(0, alignment, cFindMode_BestFit); return block ? block->size : 0; }
bool ExpHeap::isInclude(const void* address) const { return address >= mStart && address < static_cast<unsigned char*>(mStart) + mSize; }
bool ExpHeap::isEmpty() const { return mFreeSize == mSize; }
bool ExpHeap::isFreeable() const { return true; }
bool ExpHeap::isResizable() const { return true; }
bool ExpHeap::isAdjustable() const { return true; }
void ExpHeap::dump() const { char text[128]; ::snprintf(text, sizeof(text), "ExpHeap %s size=%u free=%u", mName.cstr(), mSize, mFreeSize); system::PrintString(text, 0); }
void ExpHeap::dumpYAML(WriteStream& stream, int indent) const { Heap::dumpYAML(stream, indent); }
void ExpHeap::setAllocMode(AllocMode mode) { mAllocMode = mode; }
ExpHeap::AllocMode ExpHeap::getAllocMode() const { return mAllocMode; }
unsigned int ExpHeap::getAllocatedSize(void* address) { for (MemBlock* block = mUseHead; block; block = block->next) if (block->payload == address) return block->size; return 0; }
void ExpHeap::dumpUseList() const { for (MemBlock* block = mUseHead; block; block = block->next) { char text[96]; ::snprintf(text, sizeof(text), "use %p %u", block->payload, block->size); system::PrintString(text, 0); } }
void ExpHeap::dumpFreeList() const { for (MemBlock* block = mFreeHead; block; block = block->next) { char text[96]; ::snprintf(text, sizeof(text), "free %p %u", block->address, block->span); system::PrintString(text, 0); } }

FindContainHeapCache::FindContainHeapCache() : mHeap(0) {}
bool FindContainHeapCache::tryRemoveHeap(Heap* heap)
{
    if (mHeap == heap) { mHeap = reinterpret_cast<Heap*>(uintptr_t(1)); return true; }
    return (reinterpret_cast<uintptr_t>(mHeap) & ~uintptr_t(1)) != reinterpret_cast<uintptr_t>(heap);
}

HeapMgr::HeapMgr() : mRootHeap(0), mCurrentHeap(0), mUnknown8(), mLock(), mCacheOwner(0), mCacheList(0) {}
HeapMgr::~HeapMgr()
{
    mLock.lock();
    mCurrentHeap = 0;
    mRootHeap = 0;
    mCacheOwner = 0;
    mCacheList = 0;
    mLock.unlock();
}
void HeapMgr::initialize(Arena*) { if (!sHeapMgr) sHeapMgr = new HeapMgr(); initializeImpl_(); }
void HeapMgr::destroy() { delete sHeapMgr; sHeapMgr = 0; }
Heap* HeapMgr::findContainHeap(const void* address) const { return mRootHeap ? mRootHeap->findContainHeap_(address) : 0; }
Heap* HeapMgr::getCurrentHeap() const { return mCurrentHeap ? mCurrentHeap : mRootHeap; }
bool HeapMgr::isContainedInAnyHeap(const void* address) { return sHeapMgr && sHeapMgr->findContainHeap(address); }
HeapMgr* HeapMgr::instance() { return sHeapMgr; }
void HeapMgr::createRootHeap_()
{
    if (sHeapMgr && sHeapMgr->mRootHeap)
        sHeapMgr->mCurrentHeap = sHeapMgr->mRootHeap;
}
void HeapMgr::initializeImpl_() { createRootHeap_(); }
Heap* HeapMgr::findHeapByName_(Heap* heap, const SafeStringBase<char>& name, int* index)
{
    for (std::vector<Heap*>::iterator it = sHeaps.begin(); it != sHeaps.end(); ++it)
    {
        if (heap && *it != heap && (*it)->mParent != heap) continue;
        if (strcmp((*it)->mName.cstr(), name.cstr()) != 0) continue;
        if (*index == 0) return *it;
        --*index;
    }
    return 0;
}
void HeapMgr::removeFromFindContainHeapCache_(Heap* heap)
{
    if (!heap || !sHeapMgr) return;
    unsigned char* manager = reinterpret_cast<unsigned char*>(sHeapMgr);
    FindContainHeapCache* primaryOwner = *reinterpret_cast<FindContainHeapCache**>(manager + 0x58);
    FindContainHeapCache* primary = primaryOwner ? reinterpret_cast<FindContainHeapCache*>(reinterpret_cast<unsigned char*>(primaryOwner) + 0x70) : 0;
    if (primary)
    {
        while (!primary->tryRemoveHeap(heap)) Thread::sleep(TickSpan(OSNanosecondsToTicks(10000)));
    }
    CriticalSection* lock = reinterpret_cast<CriticalSection*>(manager + 0x1c);
    unsigned char* sentinel = manager + 0x10;
    for (;;)
    {
        lock->lock();
        unsigned char* node = *reinterpret_cast<unsigned char**>(manager + 0x14);
        bool retry = false;
        while (node != sentinel)
        {
            void* owner = *reinterpret_cast<void**>(node + 8);
            FindContainHeapCache* cache = owner ? reinterpret_cast<FindContainHeapCache*>(reinterpret_cast<unsigned char*>(owner) + 0x70) : 0;
            if (!cache || !cache->tryRemoveHeap(heap)) { retry = true; break; }
            node = *reinterpret_cast<unsigned char**>(node + 4);
        }
        lock->unlock();
        if (!retry) return;
        Thread::sleep(TickSpan(OSNanosecondsToTicks(10000)));
    }
}

void* allocateFromSeadHeap(unsigned int size, Heap* heap)
{
    return allocateFromSeadHeap(size, heap, 4);
}

void* allocateFromSeadHeap(unsigned int size)
{
    return allocateFromSeadHeap(size, sHeapMgr ? sHeapMgr->getCurrentHeap() : 0, 4);
}

void* allocateFromSeadHeap(unsigned int size, Heap* heap, int alignment)
{
    return heap ? heap->tryAlloc(size, alignment) : 0;
}

void freeToSeadHeap(void* address)
{
    if (!address || !sHeapMgr) return;
    Heap* heap = sHeapMgr->findContainHeap(address);
    if (heap) heap->free(address);
}

unsigned int getArraySize(void* address)
{
    if (!address) return 0;
    unsigned int header = static_cast<unsigned int*>(address)[-1];
    return header & 1 ? *(unsigned int*)(header + 11) : header;
}

CriticalSection::CriticalSection() : IDisposer() { OSInitMutex(reinterpret_cast<OSMutex*>(mStorage)); }
CriticalSection::CriticalSection(Heap* heap) : IDisposer(heap, cHeapNullOption_Allow) { OSInitMutex(reinterpret_cast<OSMutex*>(mStorage)); }
CriticalSection::~CriticalSection() {}
void CriticalSection::lock() { OSLockMutex(reinterpret_cast<OSMutex*>(mStorage)); }
void CriticalSection::unlock() { OSUnlockMutex(reinterpret_cast<OSMutex*>(mStorage)); }

pead::Mutex::Mutex() : IDisposer() { OSInitMutex(reinterpret_cast<OSMutex*>(mStorage)); }
Mutex::~Mutex() {}
void Mutex::lock() { OSLockMutex(reinterpret_cast<OSMutex*>(mStorage)); }
void Mutex::unlock() { OSUnlockMutex(reinterpret_cast<OSMutex*>(mStorage)); }

long long TickSpan::toNanoSeconds() const { return OSTicksToNanoseconds(mTicks); }
void TickSpan::setNanoSeconds(long long nanoSeconds) { mTicks = OSNanosecondsToTicks(nanoSeconds); }

pead::Event::Event(bool manualReset) : IDisposer() { initialize(manualReset); }
Event::~Event() {}
void Event::initialize(bool manualReset) { OSInitEvent(reinterpret_cast<OSEvent*>(mStorage), false, static_cast<OSEventMode>(manualReset)); }
void Event::wait() { OSWaitEvent(reinterpret_cast<OSEvent*>(mStorage)); }
bool Event::wait(TickSpan timeout) { return OSWaitEventWithTimeout(reinterpret_cast<OSEvent*>(mStorage), timeout.toNanoSeconds()); }
void Event::setSignal() { OSSignalEvent(reinterpret_cast<OSEvent*>(mStorage)); }
void Event::resetSignal() { OSResetEvent(reinterpret_cast<OSEvent*>(mStorage)); }

MessageQueue::MessageQueue() : mMessages(0), mCapacity(0), mCount(0), mRead(0), mWrite(0), mHeap(0) {}
MessageQueue::~MessageQueue() { free(); }
void MessageQueue::allocate(int capacity, Heap* heap) { free(); mMessages = static_cast<int*>(allocateFromSeadHeap(sizeof(int) * (capacity << 4), heap, 4)); mCapacity = mMessages ? capacity : 0; mHeap = heap; }
void MessageQueue::free() { if (mMessages) freeToSeadHeap(mMessages); mMessages = 0; mCapacity = mCount = mRead = mWrite = 0; }
bool MessageQueue::push(int value, BlockType block) { if (mCount >= mCapacity) return false; mMessages[mWrite] = value; mWrite = (mWrite + 1) % mCapacity; ++mCount; (void)block; return true; }
int MessageQueue::pop(BlockType block) { if (!mCount) return 0; int value = mMessages[mRead]; mRead = (mRead + 1) % mCapacity; --mCount; (void)block; return value; }
bool MessageQueue::jam(int value, BlockType block) { if (mCount >= mCapacity) return false; mRead = (mRead + mCapacity - 1) % mCapacity; mMessages[mRead] = value; ++mCount; (void)block; return true; }

Thread::Thread(Heap* heap, OSThread* thread) : IDisposer(heap, cHeapNullOption_Allow), mThreadNameVtable(0), mThreadName("_pead__MainThread"), mMessageQueue(), mStackSize(0), mUnknown5c(0), mUnknown60(0), mUnknown64(0), mUnknown68(0), mUnknown6c(0), mFindContainHeapCache(), mBlockType(static_cast<MessageQueue::BlockType>(1)), mUnknown78(0x7fffffff), mMessageState(static_cast<int>(reinterpret_cast<uintptr_t>(thread))), mThreadState(1), mAffinityMask(1), mThread(thread), mStack(0), mPriority(thread ? OSGetThreadPriority(thread) : 0) {
    mUnknown64 = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(this));
    mMessageState = static_cast<int>(reinterpret_cast<uintptr_t>(thread));
    if (thread)
    {
        unsigned char* raw = reinterpret_cast<unsigned char*>(thread);
        const unsigned int stackTop = *reinterpret_cast<unsigned int*>(raw + 0x398);
        mStack = reinterpret_cast<void*>(static_cast<uintptr_t>(stackTop));
        mStackSize = static_cast<int>(*reinterpret_cast<unsigned int*>(raw + 0x394) - stackTop);
    }
    mMessageQueue.allocate(0x20, heap);
}
Thread::Thread(const SafeStringBase<char>& name, Heap* heap, int priority, MessageQueue::BlockType blockType, int affinityMask, int stackSize, int)
    : IDisposer(heap, cHeapNullOption_Allow), mThreadNameVtable(0), mThreadName(name.cstr()), mMessageQueue(), mStackSize(stackSize), mUnknown5c(0), mUnknown60(0), mUnknown64(0), mUnknown68(0), mUnknown6c(0), mFindContainHeapCache(), mBlockType(blockType), mUnknown78(0x7fffffff), mMessageState(0), mThreadState(0), mAffinityMask(static_cast<unsigned int>(affinityMask)), mThread(0), mStack(0), mPriority(priority) {
    mUnknown64 = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(this));
    if (mStackSize > 0) mStack = allocateFromSeadHeap(static_cast<unsigned int>(mStackSize), heap, 8);
    mThread = static_cast<OSThread*>(allocateFromSeadHeap(1696, heap, 8));
    if (mThread)
    {
        unsigned char* raw = reinterpret_cast<unsigned char*>(mThread);
        *reinterpret_cast<unsigned int*>(raw + 0x394) = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(mStack));
        *reinterpret_cast<unsigned int*>(raw + 0x398) = 0xffffffffu;
    }
    mMessageQueue.allocate(0x20, heap);
}
Thread::~Thread() { if (mStack) freeToSeadHeap(mStack); }
void Thread::waitDone() { if (mThreadState != 0 && mThreadState != 4) { OSJoinThread(mThread, 0); mThreadState = 4; } }
int Thread::cafeThreadFunc_(int, void* context)
{
    Thread* thread = static_cast<Thread*>(context);
    thread->run_();
    thread->mThreadState = 3;
    return 0;
}
unsigned int Thread::start()
{
    if (mThreadState != 0 || !mThread || !mStack || mStackSize <= 0) return 0;
    if (!C4JThreadImpl::OSCreateThread4J(mThread, cafeThreadFunc_, 0, this,
                                         static_cast<unsigned char*>(mStack) + mStackSize,
                                         static_cast<unsigned int>(mStackSize),
                                         static_cast<unsigned int>(mPriority), 0)) return 0;
    OSSetThreadName(mThread, mThreadName);
    unsigned short platformMask = 0;
    for (unsigned int id = 0; id < 3; ++id)
        if (mAffinityMask & (1u << id)) platformMask = static_cast<unsigned short>(platformMask | CoreInfo::getPlatformMask(CoreId(static_cast<int>(id))));
    OSSetThreadAffinity(mThread, platformMask);
    mThreadState = 1;
    mMessageState = static_cast<int>(reinterpret_cast<uintptr_t>(mThread));
    OSResumeThread(mThread);
    return 1;
}
void Thread::sleep(TickSpan span) { OSSleepTicks(span.getTicks()); }
void Thread::yield() { OSYieldThread(); }
void Thread::setPriority(int priority) { mPriority = priority; if (mThread) OSSetThreadPriority(mThread, priority); }
int Thread::getPriority() const { return mThread ? OSGetThreadPriority(mThread) : mPriority; }
void Thread::setAffinity(const CoreIdMask& mask)
{
    mAffinityMask = mask.value();
    unsigned short platformMask = 0;
    for (unsigned int id = 0; id < 3; ++id)
        if (mAffinityMask & (1u << id)) platformMask = static_cast<unsigned short>(platformMask | CoreInfo::getPlatformMask(CoreId(static_cast<int>(id))));
    if (mThread) OSSetThreadAffinity(mThread, platformMask);
}
void* Thread::getStackCheckStartAddress_() const { return mStack ? static_cast<unsigned char*>(mStack) + 4 : 0; }
void Thread::destroy() { waitDone(); }
MessageQueue* Thread::getMessageQueue() const { return const_cast<MessageQueue*>(&mMessageQueue); }
bool Thread::sendMessage(int message, MessageQueue::BlockType block) { if (!message || mThreadState == 3 || mThreadState == 4 || message == mUnknown78) return false; return mMessageQueue.push(message, block); }
int Thread::recvMessage(MessageQueue::BlockType block) { if (mThreadState == 2) return 0; return mMessageQueue.pop(block); }
void Thread::quit(bool jam) { mThreadState = 2; if (jam) mMessageQueue.jam(0, mBlockType); else mMessageQueue.push(0, mBlockType); }
void Thread::quitAndDestroySingleThread(bool jam) { quitAndWaitDoneSingleThread(jam); destroy(); }
void Thread::quitAndWaitDoneSingleThread(bool jam) { quit(jam); waitDone(); }
MessageQueue::BlockType Thread::getBlockType() const { return mBlockType; }
int Thread::getStackSize() const { return mStackSize; }
void Thread::run_() { int message; while ((message = recvMessage(mBlockType)) != mUnknown78) calc_(message); }
int Thread::calcStackUsedSizePeak() const { return 0; }

MainThread::MainThread(Heap* heap, OSThread* thread) : Thread(heap, thread) { mUnknown64 = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(this)); mMessageState = static_cast<int>(reinterpret_cast<uintptr_t>(thread)); mStack = 0; }
MainThread::~MainThread() { mThreadState = 3; }
void MainThread::destroy() {}
void MainThread::quit(bool) {}
void MainThread::waitDone() {}
void MainThread::quitAndDestroySingleThread(bool) {}
void MainThread::setPriority(int) {}
void MainThread::calc_(int) {}

DelegateThread::DelegateThread(const SafeStringBase<char>& name, IDelegate2<Thread*, int>* delegate, Heap* heap, int priority, MessageQueue::BlockType block, int queueSize, int stackSize, int affinity)
    : Thread(name, heap, priority, block, queueSize, stackSize, affinity), mDelegate(delegate) {}
DelegateThread::~DelegateThread() {}
void DelegateThread::calc_(int message) { if (mDelegate) mDelegate->invoke(this, message); }

ThreadMgr::SingletonDisposer_::~SingletonDisposer_() { ThreadMgr::deleteInstance(); }
namespace
{
struct ThreadEntry
{
    ListNode node;
    Thread* thread;
    explicit ThreadEntry(Thread* value) : node(), thread(value) {}
};
std::vector<ThreadEntry*> sThreadEntries;
}

ThreadMgr::ThreadMgr()
    : mUnknown0(), mThreadList(), mUnknown18(0), mLock(), mCurrentThread(0), mMainThread(0) {}
ThreadMgr::~ThreadMgr()
{
    mLock.lock();
    for (ListNode* node = mThreadList.next(); node != &mThreadList; )
    {
        ThreadEntry* entry = reinterpret_cast<ThreadEntry*>(node);
        node = node->next();
        entry->node.erase_();
        delete entry;
    }
    sThreadEntries.clear();
    mLock.unlock();
    destroyMainThread_();
    mCurrentThread = 0;
    sThreadMgr = 0;
}
void ThreadMgr::createInstance(Heap* heap) { if (!sThreadMgr) { sThreadMgr = new ThreadMgr(); sThreadMgr->initialize(heap); } }
void ThreadMgr::deleteInstance() { delete sThreadMgr; sThreadMgr = 0; }
void ThreadMgr::initMainThread_(Heap* heap)
{
    mMainThread = new MainThread(heap, OSGetCurrentThread());
    mCurrentThread = mMainThread;
    ThreadEntry* entry = new ThreadEntry(mMainThread);
    mThreadList.insertBack_(&entry->node);
    sThreadEntries.push_back(entry);
}
unsigned int ThreadMgr::getCurrentThreadID_()
{
    return static_cast<unsigned int>(reinterpret_cast<uintptr_t>(OSGetCurrentThread()));
}
void ThreadMgr::initialize(Heap* heap) { if (!mMainThread) initMainThread_(heap); }
void ThreadMgr::destroyMainThread_()
{
    if (!mMainThread) return;
    mLock.lock();
    for (std::vector<ThreadEntry*>::iterator it = sThreadEntries.begin(); it != sThreadEntries.end(); ++it)
        if ((*it)->thread == mMainThread) { (*it)->node.erase_(); delete *it; sThreadEntries.erase(it); break; }
    mLock.unlock();
    delete mMainThread;
    mMainThread = 0;
}
void ThreadMgr::destroy() { destroyMainThread_(); }
Thread* ThreadMgr::getCurrentThread() const
{
    ThreadMgr* self = const_cast<ThreadMgr*>(this);
    self->mLock.lock();
    const unsigned int id = getCurrentThreadID_();
    for (ListNode* node = self->mThreadList.next(); node != &self->mThreadList; node = node->next())
    {
        ThreadEntry* entry = reinterpret_cast<ThreadEntry*>(node);
        if (entry->thread && static_cast<unsigned int>(reinterpret_cast<uintptr_t>(entry->thread->mThread)) == id)
        {
            Thread* result = entry->thread;
            self->mLock.unlock();
            return result;
        }
    }
    Thread* result = self->mCurrentThread;
    self->mLock.unlock();
    return result;
}
void ThreadMgr::waitDoneMultipleThread(const Thread** threads, int count) { for (int index = 0; index < count; ++index) const_cast<Thread*>(threads[index])->waitDone(); }
void ThreadMgr::quitAndWaitDoneMultipleThread(Thread** threads, int count, bool jam) { for (int index = 0; index < count; ++index) threads[index]->quit(jam); for (int index = 0; index < count; ++index) threads[index]->waitDone(); }

PrintOutput::~PrintOutput() {}
void PrintOutput::writeLineBreak() { write("\n", 1); }
BufferingPrintOutput::~BufferingPrintOutput() {}
void BufferingPrintOutput::write(const char*, int) {}
StringPrintOutput::~StringPrintOutput() {}
void StringPrintOutput::write(const char* text, int length) { if (mString) appendText(mString->getBuffer(), mString->getBufferSize(), text, length); }
StringCutOffPrintOutput::~StringCutOffPrintOutput() {}
void StringCutOffPrintOutput::write(const char* text, int length) { StringPrintOutput::write(text, length); }
StreamPrintOutput::~StreamPrintOutput() {}
void StreamPrintOutput::write(const char* text, int length) { if (mSrc) mSrc->write(text, length); }

void PrintFormatter::proceedToFormatMark_(char* specifier)
{
    specifier[0] = 0;
    if (!mFormat || mPassThrough || mPosition >= mLength) return;
    const char* begin = mFormat + mPosition;
    const char* current = begin;
    while (*current)
    {
        if (*current++ != '%') continue;
        if (*current == '%')
        {
            mOutput->write(begin, static_cast<int>(current - begin));
            ++current;
            mPosition = static_cast<int>(current - mFormat);
            begin = current;
            continue;
        }
        if (current - 1 > begin) mOutput->write(begin, static_cast<int>(current - begin - 1));
        if (*current == '@')
        {
            mPosition = static_cast<int>(current - mFormat + 1);
            return;
        }
        if (*current == '<')
        {
            mPassThrough = true;
            mPosition = static_cast<int>(current - mFormat + 1);
            return;
        }
        int length = 0;
        specifier[length++] = '%';
        while (*current && length < 31)
        {
            char character = *current++;
            specifier[length++] = character;
            if (!(character == '+' || character == '-' || character == ' ' || character == '#' || character == 'h' || character == 'l' || character == 'L' || character == '.' || (character >= '0' && character <= '9'))) break;
        }
        specifier[length] = 0;
        mPosition = static_cast<int>(current - mFormat);
        return;
    }
    if (current > begin) mOutput->write(begin, static_cast<int>(current - begin));
    mPosition = mLength;
}
void PrintFormatter::outputString_(const char*, PrintOutput* output, const char* text, int length) { if (output) output->write(text, length); }
void PrintFormatter::outputPtr_(const char* format, PrintOutput* output, unsigned int pointer) { char text[32]; int length = ::snprintf(text, sizeof(text), format, pointer); if (output) output->write(text, length); }

template <typename T>
void PrintFormatter::out(const T& value, const char* format, PrintOutput* output)
{
    char text[128];
    int length = ::snprintf(text, sizeof(text), format, value);
    if (length > 0 && output) output->write(text, length);
}

void PrintFormatter::out(const char* value, const char* format, PrintOutput* output) { out<const char*>(value, format, output); }
void PrintFormatter::out(const wchar_t* value, const char*, PrintOutput* output)
{
    char text[256];
    int length = StringUtil::convertUtf16ToUtf8(text, sizeof(text), value, -1);
    if (length > 0 && output) output->write(text, length);
}
void PrintFormatter::out(const Heap& value, const char* format, PrintOutput* output) { outputPtr_(format, output, static_cast<unsigned int>((size_t)&value)); }
void PrintFormatter::out(const ExpHeap& value, const char* format, PrintOutput* output) { outputPtr_(format, output, static_cast<unsigned int>((size_t)&value)); }
void PrintFormatter::out(const TickSpan& value, const char* format, PrintOutput* output) { out<long long>(value.toNanoSeconds(), format, output); }

template <typename T, template <typename> class Wrapper>
void PrintFormatter::OutImpl<T, Wrapper>::out(const Wrapper<T>& value, const char* format, PrintOutput* output)
{
    PrintFormatter::out(value, format, output);
}

template <>
void PrintFormatter::OutImpl<char, BufferedSafeStringBase>::out__S(const BufferedSafeStringBase<char>& value, const char* format, PrintOutput* output)
{
    PrintFormatter::out(value.cstr(), format, output);
}

template <>
void PrintFormatter::OutImpl<char, SafeStringBase>::out__S(const SafeStringBase<char>& value, const char* format, PrintOutput* output)
{
    PrintFormatter::out(value.cstr(), format, output);
}

template <>
void PrintFormatter::OutImpl<unsigned char, BitFlag>::out__S(const BitFlag<unsigned char>& value, const char* format, PrintOutput* output)
{
    PrintFormatter::out(value.getDirect(), format, output);
}

template <>
void PrintFormatter::OutImpl<unsigned int, BitFlag>::out__S(const BitFlag<unsigned int>& value, const char* format, PrintOutput* output)
{
    PrintFormatter::out(value.getDirect(), format, output);
}

template <>
void PrintFormatter::OutImpl<unsigned long long, BitFlag>::out__S(const BitFlag<unsigned long long>& value, const char* format, PrintOutput* output)
{
    PrintFormatter::out(value.getDirect(), format, output);
}

template <>
void PrintFormatter::OutImpl<unsigned short, BitFlag>::out__S(const BitFlag<unsigned short>& value, const char* format, PrintOutput* output)
{
    PrintFormatter::out(value.getDirect(), format, output);
}

template void PrintFormatter::out<unsigned char>(const unsigned char&, const char*, PrintOutput*);
template void PrintFormatter::out<unsigned short>(const unsigned short&, const char*, PrintOutput*);
template void PrintFormatter::out<unsigned int>(const unsigned int&, const char*, PrintOutput*);
template void PrintFormatter::out<unsigned long long>(const unsigned long long&, const char*, PrintOutput*);
template void PrintFormatter::out<signed char>(const signed char&, const char*, PrintOutput*);
template void PrintFormatter::out<short>(const short&, const char*, PrintOutput*);
template void PrintFormatter::out<int>(const int&, const char*, PrintOutput*);
template void PrintFormatter::out<long long>(const long long&, const char*, PrintOutput*);
template void PrintFormatter::out<float>(const float&, const char*, PrintOutput*);
template void PrintFormatter::out<double>(const double&, const char*, PrintOutput*);
template void PrintFormatter::out<char>(const char&, const char*, PrintOutput*);
template void PrintFormatter::out<bool>(const bool&, const char*, PrintOutput*);

void PrintConfig::registerFinalCallback(IDelegate1<const PrintEventArg&>* callback) { sFinalPrintCallback = callback; }
void PrintConfig::execCallbacks(const PrintEventArg& event) { if (sFinalPrintCallback) sFinalPrintCallback->invoke(event); }

int StringUtil::__N_18_peadStringUtil_cpp_6714fef4::checkAndConvertDecChar_(char character)
{
    return character > '9' ? -1 : character - '0';
}

int StringUtil::__N_18_peadStringUtil_cpp_6714fef4::checkAndConvertHexChar_(char character)
{
    if (character >= 'A' && character <= 'Z') character = static_cast<char>(character + ('a' - 'A'));
    if (character >= '0' && character <= '9') return character - '0';
    if (character >= 'a' && character <= 'f') return character - 'a' + 10;
    return -1;
}

void StringUtil::__N_18_peadStringUtil_cpp_6714fef4::tryParseSignImpl_(int* sign, SafeStringBase<char>::iterator& current)
{
    *sign = 1;
    if (*current == '-') { *sign = -1; ++current; }
    else if (*current == '+') ++current;
}

bool StringUtil::__N_18_peadStringUtil_cpp_6714fef4::tryParsePrefixImpl_(int* base, SafeStringBase<char>::iterator& current, const SafeStringBase<char>& text)
{
    if (*base == 0) *base = 10;
    SafeStringBase<char>::iterator next = current;
    if (next == text.end() || *next != '0') return true;
    ++next;
    if (next == text.end()) return true;
    if ((*next == 'x' || *next == 'X') && (*base == 10 || *base == 16)) { *base = 16; ++next; current = next; }
    else if ((*next == 'b' || *next == 'B') && (*base == 10 || *base == 2)) { *base = 2; ++next; current = next; }
    else if (*base == 10) *base = 8;
    return true;
}

template <typename T>
bool StringUtil::__N_18_peadStringUtil_cpp_6714fef4::tryParseNumberImpl_(T* value, SafeStringBase<char>::iterator& current, int base)
{
    T parsed = 0;
    bool found = false;
    for (;; ++current)
    {
        char character = *current;
        if (!character) break;
        int digit = base == 16 ? checkAndConvertHexChar_(character) : checkAndConvertDecChar_(character);
        if (digit < 0 || digit >= base) break;
        parsed = static_cast<T>(parsed * base + digit);
        found = true;
    }
    if (found) *value = parsed;
    return found;
}

template <typename T>
bool StringUtil::__N_18_peadStringUtil_cpp_6714fef4::tryParseDecImpl_(T* value, SafeStringBase<char>::iterator& current, int sign)
{
    T parsed = 0;
    if (!tryParseNumberImpl_(&parsed, current, 10)) return false;
    *value = static_cast<T>(parsed * sign);
    return true;
}

template <typename T>
bool StringUtil::__N_18_peadStringUtil_cpp_6714fef4::tryParseImpl_(T* value, const SafeStringBase<char>& text, int base)
{
    if (!value) return false;
    SafeStringBase<char>::iterator current = text.begin();
    int sign;
    tryParseSignImpl_(&sign, current);
    tryParsePrefixImpl_(&base, current, text);
    T parsed = 0;
    if (!tryParseNumberImpl_(&parsed, current, base) || *current) return false;
    *value = static_cast<T>(parsed * sign);
    return true;
}

template <typename T>
void StringUtil::__N_18_peadStringUtil_cpp_6714fef4::tryParseUnderDecimalPointImpl_(T* value, SafeStringBase<char>::iterator& current, T place, T sign)
{
    while (*current)
    {
        int digit = checkAndConvertDecChar_(*current);
        if (digit < 0 || digit > 9) break;
        *value += static_cast<T>(digit) * place * sign;
        place /= static_cast<T>(10);
        ++current;
    }
}

template <typename T>
bool StringUtil::__N_18_peadStringUtil_cpp_6714fef4::tryParseDecimalFractionImpl_(T* value, const SafeStringBase<char>& text)
{
    if (!value) return false;
    char* end = 0;
    double parsed = strtod(text.cstr(), &end);
    if (end == text.cstr() || *end) return false;
    *value = static_cast<T>(parsed);
    return true;
}

template <typename T>
T pead::StringUtil::__N_18_peadStringUtil_cpp_6714fef4::parseImpl_(const SafeStringBase<char>& text, CardinalNumber cardinal)
{
    T value = 0;
    tryParseImpl_(&value, text, cardinal == cHexadecimal ? 16 : cardinal == cOctal ? 8 : cardinal == cBinary ? 2 : 10);
    return value;
}

template <typename T>
T StringUtil::__N_18_peadStringUtil_cpp_6714fef4::parseDecimalFractionImpl_(const SafeStringBase<char>& text)
{
    T value = 0;
    tryParseDecimalFractionImpl_(&value, text);
    return value;
}

typedef StringUtil::__N_18_peadStringUtil_cpp_6714fef4 StringUtilImpl;
typedef StringUtilImpl::FormatParser16 FormatParser16;
typedef StringUtilImpl::OutBuffer16 OutBuffer16;

FormatParser16::FormatParser16(va_list* args) : mArgs(args) { clear_(); }
void FormatParser16::clear_() { mLeftJustify = mForceSign = mBlankSign = mAlternate = mZeroPad = false; mWidth = 0; mPrecision = -1; mSize = 0; }
const wchar_t* FormatParser16::parseFlag_(const wchar_t* format)
{
    for (;; ++format)
    {
        if (*format == L'-') mLeftJustify = true;
        else if (*format == L'+') mForceSign = true;
        else if (*format == L' ') mBlankSign = true;
        else if (*format == L'#') mAlternate = true;
        else if (*format == L'0') mZeroPad = true;
        else return format;
    }
}
const wchar_t* FormatParser16::parseWidth_(const wchar_t* format)
{
    mWidth = 0;
    if (*format == L'*') { mWidth = va_arg(*mArgs, int); return format + 1; }
    while (*format >= L'0' && *format <= L'9') mWidth = mWidth * 10 + *format++ - L'0';
    return format;
}
const wchar_t* FormatParser16::parsePrecision_(const wchar_t* format)
{
    mPrecision = -1;
    if (*format != L'.') return format;
    ++format;
    mPrecision = 0;
    if (*format == L'*') { mPrecision = va_arg(*mArgs, int); return format + 1; }
    while (*format >= L'0' && *format <= L'9') mPrecision = mPrecision * 10 + *format++ - L'0';
    return format;
}
const wchar_t* FormatParser16::parseSize_(const wchar_t* format)
{
    mSize = 0;
    if (*format == L'h' || *format == L'l' || *format == L'L') { mSize = *format++; if (mSize == L'l' && *format == L'l') { mSize = L'L'; ++format; } }
    return format;
}

template <typename T>
void FormatParser16::printIntegerInverseString_(OutBuffer16* output, unsigned int base, bool upper, T value)
{
    wchar_t reverse[65];
    int length = 0;
    do { unsigned int digit = static_cast<unsigned int>(value % base); reverse[length++] = static_cast<wchar_t>(digit < 10 ? L'0' + digit : (upper ? L'A' : L'a') + digit - 10); value = static_cast<T>(value / base); } while (value);
    while (length) appendWide(output, reverse[--length]);
}

template <typename T>
void FormatParser16::printIntegerInverseStringFromArg_(OutBuffer16* output, bool* negative, bool signedValue, unsigned int base, bool upper)
{
    T value = va_arg(*mArgs, T);
    if (signedValue && value < 0) { *negative = true; printIntegerInverseString_(output, base, upper, static_cast<unsigned long long>(-(long long)value)); }
    else printIntegerInverseString_(output, base, upper, static_cast<unsigned long long>(value));
}

template <typename T>
void FormatParser16::printStringCore_(OutBuffer16* output, const T* text)
{
    if (!text) return;
    int count = 0;
    while (*text && (mPrecision < 0 || count < mPrecision)) { appendWide(output, static_cast<wchar_t>(*text++)); ++count; }
}

void FormatParser16::printInteger_(OutBuffer16* output, unsigned int base, bool signedValue, bool upper)
{
    wchar_t text[96];
    wchar_t format[24];
    wchar_t type = base == 16 ? (upper ? L'X' : L'x') : base == 8 ? L'o' : L'd';
    ::swprintf(format, 24, L"%%%s%s%s%s%s%d.%d%c", mLeftJustify ? L"-" : L"", mForceSign ? L"+" : mBlankSign ? L" " : L"", mAlternate ? L"#" : L"", mZeroPad ? L"0" : L"", mSize == L'L' ? L"ll" : mSize == L'l' ? L"l" : L"", mWidth, mPrecision < 0 ? 1 : mPrecision, type);
    if (mSize == L'L') ::swprintf(text, 96, format, signedValue ? va_arg(*mArgs, long long) : static_cast<long long>(va_arg(*mArgs, unsigned long long)));
    else ::swprintf(text, 96, format, signedValue ? va_arg(*mArgs, int) : static_cast<int>(va_arg(*mArgs, unsigned int)));
    printStringCore_(output, text);
}
void FormatParser16::printCharacter_(OutBuffer16* output) { appendWide(output, static_cast<wchar_t>(va_arg(*mArgs, int))); }
void FormatParser16::printString_(OutBuffer16* output) { if (mSize == L'l') printStringCore_(output, va_arg(*mArgs, const wchar_t*)); else printStringCore_(output, va_arg(*mArgs, const char*)); }
void FormatParser16::printFloat_(OutBuffer16* output, wchar_t type, bool)
{
    wchar_t text[128];
    wchar_t format[24];
    ::swprintf(format, 24, L"%%%s%s%s%s%d.%d%c", mLeftJustify ? L"-" : L"", mForceSign ? L"+" : mBlankSign ? L" " : L"", mAlternate ? L"#" : L"", mZeroPad ? L"0" : L"", mWidth, mPrecision < 0 ? 6 : mPrecision, type);
    ::swprintf(text, 128, format, va_arg(*mArgs, double));
    printStringCore_(output, text);
}
int FormatParser16::execute(OutBuffer16* output, const wchar_t* format)
{
    output->length = 0;
    while (*format)
    {
        if (*format != L'%') { appendWide(output, *format++); continue; }
        ++format;
        if (*format == L'%') { appendWide(output, *format++); continue; }
        clear_();
        format = parseSize_(parsePrecision_(parseWidth_(parseFlag_(format))));
        wchar_t type = *format ? *format++ : 0;
        if (type == L'd' || type == L'i') printInteger_(output, 10, true, false);
        else if (type == L'u') printInteger_(output, 10, false, false);
        else if (type == L'x' || type == L'X') printInteger_(output, 16, false, type == L'X');
        else if (type == L'o') printInteger_(output, 8, false, false);
        else if (type == L'c') printCharacter_(output);
        else if (type == L's') printString_(output);
        else if (type == L'f' || type == L'e' || type == L'E' || type == L'g' || type == L'G') printFloat_(output, type, type == L'E' || type == L'G');
    }
    int termination = output->length < output->capacity ? output->length : output->capacity - 1;
    if (output->capacity > 0) output->buffer[termination] = 0;
    return output->length;
}

template bool StringUtilImpl::tryParseNumberImpl_<unsigned long long>(unsigned long long*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseNumberImpl_<unsigned int>(unsigned int*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseNumberImpl_<unsigned short>(unsigned short*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseNumberImpl_<unsigned char>(unsigned char*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<unsigned long long>(unsigned long long*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<long long>(long long*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<unsigned int>(unsigned int*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<int>(int*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<unsigned short>(unsigned short*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<short>(short*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<unsigned char>(unsigned char*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<signed char>(signed char*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<float>(float*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseDecImpl_<double>(double*, SafeStringBase<char>::iterator&, int);
template bool StringUtilImpl::tryParseImpl_<unsigned long long>(unsigned long long*, const SafeStringBase<char>&, int);
template bool StringUtilImpl::tryParseImpl_<long long>(long long*, const SafeStringBase<char>&, int);
template bool StringUtilImpl::tryParseImpl_<unsigned int>(unsigned int*, const SafeStringBase<char>&, int);
template bool StringUtilImpl::tryParseImpl_<int>(int*, const SafeStringBase<char>&, int);
template bool StringUtilImpl::tryParseImpl_<unsigned short>(unsigned short*, const SafeStringBase<char>&, int);
template bool StringUtilImpl::tryParseImpl_<short>(short*, const SafeStringBase<char>&, int);
template bool StringUtilImpl::tryParseImpl_<unsigned char>(unsigned char*, const SafeStringBase<char>&, int);
template bool StringUtilImpl::tryParseImpl_<signed char>(signed char*, const SafeStringBase<char>&, int);
template void StringUtilImpl::tryParseUnderDecimalPointImpl_<float>(float*, SafeStringBase<char>::iterator&, float, float);
template void StringUtilImpl::tryParseUnderDecimalPointImpl_<double>(double*, SafeStringBase<char>::iterator&, double, double);
template bool StringUtilImpl::tryParseDecimalFractionImpl_<float>(float*, const SafeStringBase<char>&);
template bool StringUtilImpl::tryParseDecimalFractionImpl_<double>(double*, const SafeStringBase<char>&);
template int StringUtilImpl::parseImpl_<int>(const SafeStringBase<char>&, CardinalNumber);
template void FormatParser16::printIntegerInverseString_<unsigned int>(OutBuffer16*, unsigned int, bool, unsigned int);
template void FormatParser16::printIntegerInverseString_<unsigned long long>(OutBuffer16*, unsigned int, bool, unsigned long long);
template void FormatParser16::printIntegerInverseStringFromArg_<unsigned int>(OutBuffer16*, bool*, bool, unsigned int, bool);
template void FormatParser16::printIntegerInverseStringFromArg_<int>(OutBuffer16*, bool*, bool, unsigned int, bool);
template void FormatParser16::printIntegerInverseStringFromArg_<long long>(OutBuffer16*, bool*, bool, unsigned int, bool);
template void FormatParser16::printIntegerInverseStringFromArg_<unsigned long long>(OutBuffer16*, bool*, bool, unsigned int, bool);
template void FormatParser16::printStringCore_<char>(OutBuffer16*, const char*);
template void FormatParser16::printStringCore_<wchar_t>(OutBuffer16*, const wchar_t*);

bool StringUtil::tryParseU64(unsigned long long* v, const SafeStringBase<char>& s, CardinalNumber c) { return parseInteger(v, s, c); }
bool StringUtil::tryParseS64(long long* v, const SafeStringBase<char>& s, CardinalNumber c) { return parseSignedInteger(v, s, c); }
bool StringUtil::tryParseU32(unsigned int* v, const SafeStringBase<char>& s, CardinalNumber c) { return parseInteger(v, s, c); }
bool StringUtil::tryParseS32(int* v, const SafeStringBase<char>& s, CardinalNumber c) { return parseSignedInteger(v, s, c); }
bool StringUtil::tryParseU16(unsigned short* v, const SafeStringBase<char>& s, CardinalNumber c) { return parseInteger(v, s, c); }
bool StringUtil::tryParseS16(short* v, const SafeStringBase<char>& s, CardinalNumber c) { return parseSignedInteger(v, s, c); }
bool StringUtil::tryParseU8(unsigned char* v, const SafeStringBase<char>& s, CardinalNumber c) { return parseInteger(v, s, c); }
bool StringUtil::tryParseS8(signed char* v, const SafeStringBase<char>& s, CardinalNumber c) { return parseSignedInteger(v, s, c); }
int StringUtil::parseS32(const SafeStringBase<char>& s, CardinalNumber c) { int value = 0; tryParseS32(&value, s, c); return value; }
int StringUtil::vsw16printf(wchar_t* b, unsigned int n, const wchar_t* f, va_list a) { return ::vswprintf(b, n, f, a); }
int StringUtil::vsnprintf(char* b, unsigned int n, const char* f, va_list a) { return ::vsnprintf(b, n, f, a); }
int StringUtil::convertUtf16ToSjis(char* destination, unsigned int capacity, const wchar_t* source, int length)
{
    if (!destination || capacity == 0) return 0;
    unsigned int written = 0;
    for (int index = 0; source && source[index] && (length < 0 || index < length) && written + 1 < capacity; ++index)
        destination[written++] = source[index] <= 0x7f ? static_cast<char>(source[index]) : '?';
    destination[written] = 0;
    return static_cast<int>(written);
}
int StringUtil::convertUtf16ToUtf8(char* destination, unsigned int capacity, const wchar_t* source, int length)
{
    if (!destination || capacity == 0) return 0;
    unsigned int written = 0;
    for (int index = 0; source && source[index] && (length < 0 || index < length); ++index)
    {
        unsigned int character = static_cast<unsigned int>(source[index]);
        if (character < 0x80)
        {
            if (written + 1 >= capacity) break;
            destination[written++] = static_cast<char>(character);
        }
        else if (character < 0x800)
        {
            if (written + 2 >= capacity) break;
            destination[written++] = static_cast<char>(0xc0 | character >> 6);
            destination[written++] = static_cast<char>(0x80 | character & 0x3f);
        }
        else
        {
            if (written + 3 >= capacity) break;
            destination[written++] = static_cast<char>(0xe0 | character >> 12);
            destination[written++] = static_cast<char>(0x80 | character >> 6 & 0x3f);
            destination[written++] = static_cast<char>(0x80 | character & 0x3f);
        }
    }
    destination[written] = 0;
    return static_cast<int>(written);
}
int StringUtil::convertUtf8ToSjis(char* destination, unsigned int capacity, const char* source, int length)
{
    if (!destination || capacity == 0) return 0;
    unsigned int written = 0;
    for (int index = 0; source && source[index] && (length < 0 || index < length) && written + 1 < capacity; ++index)
    {
        unsigned char character = static_cast<unsigned char>(source[index]);
        if (character < 0x80) destination[written++] = static_cast<char>(character);
        else
        {
            destination[written++] = '?';
            while ((static_cast<unsigned char>(source[index + 1]) & 0xc0) == 0x80) ++index;
        }
    }
    destination[written] = 0;
    return static_cast<int>(written);
}
int StringUtil::compareChar16Pair(const Char16Pair& pair, const wchar_t& value) { return value < pair.from ? 1 : value > pair.from ? -1 : 0; }
wchar_t StringUtil::replace(wchar_t value, const Buffer<const Char16Pair>& pairs)
{
    int first = 0;
    int last = pairs.size();
    while (first < last)
    {
        int middle = first + (last - first) / 2;
        int order = compareChar16Pair(pairs[middle], value);
        if (order == 0) return pairs[middle].to;
        if (order < 0) first = middle + 1;
        else last = middle;
    }
    return value;
}
wchar_t StringUtil::toUpperCapital(wchar_t value) { return value >= L'a' && value <= L'z' ? value - (L'a' - L'A') : value; }
void StringUtil::toUpperCapitalFirstCharactor(BufferedSafeStringBase<wchar_t>* text) { if (text && text->getBufferSize()) text->getBuffer()[0] = toUpperCapital(text->getBuffer()[0]); }
wchar_t StringUtil::toLowerCapital(wchar_t value) { return value >= L'A' && value <= L'Z' ? value + (L'a' - L'A') : value; }
void StringUtil::toLowerCapitalFirstCharactor(BufferedSafeStringBase<wchar_t>* text) { if (text && text->getBufferSize()) text->getBuffer()[0] = toLowerCapital(text->getBuffer()[0]); }

namespace system
{
void PrintStringImpl(const char*, int) {}
void PrintString(const char* text, int level) { PrintConfig::execCallbacks(PrintConfig::PrintEventArg(text, level)); }
}

int replaceStringImpl(char* destination, int* resultLength, int capacity, const char* source, int sourceLength, const SafeStringBase<char>& from, const SafeStringBase<char>& to, bool* overflow)
{
    if (overflow) *overflow = false;
    if (!destination || capacity <= 0) return 0;
    int replacements = 0;
    int written = 0;
    int matchLength = from.calcLength();
    int replacementLength = to.calcLength();
    for (int index = 0; index < sourceLength && written < capacity - 1;)
    {
        bool match = matchLength > 0 && index + matchLength <= sourceLength && memcmp(source + index, from.cstr(), matchLength) == 0;
        const char* text = match ? to.cstr() : source + index;
        int length = match ? replacementLength : 1;
        int available = capacity - 1 - written;
        int copyLength = length < available ? length : available;
        memcpy(destination + written, text, copyLength);
        written += copyLength;
        index += match ? matchLength : 1;
        if (match) ++replacements;
        if (copyLength != length && overflow) *overflow = true;
    }
    destination[written] = 0;
    if (resultLength) *resultLength = written;
    return replacements;
}

int replaceStringImpl(wchar_t* destination, int* resultLength, int capacity, const wchar_t* source, int sourceLength, const SafeStringBase<wchar_t>& from, const SafeStringBase<wchar_t>& to, bool* overflow)
{
    if (overflow) *overflow = false;
    if (!destination || capacity <= 0) return 0;
    int replacements = 0;
    int written = 0;
    int matchLength = from.calcLength();
    int replacementLength = to.calcLength();
    for (int index = 0; index < sourceLength && written < capacity - 1;)
    {
        bool match = matchLength > 0 && index + matchLength <= sourceLength && wmemcmp(source + index, from.cstr(), matchLength) == 0;
        const wchar_t* text = match ? to.cstr() : source + index;
        int length = match ? replacementLength : 1;
        int available = capacity - 1 - written;
        int copyLength = length < available ? length : available;
        wmemcpy(destination + written, text, copyLength);
        written += copyLength;
        index += match ? matchLength : 1;
        if (match) ++replacements;
        if (copyLength != length && overflow) *overflow = true;
    }
    destination[written] = 0;
    if (resultLength) *resultLength = written;
    return replacements;
}

template <typename T>
int replaceStringImpl_(T* destination, int* resultLength, int capacity, const T* source, int sourceLength, const SafeStringBase<T>& from, const SafeStringBase<T>& to, bool* overflow)
{
    return replaceStringImpl(destination, resultLength, capacity, source, sourceLength, from, to, overflow);
}

template int replaceStringImpl_<char>(char*, int*, int, const char*, int, const SafeStringBase<char>&, const SafeStringBase<char>&, bool*);
template int replaceStringImpl_<wchar_t>(wchar_t*, int*, int, const wchar_t*, int, const SafeStringBase<wchar_t>&, const SafeStringBase<wchar_t>&, bool*);
}
