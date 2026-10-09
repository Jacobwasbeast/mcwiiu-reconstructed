#include "platform/wiiu/pead.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

#include <algorithm>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <new>

#include <coreinit/time.h>
#include <coreinit/core.h>
#include <coreinit/mutex.h>
#include <coreinit/thread.h>
#include <coreinit/event.h>

namespace pead
{
namespace
{
BinaryStreamFormat sBinaryFormat;
TextStreamFormat sTextFormat;
FixedSafeString1024 sTextBuffer;
Mutex sMutex;
IDelegate1<const PrintConfig::PrintEventArg&>* sFinalPrintCallback;

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

void ListNode::insertBack_(ListNode* node)
{
    ListNode* next = mNext;
    mNext = node;
    node->mPrev = this;
    node->mNext = next;
    if (next)
        next->mPrev = node;
}

void ListNode::insertFront_(ListNode* node)
{
    ListNode* prev = mPrev;
    mPrev = node;
    node->mPrev = prev;
    node->mNext = this;
    if (prev)
        prev->mNext = node;
}

void ListNode::erase_()
{
    if (mPrev)
        mPrev->mNext = mNext;
    if (mNext)
        mNext->mPrev = mPrev;
    mNext = 0;
    mPrev = 0;
}

ListNode* ListImpl::nth(int index) const
{
    if (index < 0 || index >= mCount) return 0;
    ListNode* node = mStartEnd.next();
    while (index--) node = node->next();
    return node;
}

void ListImpl::sort(int offset, int (*compare)(const void*, const void*))
{
    if (mCount <= 1)
        return;

    ListNode* node = nth(1);
    while (node != &mStartEnd)
    {
        ListNode* next = node->next();
        if (compare(reinterpret_cast<const char*>(node->prev()) - offset,
                    reinterpret_cast<const char*>(node) - offset) > 0)
        {
            ListNode* insertion = node->prev();
            while (insertion != &mStartEnd &&
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
    ListNode* node = mStartEnd.mNext;
    while (node != &mStartEnd)
    {
        ListNode* current = node;
        node = node->mNext;
        current->mNext = 0;
        current->mPrev = 0;
    }
    mStartEnd.mPrev = &mStartEnd;
    mStartEnd.mNext = &mStartEnd;
    mCount = 0;
}

void PtrArrayImpl::setBuffer(int capacity, void* buffer)
{
    if (capacity < 1) return;
    if (!buffer) return;
    mPtrNumMax = capacity;
    mPtrs = static_cast<void**>(buffer);
    mPtrNum = 0;
}

bool PtrArrayImpl::tryAllocBuffer(int capacity, Heap* heap, int alignment)
{
    if (capacity > 0)
    {
        unsigned int size = sizeof(void*) * capacity;
        unsigned char* buffer = static_cast<unsigned char*>(allocateFromSeadHeap(size, heap, alignment));
        if (!buffer) return false;
        for (unsigned char* byte = buffer; size != 0; --size, ++byte)
            new (byte) unsigned char();
        if (buffer)
        {
            setBuffer(capacity, buffer);
            return true;
        }
    }
    return false;
}

bool PtrArrayImpl::allocBuffer(int capacity, Heap* heap, int alignment)
{
    return tryAllocBuffer(capacity, heap, alignment);
}

void PtrArrayImpl::freeBuffer()
{
    if (mPtrs)
    {
        DeleteArray(reinterpret_cast<unsigned char*>(mPtrs));
        mPtrs = 0;
        mPtrNum = 0;
        mPtrNumMax = 0;
    }
}

void PtrArrayImpl::erase(int index, int count)
{
    if (index >= 0 && count >= 0)
    {
        int end = index + count;
        if (end <= mPtrNum)
        {
            if (end < mPtrNum) memmove(mPtrs + index, mPtrs + end, sizeof(void*) * (mPtrNum - end));
            mPtrNum -= count;
        }
    }
}

bool PtrArrayImpl::checkInsert(int index, int count)
{
    return index >= 0 && mPtrNum + count <= mPtrNumMax && index <= mPtrNum;
}

void PtrArrayImpl::insert(int index, void* value)
{
    if (checkInsert(index, 1))
    {
        if (index < mPtrNum) memmove(mPtrs + index + 1, mPtrs + index, sizeof(void*) * (mPtrNum - index));
        mPtrs[index] = value;
        ++mPtrNum;
    }
}

int PtrArrayImpl::binarySearch(const void* value, int (*compare)(const void*, const void*)) const
{
    int low = 0;
    int high = mPtrNum;
    while (low < high)
    {
        int middle = (low + high) / 2;
        int result = compare(value, mPtrs[middle]);
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
    unsigned int restSize = 0;
    if (size > static_cast<unsigned int>(string->getBufferSize()))
    {
        restSize = size - string->getBufferSize();
        size = string->getBufferSize();
    }
    src->read(string->getBuffer(), size);
    if (size + 1 < static_cast<unsigned int>(string->getBufferSize()))
        string->trim(size);
    else
        string->trim(string->getBufferSize() - 1);
    if (restSize != 0)
        src->read(string->getBuffer(), restSize);
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
TextStreamFormat::TextStreamFormat() : mSeparator(" \t\n") {}
void TextStreamFormat::getNextData_(StreamSrc* src)
{
    char commentEnd = '\0';
    bool quoted = false;
    bool multiByte = false;
    char character;
    sTextBuffer.clear();
    int length = 0;
    while (src->read(&character, 1) != 0)
    {
        if (commentEnd != '\0')
        {
            if (__N_24_peadTextStreamFormat_cpp_8d174ff4::isSJISCode_(character))
            {
                if (src->read(&character, 1) == 0)
                    return;
                continue;
            }
            if (character == commentEnd)
            {
                if (commentEnd == '*')
                {
                    commentEnd = '/';
                    continue;
                }
                commentEnd = '\0';
                character = mSeparator[0];
            }
            else
            {
                if (commentEnd == '/')
                    commentEnd = '*';
                continue;
            }
        }
        if (quoted)
        {
            if (character == '"')
            {
                if (!multiByte && length > 1 && sTextBuffer[length - 1] == '\\')
                {
                    sTextBuffer.copyAt(length - 1, "\"", 1);
                    continue;
                }
                return;
            }
            sTextBuffer.append(character);
            if (!__N_24_peadTextStreamFormat_cpp_8d174ff4::isSJISCode_(character))
            {
                multiByte = false;
                ++length;
                continue;
            }
        }
        else
        {
            if (length == 0 && character == '"')
            {
                quoted = true;
                continue;
            }
            if (mSeparator.include(character))
            {
                if (!sTextBuffer.isEmpty())
                    return;
                continue;
            }
            sTextBuffer.append(character);
            if (!__N_24_peadTextStreamFormat_cpp_8d174ff4::isSJISCode_(character))
            {
                multiByte = false;
                ++length;
                if (length >= 1 && sTextBuffer[length - 1] == '#')
                {
                    commentEnd = '\n';
                    --length;
                    sTextBuffer.trim(length);
                }
                if (length >= 2 && sTextBuffer[length - 2] == '/' && sTextBuffer[length - 1] == '/')
                {
                    commentEnd = '\n';
                    length -= 2;
                    sTextBuffer.trim(length);
                }
                else if (length >= 2 && sTextBuffer[length - 2] == '/' && sTextBuffer[length - 1] == '*')
                {
                    commentEnd = '*';
                    length -= 2;
                    sTextBuffer.trim(length);
                }
                continue;
            }
        }
        if (src->read(&character, 1) == 0)
            return;
        sTextBuffer.append(character);
        multiByte = true;
        length += 2;
    }
}
unsigned char TextStreamFormat::readU8(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sMutex);
    unsigned char value = 0;
    getNextData_(src);
    StringUtil::tryParseU8(&value, sTextBuffer.cstr(), StringUtil::cAuto);
    return value;
}
unsigned short TextStreamFormat::readU16(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sMutex);
    unsigned short value = 0;
    getNextData_(src);
    StringUtil::tryParseU16(&value, sTextBuffer.cstr(), StringUtil::cAuto);
    return value;
}
unsigned int TextStreamFormat::readU32(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sMutex);
    unsigned int value = 0;
    getNextData_(src);
    StringUtil::tryParseU32(&value, sTextBuffer.cstr(), StringUtil::cAuto);
    return value;
}
unsigned long long TextStreamFormat::readU64(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sMutex);
    unsigned long long value = 0;
    getNextData_(src);
    StringUtil::tryParseU64(&value, sTextBuffer.cstr(), StringUtil::cAuto);
    return value;
}
signed char TextStreamFormat::readS8(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sMutex);
    signed char value = 0;
    getNextData_(src);
    StringUtil::tryParseS8(&value, sTextBuffer.cstr(), StringUtil::cAuto);
    return value;
}
short TextStreamFormat::readS16(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sMutex);
    short value = 0;
    getNextData_(src);
    StringUtil::tryParseS16(&value, sTextBuffer.cstr(), StringUtil::cAuto);
    return value;
}
int TextStreamFormat::readS32(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sMutex);
    int value = 0;
    getNextData_(src);
    StringUtil::tryParseS32(&value, sTextBuffer.cstr(), StringUtil::cAuto);
    return value;
}
long long TextStreamFormat::readS64(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sMutex);
    long long value = 0;
    getNextData_(src);
    StringUtil::tryParseS64(&value, sTextBuffer.cstr(), StringUtil::cAuto);
    return value;
}
float TextStreamFormat::readF32(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sMutex);
    float value = 0.0f;
    getNextData_(src);
    if (sTextBuffer.calcLength() != 0)
        ::sscanf(sTextBuffer.cstr(), "%f", &value);
    return value;
}
void TextStreamFormat::readBit(StreamSrc* src, void* data, unsigned int bitNum)
{
    ScopedLock<Mutex> lock(&sMutex);
    getNextData_(src);
    SafeStringBase<char> text(sTextBuffer.cstr());
    if (text.calcLength() >= 2 && text.at(0) == '0' && text.at(1) == 'b')
        text = SafeStringBase<char>(text.cstr() + 2);
    unsigned char* bytes = static_cast<unsigned char*>(data);
    unsigned char value = 0;
    unsigned int bit = 0;
    int length = text.calcLength();
    for (int index = 0; index != length + 1 && bit < bitNum; ++index)
    {
        value <<= 1;
        if (text.at(index) == '1')
            value |= 1;
        ++bit;
        if ((bit & 7) == 0)
        {
            bytes[(bit >> 3) - 1] = value;
            value = 0;
        }
    }
    if ((bit & 7) != 0)
        bytes[bit >> 3] = static_cast<unsigned char>((bytes[bit >> 3] & (0xff << (bit & 7))) | value);
}
void TextStreamFormat::readString(StreamSrc* src, BufferedSafeStringBase<char>* dst, unsigned int)
{
    ScopedLock<Mutex> lock(&sMutex);
    getNextData_(src);
    dst->copy(sTextBuffer);
}
void TextStreamFormat::readMemBlock(StreamSrc* src, void* dst, unsigned int size)
{
    ScopedLock<Mutex> lock(&sMutex);
    getNextData_(src);
    unsigned int decodedSize = 0;
    Base64::decode(dst, size, sTextBuffer.cstr(), sTextBuffer.calcLength(), &decodedSize);
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
void TextStreamFormat::writeMemBlock(StreamSrc* src, const void* data, unsigned int size)
{
    ScopedLock<Mutex> lock(&sMutex);
    sTextBuffer.clear();
    unsigned int encodedSize = (size / 3 + (size % 3 != 0)) * 4;
    if (encodedSize + 1 < sTextBuffer.getBufferSize())
    {
        char* buffer = sTextBuffer.getBuffer();
        buffer[encodedSize] = '\0';
        Base64::encode(buffer, data, size, false);
        src->write("\"", 1);
        src->write(sTextBuffer.cstr(), sTextBuffer.calcLength());
        src->write("\"", 1);
        src->write(mSeparator.cstr(), 1);
    }
}
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

int BitFlagUtil::findOnBitFromRight(unsigned int value, int ordinal)
{
    while (value != 0)
    {
        ordinal = static_cast<int>(static_cast<unsigned int>(ordinal) - 1u);
        if (ordinal <= 0)
            return countOnBit((value & (0u - value)) - 1u);
        value &= value - 1u;
    }
    return -1;
}

CoreId::CoreId() : mId(0) {}
unsigned int CoreInfo::sCoreCount;
unsigned int CoreInfo::sCoreIdMap[3];
unsigned int CoreInfo::sCoreIndexTable[3];

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

Heap* IDisposer::cDestructedFlag = reinterpret_cast<Heap*>(1);

IDisposer::IDisposer() : mDisposerHeap(0), mListNode()
{
    if (HeapMgr::instance())
    {
        mDisposerHeap = HeapMgr::instance()->findContainHeap(this);
        if (mDisposerHeap)
            mDisposerHeap->appendDisposer_(this);
    }
}

IDisposer::IDisposer(Heap* heap, HeapNullOption option) : mDisposerHeap(heap), mListNode()
{
    if (!mDisposerHeap)
    {
        switch (option)
        {
        case cHeapNullOption_AlwaysUseSpecifiedHeap:
        case cHeapNullOption_UseSpecifiedOrContainHeap:
            if (!HeapMgr::instance())
                return;
            mDisposerHeap = HeapMgr::instance()->findContainHeap(this);
            break;
        case cHeapNullOption_UseSpecifiedOrCurrentHeap:
            if (!HeapMgr::instance())
                return;
            mDisposerHeap = HeapMgr::instance()->getCurrentHeap();
            break;
        default:
            return;
        }
        if (!mDisposerHeap)
            return;
    }
    mDisposerHeap->appendDisposer_(this);
}

IDisposer::~IDisposer()
{
    if (mDisposerHeap != cDestructedFlag)
    {
        if (mDisposerHeap)
            mDisposerHeap->removeDisposer_(this);
        mDisposerHeap = cDestructedFlag;
    }
}

Heap::Heap(const SafeStringBase<char>& name, Heap* parent, void* start, unsigned int size, HeapDirection direction, bool enableLock)
    : IDisposer(parent, cHeapNullOption_UseSpecifiedOrContainHeap), mName(name), mStart(start), mSize(size), mParent(parent),
      mChildren(), mListNode(), mDisposerList(), mDirection(direction), mCS(parent), mFlag(cFlag_EnableWarning),
      mHeapCheckTag(HeapMgr::issueHeapCheckTag())
{
    if (enableLock)
        mFlag.set(cFlag_EnableLock);
    else
        mFlag.reset(cFlag_EnableLock);
    ConditionalScopedLock<CriticalSection> lock(&mCS, enableLock);
    mChildren.initOffset(offsetof(Heap, mListNode));
    mDisposerList.initOffset(offsetof(IDisposer, mListNode));
}

Heap::~Heap() {}

bool Heap::checkDerivedRuntimeTypeInfo(const RuntimeTypeInfo::Interface* type) const { return checkDerivedRuntimeTypeInfoStatic(type); }
const RuntimeTypeInfo::Interface* Heap::getRuntimeTypeInfo() const { return getRuntimeTypeInfoStatic(); }
bool ExpHeap::checkDerivedRuntimeTypeInfo(const RuntimeTypeInfo::Interface* type) const { return checkDerivedRuntimeTypeInfoStatic(type); }
const RuntimeTypeInfo::Interface* ExpHeap::getRuntimeTypeInfo() const { return getRuntimeTypeInfoStatic(); }
void* Heap::tryRealloc(void*, unsigned int, int) { return 0; }
void Heap::dump() const {}

void Heap::dumpYAML(WriteStream& stream, int indent) const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isEnableLock());
    FixedSafeString128 line(SafeStringBase<char>(""));
    line.append(' ', indent);
    line.appendWithFormat("- name: \"%s\"\n", mName.cstr());
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    line.appendWithFormat("  start_address: 0x%08X\n", getStartAddress());
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    line.appendWithFormat("  end_address: 0x%08X\n", getEndAddress());
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    if (!mParent)
        line.appendWithFormat("  parent: %s\n", "~");
    else
        line.appendWithFormat("  parent: %s\n", mParent->mName.cstr());
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    line.appendWithFormat("  direction: %s\n", mDirection == cHeapDirection_Forward ? "Forward" : "Reverse");
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    line.appendWithFormat("  size: %u\n", getSize());
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    line.appendWithFormat("  free_size: %u\n", getFreeSize());
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    line.appendWithFormat("  max_allocatable_size: %u\n", getMaxAllocatableSize(4));
    stream.writeDecorationText(line);
}

void Heap::pushBackChild_(Heap* child)
{
    ScopedLock<CriticalSection> treeLock(HeapMgr::getHeapTreeLockCS());
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    mChildren.pushBack(child);
}

void Heap::dumpTreeYAML(WriteStream& stream, int indent) const
{
    dumpYAML(stream, indent);
    FixedSafeString128 line(SafeStringBase<char>(""));
    line.append(' ', indent);
    line.appendWithFormat("  children:\n");
    stream.writeDecorationText(line);
    for (OffsetList<Heap>::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
        it->dumpTreeYAML(stream, indent + 4);
}

Heap* Heap::findContainHeap_(const void* address)
{
    if (!isInclude(address))
        return 0;
    for (OffsetList<Heap>::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        if (it->isInclude(address))
            return it->findContainHeap_(address);
    }
    return this;
}

void Heap::dispose_(const void* begin, const void* end)
{
    mFlag.set(cFlag_Disposing);
    OffsetList<IDisposer>::iterator it = mDisposerList.begin();
    while (it != mDisposerList.end())
    {
        IDisposer* disposer = &*it;
        if (disposer->mDisposerHeap && ((!begin && !end) || (begin <= disposer && disposer < end)))
        {
            disposer->~IDisposer();
            it = mDisposerList.begin();
        }
        else
        {
            ++it;
        }
    }
    mFlag.reset(cFlag_Disposing);
}

void Heap::eraseChild_(Heap* child)
{
    ScopedLock<CriticalSection> treeLock(HeapMgr::getHeapTreeLockCS());
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    mChildren.erase(child);
}

void Heap::destruct_()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    dispose_(0, 0);
    HeapMgr::removeFromFindContainHeapCache_(this);
    if (mParent)
        mParent->eraseChild_(this);
}

void Heap::appendDisposer_(IDisposer* disposer)
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    mDisposerList.pushBack(disposer);
}

void Heap::removeDisposer_(IDisposer* disposer)
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    mDisposerList.erase(disposer);
}

ExpHeap::ExpHeap(const SafeStringBase<char>& name, Heap* parent, void* start, unsigned int size, HeapDirection direction, bool enableLock)
    : Heap(name, parent, start, size, direction, enableLock), mAllocMode(cAllocMode_FirstFit), mFreeList(), mUseList()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    mFreeList.initOffset(offsetof(MemBlock, mListNode));
    mUseList.initOffset(offsetof(MemBlock, mListNode));
}

ExpHeap::~ExpHeap()
{
    destruct_();
}

ExpHeap* ExpHeap::tryCreate(unsigned int size, const SafeStringBase<char>& name, Heap* parent, HeapDirection direction, bool enableLock)
{
    if (!parent)
    {
        parent = HeapMgr::instance()->getCurrentHeap();
        if (!parent)
            return 0;
    }
    if (size == 0)
        size = parent->getMaxAllocatableSize(4) & ~3u;
    else
        size = (size + 3) & ~3u;
    if (size <= sizeof(ExpHeap) + sizeof(MemBlock))
        return 0;
    unsigned char* start = static_cast<unsigned char*>(parent->tryAlloc(size, direction * 4));
    if (!start)
        return 0;
    if (parent->mDirection == cHeapDirection_Reverse)
        direction = static_cast<HeapDirection>(-direction);
    ExpHeap* heap;
    if (direction == cHeapDirection_Forward)
        heap = new (start) ExpHeap(name, parent, start, size, cHeapDirection_Forward, enableLock);
    else
        heap = new (start + size - sizeof(ExpHeap)) ExpHeap(name, parent, start, size, direction, enableLock);
    doCreate(heap, parent);
    return heap;
}

ExpHeap* ExpHeap::create(unsigned int size, const SafeStringBase<char>& name, Heap* parent, HeapDirection direction, bool enableLock)
{
    return tryCreate(size, name, parent, direction, enableLock);
}

ExpHeap* ExpHeap::tryCreate(void* address, unsigned int size, const SafeStringBase<char>& name, bool enableLock)
{
    size &= ~3u;
    if (size <= sizeof(ExpHeap) + sizeof(MemBlock))
        return 0;
    ExpHeap* heap = new (address) ExpHeap(name, 0, address, size, cHeapDirection_Forward, enableLock);
    doCreate(heap, 0);
    return heap;
}

void ExpHeap::pushToFreeList_(MemBlock* block)
{
    MemBlock* previous = 0;
    for (OffsetList<MemBlock>::iterator it = mFreeList.begin(); it != mFreeList.end(); ++it)
    {
        MemBlock* next = &*it;
        if (block < next)
        {
            bool merged = false;
            if (previous && previous->memory() + previous->mSize == reinterpret_cast<unsigned char*>(block))
            {
                previous->mSize += block->mOffset + block->mSize + sizeof(MemBlock);
                block = previous;
                merged = true;
            }
            if (block->memory() + block->mSize == reinterpret_cast<unsigned char*>(next))
            {
                if (!merged)
                    mFreeList.insertBefore(next, block);
                block->mSize += next->mOffset + next->mSize + sizeof(MemBlock);
                mFreeList.erase(next);
            }
            else if (!merged)
            {
                mFreeList.insertBefore(next, block);
            }
            return;
        }
        previous = next;
    }
    if (mFreeList.size() != 0 && previous->memory() + previous->mSize == reinterpret_cast<unsigned char*>(block))
    {
        previous->mSize += block->mOffset + block->mSize + sizeof(MemBlock);
        return;
    }
    mFreeList.pushBack(block);
}

void ExpHeap::createMaxSizeFreeMemBlock_(ExpHeap* heap)
{
    ConditionalScopedLock<CriticalSection> lock(&heap->mCS, heap->isEnableLock());
    void* address = heap->mDirection == cHeapDirection_Forward ? static_cast<void*>(heap + 1) : heap->mStart;
    MemBlock* block = new (address) MemBlock();
    block->mSize = heap->mSize - sizeof(ExpHeap) - sizeof(MemBlock);
    heap->pushToFreeList_(block);
}

void ExpHeap::doCreate(ExpHeap* heap, Heap* parent)
{
    createMaxSizeFreeMemBlock_(heap);
    if (parent)
        parent->pushBackChild_(heap);
}

int ExpHeap::compareMemBlockAddr_(const MemBlock* first, const MemBlock* second)
{
    if (reinterpret_cast<const char*>(first) - reinterpret_cast<const char*>(second) < 0)
        return -1;
    return 1;
}

MemBlock* ExpHeap::findLastMemBlockIfFree_()
{
    mUseList.sort(compareMemBlockAddr_);
    MemBlock* lastFree = mFreeList.back();
    MemBlock* lastUse = mUseList.back();
    if (!lastUse)
    {
        if (lastFree)
            return lastFree;
    }
    else if (lastUse < lastFree)
    {
        return lastFree;
    }
    return 0;
}

unsigned int ExpHeap::adjustBack_()
{
    MemBlock* block = findLastMemBlockIfFree_();
    if (block)
    {
        unsigned int size = reinterpret_cast<unsigned char*>(block) - static_cast<unsigned char*>(mStart);
        mFreeList.erase(block);
        if (mParent->resizeBack(mStart, size))
        {
            mSize = size;
            return size;
        }
    }
    return mSize;
}

MemBlock* ExpHeap::findFirstMemBlockIfFree_()
{
    mUseList.sort(compareMemBlockAddr_);
    MemBlock* firstFree = mFreeList.front();
    MemBlock* firstUse = mUseList.front();
    if (firstUse && firstUse <= firstFree)
        return 0;
    return firstFree;
}

unsigned int ExpHeap::adjustFront_()
{
    MemBlock* block = findFirstMemBlockIfFree_();
    if (block)
    {
        unsigned int size = mSize - (block->mOffset + block->mSize + sizeof(MemBlock));
        mFreeList.erase(block);
        void* start = mParent->resizeFront(mStart, size);
        if (start)
        {
            mSize = size;
            mStart = start;
            return size;
        }
    }
    return mSize;
}

unsigned int ExpHeap::adjust()
{
    if (!mParent)
        return mSize;
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    ConditionalScopedLock<CriticalSection> parentLock(&mParent->mCS, mParent->isEnableLock());
    if (mDirection == cHeapDirection_Forward)
        return adjustBack_();
    return adjustFront_();
}

MemBlock* ExpHeap::findFreeMemBlockFromHead_(unsigned int size, FindMode mode) const
{
    MemBlock* found = 0;
    for (OffsetList<MemBlock>::iterator it = mFreeList.begin(); it != mFreeList.end(); ++it)
    {
        unsigned int blockSize = it->mSize;
        if (size <= blockSize)
        {
            if (mode == cFindMode_FirstFit)
                return &*it;
            if (!found || (mode == cFindMode_BestFit && blockSize < found->mSize) ||
                (mode == cFindMode_MaxSize && found->mSize < blockSize))
                found = &*it;
        }
    }
    return found;
}

MemBlock* ExpHeap::allocFromHead_(unsigned int size)
{
    MemBlock* block = findFreeMemBlockFromHead_(size, static_cast<FindMode>(mAllocMode));
    if (block)
    {
        unsigned short offset = block->mOffset;
        unsigned int blockSize = block->mSize;
        block->mSize = size;
        unsigned int remainSize = blockSize - size;
        mFreeList.erase(block);
        pushToUseList_(block);
        if (remainSize > sizeof(MemBlock))
        {
            MemBlock* freeBlock = new (reinterpret_cast<unsigned char*>(block) + offset + sizeof(MemBlock) + size) MemBlock(remainSize - sizeof(MemBlock));
            pushToFreeList_(freeBlock);
        }
        else if (remainSize != 0)
        {
            block->mSize = size + remainSize;
        }
    }
    return block;
}

MemBlock* ExpHeap::findFreeMemBlockFromHead_(unsigned int size, int alignment, FindMode mode) const
{
    MemBlock* found = 0;
    for (OffsetList<MemBlock>::iterator it = mFreeList.begin(); it != mFreeList.end(); ++it)
    {
        unsigned int blockSize = it->mSize;
        if (size <= blockSize)
        {
            uintptr_t memory = reinterpret_cast<uintptr_t>(it->memory());
            uintptr_t aligned = (memory + alignment - 1) & ~static_cast<uintptr_t>(alignment - 1);
            if (size + (aligned - memory) <= blockSize)
            {
                if (mode == cFindMode_FirstFit)
                    return &*it;
                if (!found || (mode == cFindMode_BestFit && blockSize < found->mSize) ||
                    (mode == cFindMode_MaxSize && found->mSize < blockSize))
                    found = &*it;
            }
        }
    }
    return found;
}

MemBlock* ExpHeap::allocFromHead_(unsigned int size, int alignment)
{
    MemBlock* block = findFreeMemBlockFromHead_(size, alignment, static_cast<FindMode>(mAllocMode));
    if (block)
    {
        uintptr_t memory = reinterpret_cast<uintptr_t>(block->memory());
        unsigned int padding = ((memory + alignment - 1) & ~static_cast<uintptr_t>(alignment - 1)) - memory;
        unsigned int remainSize = block->mSize - (size + padding);
        if (padding < 0x10000)
        {
            block->setOffset(padding);
            block->mSize = size;
            mFreeList.erase(block);
            pushToUseList_(block);
        }
        else
        {
            block->mOffset = 0;
            MemBlock* useBlock = reinterpret_cast<MemBlock*>(reinterpret_cast<unsigned char*>(block) + padding);
            block->mSize = padding - sizeof(MemBlock);
            useBlock = new (useBlock) MemBlock();
            useBlock->mOffset = 0;
            useBlock->mSize = size;
            pushToUseList_(useBlock);
            block = useBlock;
        }
        if (remainSize > sizeof(MemBlock))
        {
            MemBlock* freeBlock = new (block->memory() + size) MemBlock(remainSize - sizeof(MemBlock));
            pushToFreeList_(freeBlock);
        }
        else if (remainSize != 0)
        {
            block->mSize = size + remainSize;
        }
    }
    return block;
}

MemBlock* ExpHeap::findFreeMemBlockFromTail_(unsigned int size, FindMode mode) const
{
    MemBlock* found = 0;
    for (MemBlock* block = mFreeList.back(); block; block = mFreeList.prev(block))
    {
        unsigned int blockSize = block->mSize;
        if (size <= blockSize)
        {
            if (mode == cFindMode_FirstFit)
                return block;
            if (!found || (mode == cFindMode_BestFit && blockSize < found->mSize) ||
                (mode == cFindMode_MaxSize && found->mSize < blockSize))
                found = block;
        }
    }
    return found;
}

MemBlock* ExpHeap::allocFromTail_(unsigned int size)
{
    MemBlock* block = findFreeMemBlockFromTail_(size, static_cast<FindMode>(mAllocMode));
    if (!block)
        return 0;
    if (block->mSize - size > sizeof(MemBlock))
    {
        unsigned int freeSize = block->mSize - size - sizeof(MemBlock);
        unsigned char* address = block->memory() + freeSize;
        block->mSize = freeSize;
        MemBlock* useBlock = new (address) MemBlock(size);
        pushToUseList_(useBlock);
        return useBlock;
    }
    mFreeList.erase(block);
    pushToUseList_(block);
    return block;
}

MemBlock* ExpHeap::findFreeMemBlockFromTail_(unsigned int size, int alignment, FindMode mode) const
{
    MemBlock* found = 0;
    for (MemBlock* block = mFreeList.back(); block; block = mFreeList.prev(block))
    {
        unsigned int blockSize = block->mSize;
        if (size <= blockSize)
        {
            uintptr_t address = reinterpret_cast<uintptr_t>(block->memory()) + (blockSize - size);
            if (size + (address - (address & ~static_cast<uintptr_t>(alignment - 1))) <= blockSize)
            {
                if (mode == cFindMode_FirstFit)
                    return block;
                if (!found || (mode == cFindMode_BestFit && blockSize < found->mSize) ||
                    (mode == cFindMode_MaxSize && found->mSize < blockSize))
                    found = block;
            }
        }
    }
    return found;
}

MemBlock* ExpHeap::allocFromTail_(unsigned int size, int alignment)
{
    MemBlock* block = findFreeMemBlockFromTail_(size, alignment, static_cast<FindMode>(mAllocMode));
    if (!block)
        return 0;
    uintptr_t address = reinterpret_cast<uintptr_t>(block->memory()) + (block->mSize - size);
    unsigned int useSize = size + (address - (address & ~static_cast<uintptr_t>(alignment - 1)));
    unsigned int remainSize = block->mSize - useSize;
    if (remainSize > sizeof(MemBlock))
    {
        unsigned int freeSize = remainSize - sizeof(MemBlock);
        unsigned char* useAddress = block->memory() + freeSize;
        block->mSize = freeSize;
        MemBlock* useBlock = new (useAddress) MemBlock(useSize);
        pushToUseList_(useBlock);
        return useBlock;
    }
    block->setOffset(remainSize);
    block->mSize = useSize;
    mFreeList.erase(block);
    pushToUseList_(block);
    return block;
}

void ExpHeap::pushToUseList_(MemBlock* block)
{
    if (mDirection == cHeapDirection_Forward)
        mUseList.pushBack(block);
    else
        mUseList.pushFront(block);
}

void* ExpHeap::realloc_(void* address, unsigned char* memory, unsigned int copySize, unsigned int size, int alignment)
{
    void* newAddress = tryAlloc(size, alignment);
    if (newAddress)
    {
        MemUtil::copy(newAddress, memory, copySize);
        free(address);
    }
    return newAddress;
}

void ExpHeap::destroy()
{
    Heap* parent = mParent;
    void* start = mStart;
    this->~ExpHeap();
    if (parent && parent->isFreeable())
        parent->free(start);
}

void* ExpHeap::tryAlloc(unsigned int size, int alignment)
{
    unsigned int allocSize = size < 4 ? 4 : size;
    if (allocSize >= mSize)
    {
        if (HeapMgr::instance() && HeapMgr::instance()->getAllocFailedCallback())
        {
            HeapMgr::AllocFailedCallbackArg arg;
            arg.heap = this;
            arg.requestSize = size;
            arg.requestAlignment = alignment;
            arg.allocSize = allocSize;
            arg.allocAlignment = alignment;
            HeapMgr::instance()->getAllocFailedCallback()->invoke(&arg);
        }
        return 0;
    }
    unsigned int absAlignment = alignment < 0 ? -alignment : alignment;
    if (((absAlignment - 1) & absAlignment) != 0)
    {
        if (HeapMgr::instance() && HeapMgr::instance()->getAllocFailedCallback())
        {
            HeapMgr::AllocFailedCallbackArg arg;
            arg.heap = this;
            arg.requestSize = size;
            arg.requestAlignment = alignment;
            arg.allocSize = allocSize;
            arg.allocAlignment = alignment;
            HeapMgr::instance()->getAllocFailedCallback()->invoke(&arg);
        }
        return 0;
    }
    allocSize = (allocSize + 3) & ~3u;
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    int directedAlignment = alignment * mDirection;
    MemBlock* block;
    if (directedAlignment < 0)
    {
        directedAlignment = -directedAlignment;
        if (directedAlignment < 5)
            block = allocFromTail_(allocSize);
        else
            block = allocFromTail_(allocSize, directedAlignment);
    }
    else if (directedAlignment < 5)
    {
        block = allocFromHead_(allocSize);
    }
    else
    {
        block = allocFromHead_(allocSize, directedAlignment);
    }
    if (!block)
    {
        if (HeapMgr::instance() && HeapMgr::instance()->getAllocFailedCallback())
        {
            HeapMgr::AllocFailedCallbackArg arg;
            arg.heap = this;
            arg.requestSize = size;
            arg.requestAlignment = alignment;
            arg.allocSize = allocSize;
            arg.allocAlignment = directedAlignment;
            HeapMgr::instance()->getAllocFailedCallback()->invoke(&arg);
        }
        return 0;
    }
    block->mMagic = mHeapCheckTag;
    return block->memory();
}

void ExpHeap::dumpUseList() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isEnableLock());
}

void ExpHeap::dumpFreeList() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isEnableLock());
}

void ExpHeap::free(void* address)
{
    if (!address)
        return;
    if (!isInclude(address))
        return;
    if (mFlag.isOn(cFlag_Disposing))
        return;
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    MemBlock* block = MemBlock::findManageArea(address);
    if (!block)
    {
        dumpUseList();
        dumpFreeList();
    }
    else if (block->mMagic == mHeapCheckTag)
    {
        mUseList.erase(block);
        unsigned short offset = block->mOffset;
        block->mOffset = 0;
        block->mSize += offset;
        pushToFreeList_(block);
    }
}

void* ExpHeap::resizeFront(void* address, unsigned int size)
{
    if (!isInclude(address))
        return 0;
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    MemBlock* block = MemBlock::findManageArea(address);
    unsigned int blockSize = block->mSize;
    size = (size + 3) & ~3u;
    if (size < blockSize)
    {
        unsigned int span = block->mOffset + blockSize;
        unsigned int freeSpan = span + sizeof(MemBlock) - size;
        if (freeSpan - sizeof(MemBlock) >= sizeof(MemBlock))
        {
            MemBlock* useBlock = new (reinterpret_cast<unsigned char*>(block) + (freeSpan - sizeof(MemBlock))) MemBlock();
            useBlock->mMagic = mHeapCheckTag;
            useBlock->mSize = size;
            pushToUseList_(useBlock);
            mUseList.erase(block);
            block->mOffset = 0;
            block->mSize = freeSpan - 2 * sizeof(MemBlock);
            pushToFreeList_(block);
            return useBlock->memory();
        }
        block->setOffset(span - size);
        return block->memory();
    }
    if (size <= blockSize)
        return block->memory();
    return 0;
}

void* ExpHeap::resizeBack(void* address, unsigned int size)
{
    if (!isInclude(address))
        return 0;
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    MemBlock* block = MemBlock::findManageArea(address);
    unsigned int blockSize = block->mSize;
    size = (size + 3) & ~3u;
    if (size < blockSize)
    {
        unsigned int remainSize = blockSize - size;
        if (remainSize >= sizeof(MemBlock))
        {
            unsigned char* freeAddress = block->memory() + size;
            block->mSize = size;
            MemBlock* freeBlock = new (freeAddress) MemBlock(remainSize - sizeof(MemBlock));
            pushToFreeList_(freeBlock);
        }
        return block->memory();
    }
    if (size <= blockSize)
        return block->memory();
    return 0;
}

void* ExpHeap::tryRealloc(void* address, unsigned int size, int alignment)
{
    if (!address)
        return tryAlloc(size, alignment);
    if (size == 0)
    {
        free(address);
        return 0;
    }
    if (!isInclude(address) || alignment < 0)
        return 0;
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    MemBlock* block = MemBlock::findManageArea(address);
    unsigned int blockSize = block->mSize;
    size = (size + 3) & ~3u;
    if (blockSize <= size)
    {
        if (blockSize < size)
        {
            if (alignment == 0)
                alignment = 4;
            return realloc_(address, block->memory(), blockSize, size, alignment);
        }
        unsigned char* memory = block->memory();
        if (alignment != 0 && (reinterpret_cast<uintptr_t>(memory) & (alignment - 1)) != 0)
            return realloc_(address, memory, blockSize, size, alignment);
        return memory;
    }
    if (alignment != 0)
    {
        unsigned char* memory = block->memory();
        if ((reinterpret_cast<uintptr_t>(memory) & (alignment - 1)) != 0)
            return realloc_(address, memory, size, size, alignment);
    }
    unsigned int remainSize = blockSize - size;
    if (remainSize >= sizeof(MemBlock))
    {
        unsigned char* freeAddress = block->memory() + size;
        block->mSize = size;
        MemBlock* freeBlock = new (freeAddress) MemBlock(remainSize - sizeof(MemBlock));
        pushToFreeList_(freeBlock);
    }
    return block->memory();
}

void ExpHeap::freeAll()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isEnableLock());
    dispose_(0, 0);
    mUseList.clear();
    mFreeList.clear();
    createMaxSizeFreeMemBlock_(this);
}

void* ExpHeap::getStartAddress() const { return mStart; }
void* ExpHeap::getEndAddress() const { return static_cast<unsigned char*>(mStart) + mSize; }

unsigned int ExpHeap::getFreeSize() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isEnableLock());
    unsigned int freeSize = 0;
    for (OffsetList<MemBlock>::iterator it = mFreeList.begin(); it != mFreeList.end(); ++it)
        freeSize += it->mSize;
    return freeSize;
}

unsigned int ExpHeap::getSize() const { return mSize; }

unsigned int ExpHeap::getMaxAllocatableSize(int alignment) const
{
    unsigned int absAlignment = alignment < 0 ? -alignment : alignment;
    if (((absAlignment - 1) & absAlignment) != 0)
        return 0;
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isEnableLock());
    int directedAlignment = alignment * mDirection;
    MemBlock* block;
    if (directedAlignment < 0)
    {
        directedAlignment = -directedAlignment;
        if (directedAlignment > 4)
        {
            block = findFreeMemBlockFromTail_(4, directedAlignment, cFindMode_MaxSize);
            if (!block)
                return 0;
            uintptr_t address = reinterpret_cast<uintptr_t>(block->memory()) + block->mSize - 4;
            return block->mSize - (address - (address & ~static_cast<uintptr_t>(directedAlignment - 1)));
        }
        block = findFreeMemBlockFromTail_(4, cFindMode_MaxSize);
    }
    else
    {
        if (directedAlignment > 4)
        {
            block = findFreeMemBlockFromHead_(4, directedAlignment, cFindMode_MaxSize);
            if (!block)
                return 0;
            uintptr_t memory = reinterpret_cast<uintptr_t>(block->memory());
            return block->mSize - (((memory + directedAlignment - 1) & ~static_cast<uintptr_t>(directedAlignment - 1)) - memory);
        }
        block = findFreeMemBlockFromHead_(4, cFindMode_MaxSize);
    }
    if (block)
        return block->mSize;
    return 0;
}

bool ExpHeap::isInclude(const void* address) const
{
    const unsigned char* start = static_cast<const unsigned char*>(mStart);
    unsigned int size = mSize;
    const unsigned char* begin;
    if (mDirection == cHeapDirection_Forward)
    {
        begin = start + sizeof(ExpHeap);
    }
    else
    {
        size -= sizeof(ExpHeap);
        begin = start;
    }
    return begin <= address && address < start + size;
}

bool ExpHeap::isEmpty() const { return mUseList.size() == 0; }
bool ExpHeap::isFreeable() const { return true; }
bool ExpHeap::isResizable() const { return true; }
bool ExpHeap::isAdjustable() const { return true; }

void ExpHeap::dump() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isEnableLock());
    dumpUseList();
    dumpFreeList();
}

unsigned int ExpHeap::getAllocatedSize(void* address)
{
    if (!isInclude(address))
        return 0;
    return MemBlock::findManageArea(address)->mSize;
}

void ExpHeap::dumpYAML(WriteStream& stream, int indent) const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isEnableLock());
    Heap::dumpYAML(stream, indent);
    FixedSafeString128 line(SafeStringBase<char>(""));
    line.append(' ', indent);
    line.appendWithFormat("  heap_type: ExpHeap\n");
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    line.appendWithFormat("  alloc_mode: %s\n", getAllocMode() == cAllocMode_FirstFit ? "First Fit" : "Best Fit");
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    line.appendWithFormat("  use_list_size: %d\n", mUseList.size());
    stream.writeDecorationText(line);
    line.clear();
    line.append(' ', indent);
    line.appendWithFormat("  free_list_size: %d\n", mFreeList.size());
    stream.writeDecorationText(line);
}

void ExpHeap::setAllocMode(AllocMode mode) { mAllocMode = mode; }
ExpHeap::AllocMode ExpHeap::getAllocMode() const { return mAllocMode; }

FindContainHeapCache::FindContainHeapCache() : mHeap(0) {}

bool FindContainHeapCache::tryRemoveHeap(Heap* heap)
{
    unsigned int original;
    if (mHeap.compareExchange(reinterpret_cast<uintptr_t>(heap), 0, &original))
        return true;
    return (original & ~1u) != reinterpret_cast<uintptr_t>(heap);
}

Arena* HeapMgr::sArena = 0;
HeapMgr HeapMgr::sInstance;
HeapMgr* HeapMgr::sInstancePtr = 0;
AtomicU32 HeapMgr::sHeapCheckTag;
HeapMgr::RootHeaps HeapMgr::sRootHeaps;
CriticalSection HeapMgr::sHeapTreeLockCS;
HeapMgr::IndependentHeaps HeapMgr::sIndependentHeaps;

HeapMgr::HeapMgr() : mAllocFailedCallback(0) {}
HeapMgr::~HeapMgr() {}

void HeapMgr::createRootHeap_()
{
    sRootHeaps.pushBack(ExpHeap::tryCreate(sArena->mStart, sArena->mSize, SafeStringBase<char>("RootHeap"), false));
}

void HeapMgr::initializeImpl_()
{
    sInstance.mAllocFailedCallback = 0;
    createRootHeap_();
    sInstancePtr = &sInstance;
}

void HeapMgr::initialize(Arena* arena)
{
    sArena = arena;
    initializeImpl_();
}

void HeapMgr::destroy()
{
    sHeapTreeLockCS.lock();
    sInstance.mAllocFailedCallback = 0;
    while (!sIndependentHeaps.isEmpty())
    {
        sIndependentHeaps.back()->destroy();
        sIndependentHeaps.popBack();
    }
    while (!sRootHeaps.isEmpty())
    {
        sRootHeaps.back()->destroy();
        sRootHeaps.popBack();
    }
    sInstancePtr = 0;
    sArena->mSize = 0;
    sArena->mStart = 0;
    sArena = 0;
    sHeapTreeLockCS.unlock();
}

Heap* HeapMgr::findContainHeap(const void* address) const
{
    FindContainHeapCache* cache = 0;
    if (ThreadMgr::instance())
    {
        Thread* thread = ThreadMgr::instance()->getCurrentThread();
        if (thread)
        {
            cache = thread->getFindContainHeapCache();
            if (cache)
            {
                Heap* cached = reinterpret_cast<Heap*>(cache->mHeap.fetchOr(1));
                if (cached && cached->mChildren.size() == 0 && cached->isInclude(address))
                {
                    cache->mHeap.fetchAnd(~1u);
                    return cached;
                }
                cache->mHeap.fetchAnd(~1u);
            }
        }
    }
    ScopedLock<CriticalSection> lock(&sHeapTreeLockCS);
    if (cache)
    {
        Heap* cached = reinterpret_cast<Heap*>(cache->mHeap.load());
        if (cached)
        {
            Heap* heap = cached->findContainHeap_(address);
            if (heap)
            {
                if (heap != cached)
                    cache->mHeap.store(reinterpret_cast<uintptr_t>(heap));
                return heap;
            }
        }
    }
    for (Heap** it = sRootHeaps.begin(); it != sRootHeaps.end(); ++it)
    {
        Heap* heap = (*it)->findContainHeap_(address);
        if (heap)
        {
            if (cache)
                cache->mHeap.store(reinterpret_cast<uintptr_t>(heap));
            return heap;
        }
    }
    for (Heap** it = sIndependentHeaps.begin(); it != sIndependentHeaps.end(); ++it)
    {
        Heap* heap = (*it)->findContainHeap_(address);
        if (heap)
        {
            if (cache)
                cache->mHeap.store(reinterpret_cast<uintptr_t>(heap));
            return heap;
        }
    }
    return 0;
}

Heap* HeapMgr::findHeapByName_(Heap* heap, const SafeStringBase<char>& name, int* index)
{
    if (heap->mName == name)
    {
        if (*index == 0)
            return heap;
        --*index;
    }
    for (OffsetList<Heap>::iterator it = heap->mChildren.begin(); it != heap->mChildren.end(); ++it)
    {
        Heap* found = findHeapByName_(&*it, name, index);
        if (found)
            return found;
    }
    return 0;
}

Heap* HeapMgr::getCurrentHeap() const
{
    return ThreadMgr::instance()->getCurrentThread()->getCurrentHeap();
}

bool HeapMgr::isContainedInAnyHeap(const void* address)
{
    for (Heap** it = sRootHeaps.begin(); it != sRootHeaps.end(); ++it)
    {
        if ((*it)->isInclude(address))
            return true;
    }
    for (Heap** it = sIndependentHeaps.begin(); it != sIndependentHeaps.end(); ++it)
    {
        if ((*it)->isInclude(address))
            return true;
    }
    return false;
}

void* allocateFromSeadHeap(unsigned int size, Heap* heap)
{
    if (!heap)
    {
        heap = HeapMgr::instance()->getCurrentHeap();
        if (!heap)
            return 0;
    }
    void* address = heap->tryAlloc(size, 4);
    if (!address)
        return 0;
    return address;
}

void* allocateFromSeadHeap(unsigned int size)
{
    Heap* heap = HeapMgr::instance()->getCurrentHeap();
    if (!heap)
        return 0;
    void* address = heap->tryAlloc(size, 4);
    if (!address)
        return 0;
    return address;
}

void* allocateFromSeadHeap(unsigned int size, Heap* heap, int alignment)
{
    if (!heap)
    {
        heap = HeapMgr::instance()->getCurrentHeap();
        if (!heap)
            return 0;
    }
    void* address = heap->tryAlloc(size, alignment);
    if (!address)
        return 0;
    return address;
}

void freeToSeadHeap(void* address)
{
    Heap* heap = HeapMgr::instance()->findContainHeap(address);
    if (heap)
        heap->free(address);
}

unsigned int getArraySize(void* address)
{
    return MemBlock::findManageArea(address)->mSize;
}

CriticalSection::CriticalSection() : IDisposer()
{
    memset(mStorage, 0, sizeof(mStorage));
    OSInitMutexEx(reinterpret_cast<OSMutex*>(mStorage), 0);
}
CriticalSection::CriticalSection(Heap* heap) : IDisposer(heap, cHeapNullOption_UseSpecifiedOrContainHeap)
{
    memset(mStorage, 0, sizeof(mStorage));
    OSInitMutexEx(reinterpret_cast<OSMutex*>(mStorage), 0);
}
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

Thread::Thread(Heap* heap, OSThread* thread)
    : IDisposer(), mName("pead::MainThread"), mMessageQueue(), mStackSize(0), mListNode(), mCurrentHeap(0),
      mFindContainHeapCache(), mBlockType(MessageQueue::cNoBlock), mQuitMsg(cDefaultQuitMsg),
      mId(reinterpret_cast<uintptr_t>(thread)), mState(cState_Running), mAffinity(1), mThread(thread), mStack(0),
      mPriority(OSGetThreadPriority(thread))
{
    mListNode.mData = this;
    mMessageQueue.allocate(cDefaultMessageQueueSize, heap);
    mStack = static_cast<unsigned char*>(thread->stackEnd);
    mStackSize = static_cast<unsigned char*>(thread->stackStart) - mStack;
}
Thread::Thread(const SafeStringBase<char>& name, Heap* heap, int priority, MessageQueue::BlockType blockType, int quitMsg, int stackSize, int messageQueueSize)
    : IDisposer(), mName(name), mMessageQueue(), mStackSize(stackSize), mListNode(), mCurrentHeap(0),
      mFindContainHeapCache(), mBlockType(blockType), mQuitMsg(quitMsg), mId(0), mState(cState_Initialized),
      mAffinity(1), mStack(0), mPriority(priority)
{
    mListNode.mData = this;
    mMessageQueue.allocate(messageQueueSize, heap);
    mStack = NewArray<unsigned char>(mStackSize, heap);
    mThread = new (allocateFromSeadHeap(sizeof(OSThread), heap, cThreadAlignment)) OSThread();
    mThread->stackEnd = reinterpret_cast<void*>(cUncreatedStackEnd);
    mThread->stackStart = 0;
    mId = reinterpret_cast<uintptr_t>(mThread);
    ThreadMgr* threadMgr = ThreadMgr::instance();
    if (threadMgr)
    {
        ScopedLock<CriticalSection> lock(&threadMgr->mListCS);
        threadMgr->mList.pushBack(&mListNode);
    }
}
Thread::~Thread()
{
    ThreadMgr* threadMgr = ThreadMgr::instance();
    if (threadMgr)
    {
        if (this != threadMgr->mMainThread)
        {
            {
                ScopedLock<CriticalSection> lock(&threadMgr->mListCS);
                mListNode.erase();
            }
            if (mState == cState_Running)
                quitAndWaitDoneSingleThread(false);
            else if (mState == cState_Quitting || mState == cState_Terminated)
                waitDone();
            if (mThread)
                freeToSeadHeap(mThread);
            DeleteArray(mStack);
        }
        mMessageQueue.free();
    }
}
void Thread::waitDone() { if (mState != cState_Initialized && mState != cState_Released) { OSJoinThread(mThread, 0); mState = cState_Released; } }
int Thread::cafeThreadFunc_(int, void* context)
{
    Thread* thread = static_cast<Thread*>(context);
    thread->run_();
    thread->mState = cState_Terminated;
    return 0;
}
unsigned int Thread::start()
{
    if (mState != cState_Initialized || !mThread || !mStack || mStackSize <= 0) return 0;
    if (!C4JThreadImpl::OSCreateThread4J(mThread, cafeThreadFunc_, 0, this,
                                         mStack + mStackSize,
                                         static_cast<unsigned int>(mStackSize),
                                         static_cast<unsigned int>(mPriority), 0)) return 0;
    OSSetThreadName(mThread, mName.cstr());
    unsigned short platformMask = 0;
    for (unsigned int id = 0; id < 3; ++id)
        if (mAffinity.value() & (1u << id)) platformMask = static_cast<unsigned short>(platformMask | CoreInfo::getPlatformMask(CoreId(static_cast<int>(id))));
    OSSetThreadAffinity(mThread, platformMask);
    mState = cState_Running;
    mId = reinterpret_cast<uintptr_t>(mThread);
    OSResumeThread(mThread);
    return 1;
}
void Thread::sleep(TickSpan span) { OSSleepTicks(span.getTicks()); }
void Thread::yield() { OSYieldThread(); }
void Thread::setPriority(int priority) { mPriority = priority; if (mThread) OSSetThreadPriority(mThread, priority); }
int Thread::getPriority() const { return mThread ? OSGetThreadPriority(mThread) : mPriority; }
void Thread::setAffinity(const CoreIdMask& mask)
{
    mAffinity = mask;
    unsigned short platformMask = 0;
    for (unsigned int id = 0; id < 3; ++id)
        if (mAffinity.value() & (1u << id)) platformMask = static_cast<unsigned short>(platformMask | CoreInfo::getPlatformMask(CoreId(static_cast<int>(id))));
    if (mThread) OSSetThreadAffinity(mThread, platformMask);
}
void* Thread::getStackCheckStartAddress_() const { return mStack ? mStack + 4 : 0; }
void Thread::destroy() { waitDone(); }
MessageQueue* Thread::getMessageQueue() const { return const_cast<MessageQueue*>(&mMessageQueue); }
bool Thread::sendMessage(int message, MessageQueue::BlockType block) { if (!message || mState == cState_Terminated || mState == cState_Released || message == mQuitMsg) return false; return mMessageQueue.push(message, block); }
int Thread::recvMessage(MessageQueue::BlockType block) { if (mState == cState_Quitting) return 0; return mMessageQueue.pop(block); }
void Thread::quit(bool jam) { mState = cState_Quitting; if (jam) mMessageQueue.jam(0, mBlockType); else mMessageQueue.push(0, mBlockType); }
void Thread::quitAndDestroySingleThread(bool jam) { quitAndWaitDoneSingleThread(jam); destroy(); }
void Thread::quitAndWaitDoneSingleThread(bool jam) { quit(jam); waitDone(); }
MessageQueue::BlockType Thread::getBlockType() const { return mBlockType; }
int Thread::getStackSize() const { return mStackSize; }
void Thread::run_() { int message; while ((message = recvMessage(mBlockType)) != mQuitMsg) calc_(message); }
int Thread::calcStackUsedSizePeak() const { return 0; }

MainThread::~MainThread() { mState = cState_Terminated; }
void MainThread::destroy() {}
void MainThread::quit(bool) {}
void MainThread::waitDone() {}
void MainThread::quitAndDestroySingleThread(bool) {}
void MainThread::setPriority(int) {}
void MainThread::calc_(int) {}

DelegateThread::DelegateThread(const SafeStringBase<char>& name, IDelegate2<Thread*, int>* delegate, Heap* heap, int priority, MessageQueue::BlockType block, int quitMsg, int stackSize, int messageQueueSize)
    : Thread(name, heap, priority, block, quitMsg, stackSize, messageQueueSize), mDelegate(delegate) {}
DelegateThread::~DelegateThread() {}
void DelegateThread::calc_(int message) { if (mDelegate) mDelegate->invoke(this, message); }

ThreadMgr* ThreadMgr::sInstance = 0;
ThreadMgr::SingletonDisposer_* ThreadMgr::SingletonDisposer_::sStaticDisposer = 0;

ThreadMgr::SingletonDisposer_::~SingletonDisposer_()
{
    if (this == sStaticDisposer)
    {
        sStaticDisposer = 0;
        sInstance->~ThreadMgr();
        sInstance = 0;
    }
}

void HeapMgr::removeFromFindContainHeapCache_(Heap* heap)
{
    ThreadMgr* threadMgr = ThreadMgr::instance();
    if (!threadMgr)
        return;
    Thread* mainThread = threadMgr->getMainThread();
    TickSpan wait = TickSpan::makeFromMicroSeconds(10);
    if (mainThread)
    {
        while (!mainThread->getFindContainHeapCache()->tryRemoveHeap(heap))
            Thread::sleep(wait);
    }
    for (;;)
    {
        threadMgr->mListCS.lock();
        bool removed = true;
        for (TListNode<Thread*>* node = threadMgr->mList.begin(); node != threadMgr->mList.end(); node = ThreadMgr::ThreadList::next(node))
        {
            if (!node->mData->getFindContainHeapCache()->tryRemoveHeap(heap))
            {
                removed = false;
                break;
            }
        }
        if (removed)
            break;
        threadMgr->mListCS.unlock();
        Thread::sleep(wait);
    }
    threadMgr->mListCS.unlock();
}

ThreadMgr::ThreadMgr() : mList(), mListCS(), mMainThread(0) {}
ThreadMgr::~ThreadMgr()
{
    ScopedLock<CriticalSection> lock(&mListCS);
    for (TListNode<Thread*>* node = mList.begin(); node != mList.end(); node = ThreadList::next(node))
        node->mData->quit(false);
    bool allDone;
    do
    {
        allDone = true;
        for (TListNode<Thread*>* node = mList.begin(); node != mList.end(); node = ThreadList::next(node))
            allDone &= node->mData->isDone();
        Thread::yield();
    } while (!allDone);
    for (TListNode<Thread*>* node = mList.begin(); node != mList.end(); node = ThreadList::next(node))
        node->mData->waitDone();
    sInstance = 0;
}
ThreadMgr* ThreadMgr::createInstance(Heap* heap)
{
    if (!sInstance)
    {
        unsigned char* buffer = NewArray<unsigned char>(sizeof(ThreadMgr), heap);
        SingletonDisposer_::sStaticDisposer = new (buffer) SingletonDisposer_(heap);
        sInstance = new (buffer) ThreadMgr();
    }
    return sInstance;
}
void ThreadMgr::deleteInstance()
{
    SingletonDisposer_* disposer = SingletonDisposer_::sStaticDisposer;
    if (disposer)
    {
        SingletonDisposer_::sStaticDisposer = 0;
        disposer->~SingletonDisposer_();
        if (sInstance)
        {
            sInstance->~ThreadMgr();
            freeToSeadHeap(sInstance);
        }
        sInstance = 0;
    }
}
void ThreadMgr::initMainThread_(Heap* heap)
{
    OSThread* thread = OSGetCurrentThread();
    mMainThread = new (allocateFromSeadHeap(sizeof(MainThread), heap)) MainThread(heap, thread);
}
unsigned int ThreadMgr::getCurrentThreadID_()
{
    return reinterpret_cast<uintptr_t>(OSGetCurrentThread());
}
void ThreadMgr::initialize(Heap* heap) { initMainThread_(heap); }
void ThreadMgr::destroyMainThread_()
{
    if (mMainThread)
    {
        mMainThread->~MainThread();
        freeToSeadHeap(mMainThread);
        mMainThread = 0;
    }
}
void ThreadMgr::destroy() { destroyMainThread_(); }
Thread* ThreadMgr::getCurrentThread() const
{
    ScopedLock<CriticalSection> lock(&mListCS);
    const unsigned int id = getCurrentThreadID_();
    for (TListNode<Thread*>* node = mList.begin(); node != mList.end(); node = ThreadList::next(node))
    {
        if (node->mData->mId == id)
            return node->mData;
    }
    return mMainThread;
}
void ThreadMgr::waitDoneMultipleThread(Thread* const* threads, int count) { for (int index = 0; index < count; ++index) threads[index]->waitDone(); }
void ThreadMgr::quitAndWaitDoneMultipleThread(Thread** threads, int count, bool jam) { for (int index = 0; index < count; ++index) threads[index]->quit(jam); waitDoneMultipleThread(threads, count); }

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
void PrintFormatter::outputString_(const char* option, PrintOutput* output, const char* text, int length)
{
    if (!option || option[1] == 's')
    {
        if (length == -1)
            output->write(text, strlen(text));
        else
            output->write(text, length);
        return;
    }
    if (option[strlen(option) - 1] == 's')
    {
        FixedSafeString128 converted;
        SafeStringBase<char> source(text);
        int convertedLength = StringUtil::convertUtf8ToSjis(converted.getBuffer(), converted.getBufferSize(), source.cstr(), -1);
        if (option[1] == '-')
        {
            output->write(text, length == -1 ? strlen(text) : length);
            int width = StringUtil::parseS32(SafeStringBase<char>(option + 2), StringUtil::cDecimal);
            for (int index = 0; index < width - convertedLength; ++index)
                output->write(" ", 1);
        }
        else
        {
            int width = StringUtil::parseS32(SafeStringBase<char>(option + 1), StringUtil::cDecimal);
            for (int index = 0; index < width - convertedLength; ++index)
                output->write(" ", 1);
            if (length == -1)
                output->write(text, strlen(text));
            else
                output->write(text, length);
        }
    }
    else
    {
        FixedSafeString128 formatted;
        int formattedLength = formatted.format(option, text);
        output->write(formatted.cstr(), formattedLength);
    }
}
void PrintFormatter::outputPtr_(const char* option, PrintOutput* output, unsigned int pointer)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, pointer);
    else
        length = text.format("0x%08X", pointer);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<unsigned char>(const unsigned char& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%u", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<unsigned short>(const unsigned short& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%u", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<unsigned int>(const unsigned int& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%u", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<unsigned long long>(const unsigned long long& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%llu", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<signed char>(const signed char& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%d", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<short>(const short& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%d", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<int>(const int& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%d", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<long long>(const long long& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%lld", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<float>(const float& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%f", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<double>(const double& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%f", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<char>(const char& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%d", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<long>(const long& value, const char* option, PrintOutput* output)
{
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, value);
    else
        length = text.format("%d", value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<bool>(const bool& value, const char* option, PrintOutput* output)
{
    if (option)
        out<int>(value, option, output);
    else if (value)
        output->write("true", 4);
    else
        output->write("false", 5);
}

template <>
void PrintFormatter::out<char>(const char* value, const char* option, PrintOutput* output)
{
    bool isString = true;
    if (option)
        isString = SafeStringBase<char>(option).include('s');
    if (isString)
    {
        outputString_(option, output, value, -1);
        return;
    }
    FixedSafeString<32> text;
    int length = text.format(option, value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<wchar_t>(const wchar_t* value, const char* option, PrintOutput* output)
{
    bool isString = true;
    if (option)
        isString = SafeStringBase<char>(option).include('s');
    if (isString)
    {
        FixedSafeString<256> text;
        int length = StringUtil::convertUtf16ToUtf8(text.getBuffer(), text.getBufferSize(), SafeStringBase<wchar_t>(value).cstr(), -1);
        outputString_(option, output, text.cstr(), length);
        return;
    }
    FixedSafeString<32> text;
    int length = text.format(option, value);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::out<TickSpan>(const TickSpan& value, const char*, PrintOutput* output)
{
    out<long long>(value.toMicroSeconds(), "%lld", output);
    output->write("us", 2);
}

template <>
void PrintFormatter::OutImpl<char, SafeStringBase>::out(const SafeStringBase<char>& value, const char* option, PrintOutput* output)
{
    outputString_(option, output, value.cstr(), value.calcLength());
}

template <>
void PrintFormatter::OutImpl<char, BufferedSafeStringBase>::out(const BufferedSafeStringBase<char>& value, const char* option, PrintOutput* output)
{
    outputString_(option, output, value.cstr(), value.calcLength());
}

template <>
void PrintFormatter::OutImpl<unsigned char, BitFlag>::out(const BitFlag<unsigned char>& value, const char* option, PrintOutput* output)
{
    unsigned char bits = value.getDirect();
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, bits);
    else
        length = text.format("0x%x", bits);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::OutImpl<unsigned short, BitFlag>::out(const BitFlag<unsigned short>& value, const char* option, PrintOutput* output)
{
    unsigned short bits = value.getDirect();
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, bits);
    else
        length = text.format("0x%x", bits);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::OutImpl<unsigned int, BitFlag>::out(const BitFlag<unsigned int>& value, const char* option, PrintOutput* output)
{
    unsigned int bits = value.getDirect();
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, bits);
    else
        length = text.format("0x%x", bits);
    output->write(text.cstr(), length);
}

template <>
void PrintFormatter::OutImpl<unsigned long long, BitFlag>::out(const BitFlag<unsigned long long>& value, const char* option, PrintOutput* output)
{
    unsigned long long bits = value.getDirect();
    FixedSafeString<32> text;
    int length;
    if (option)
        length = text.format(option, bits);
    else
        length = text.format("0x%llx", bits);
    output->write(text.cstr(), length);
}

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

bool StringUtil::__N_18_peadStringUtil_cpp_6714fef4::tryParsePrefixImpl_(int* base, SafeStringBase<char>::iterator& current, const SafeStringBase<char>&)
{
    char character = *current;
    if (character < '0' || character > '9') return false;
    if (character == '0')
    {
        SafeStringBase<char>::iterator next = current;
        ++next;
        if (*next == 'b')
        {
            *base = 2;
            ++current;
            ++current;
        }
        else if (*next == 'x')
        {
            *base = 16;
            ++current;
            ++current;
        }
        else
        {
            *base = 8;
        }
    }
    else
    {
        *base = 10;
    }
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
        int digit = checkAndConvertHexChar_(character);
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
    if (value) *value = static_cast<T>(parsed * sign);
    return true;
}

template <typename T>
bool StringUtil::__N_18_peadStringUtil_cpp_6714fef4::tryParseImpl_(T* value, const SafeStringBase<char>& text, int base)
{
    SafeStringBase<char>::iterator current = text.begin();
    int sign;
    tryParseSignImpl_(&sign, current);
    if (base > 0 || tryParsePrefixImpl_(&base, current, text))
    {
        if (base == 10) return tryParseDecImpl_(value, current, sign);
        T parsed;
        if (tryParseNumberImpl_(&parsed, current, base))
        {
            *value = static_cast<T>(parsed * sign);
            return true;
        }
    }
    return false;
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
    tryParseImpl_(&value, text, cardinal);
    return value;
}

template <typename T>
T StringUtil::__N_18_peadStringUtil_cpp_6714fef4::parseDecimalFractionImpl_(const SafeStringBase<char>& text)
{
    T value = 0;
    tryParseDecimalFractionImpl_(&value, text);
    return value;
}

namespace StringUtilImpl = StringUtil::__N_18_peadStringUtil_cpp_6714fef4;
typedef StringUtilImpl::FormatParser16 FormatParser16;
typedef StringUtilImpl::OutBuffer16 OutBuffer16;

FormatParser16::FormatParser16(va_list args)
{
    mArgs[0] = args[0];
    clear_();
}
void FormatParser16::clear_()
{
    mFlags = 0;
    mWidth = 0;
    mPrecision = 0;
    mHasPrecision = false;
    mSize = cSizeInt;
}
const wchar_t* FormatParser16::parseFlag_(const wchar_t* format)
{
    for (;; ++format)
    {
        if (*format == L'-') mFlags |= cFlagLeftJustify;
        else if (*format == L'+') mFlags |= cFlagForceSign;
        else if (*format == L' ') mFlags |= cFlagBlankSign;
        else if (*format == L'#') mFlags |= cFlagAlternate;
        else if (*format == L'0') mFlags |= cFlagZeroPad;
        else return format;
    }
}
const wchar_t* FormatParser16::parseWidth_(const wchar_t* format)
{
    if (*format == L'*')
    {
        mWidth = va_arg(mArgs, int);
        return format + 1;
    }
    if (*format >= L'1' && *format <= L'9')
    {
        mWidth = *format++ - L'0';
        while (*format >= L'0' && *format <= L'9') mWidth = mWidth * 10 + *format++ - L'0';
    }
    return format;
}
const wchar_t* FormatParser16::parsePrecision_(const wchar_t* format)
{
    if (*format != L'.') return format;
    mHasPrecision = true;
    ++format;
    if (*format == L'*')
    {
        mPrecision = va_arg(mArgs, int);
        return format + 1;
    }
    if (*format >= L'0' && *format <= L'9')
    {
        mPrecision = *format++ - L'0';
        while (*format >= L'0' && *format <= L'9') mPrecision = mPrecision * 10 + *format++ - L'0';
    }
    return format;
}
const wchar_t* FormatParser16::parseSize_(const wchar_t* format)
{
    if (*format == L'h')
    {
        ++format;
        if (*format == L'h') { ++format; mSize = cSizeChar; }
        else mSize = cSizeShort;
    }
    else if (*format == L'l')
    {
        ++format;
        if (*format == L'l') { ++format; mSize = cSizeLongLong; }
        else mSize = cSizeLong;
    }
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
    T value = va_arg(mArgs, T);
    if (signedValue && value < 0) { *negative = true; printIntegerInverseString_(output, base, upper, static_cast<unsigned long long>(-(long long)value)); }
    else printIntegerInverseString_(output, base, upper, static_cast<unsigned long long>(value));
}

template <typename T>
void FormatParser16::printStringCore_(OutBuffer16* output, const T* text)
{
    if (!text) return;
    int count = 0;
    while (*text && (!mHasPrecision || count < mPrecision)) { appendWide(output, static_cast<wchar_t>(*text++)); ++count; }
}

void FormatParser16::printInteger_(OutBuffer16* output, unsigned int base, bool signedValue, bool upper)
{
    wchar_t text[96];
    wchar_t format[24];
    wchar_t type = base == 16 ? (upper ? L'X' : L'x') : base == 8 ? L'o' : L'd';
    ::swprintf(format, 24, L"%%%s%s%s%s%s%d.%d%c", (mFlags & cFlagLeftJustify) ? L"-" : L"", (mFlags & cFlagForceSign) ? L"+" : (mFlags & cFlagBlankSign) ? L" " : L"", (mFlags & cFlagAlternate) ? L"#" : L"", (mFlags & cFlagZeroPad) ? L"0" : L"", mSize == cSizeLongLong ? L"ll" : mSize == cSizeLong ? L"l" : L"", mWidth, mHasPrecision ? mPrecision : 1, type);
    if (mSize == cSizeLongLong) ::swprintf(text, 96, format, signedValue ? va_arg(mArgs, long long) : static_cast<long long>(va_arg(mArgs, unsigned long long)));
    else ::swprintf(text, 96, format, signedValue ? va_arg(mArgs, int) : static_cast<int>(va_arg(mArgs, unsigned int)));
    printStringCore_(output, text);
}
void FormatParser16::printCharacter_(OutBuffer16* output) { appendWide(output, static_cast<wchar_t>(va_arg(mArgs, int))); }
void FormatParser16::printString_(OutBuffer16* output) { if (mSize == cSizeLong) printStringCore_(output, va_arg(mArgs, const wchar_t*)); else printStringCore_(output, va_arg(mArgs, const char*)); }
void FormatParser16::printFloat_(OutBuffer16* output, wchar_t type, bool)
{
    wchar_t text[128];
    wchar_t format[24];
    ::swprintf(format, 24, L"%%%s%s%s%s%d.%d%c", (mFlags & cFlagLeftJustify) ? L"-" : L"", (mFlags & cFlagForceSign) ? L"+" : (mFlags & cFlagBlankSign) ? L" " : L"", (mFlags & cFlagAlternate) ? L"#" : L"", (mFlags & cFlagZeroPad) ? L"0" : L"", mWidth, mHasPrecision ? mPrecision : 6, type);
    ::swprintf(text, 128, format, va_arg(mArgs, double));
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

bool StringUtil::tryParseU64(unsigned long long* v, const SafeStringBase<char>& s, CardinalNumber c) { return StringUtilImpl::tryParseImpl_(v, s, c); }
bool StringUtil::tryParseS64(long long* v, const SafeStringBase<char>& s, CardinalNumber c) { return StringUtilImpl::tryParseImpl_(v, s, c); }
bool StringUtil::tryParseU32(unsigned int* v, const SafeStringBase<char>& s, CardinalNumber c) { return StringUtilImpl::tryParseImpl_(v, s, c); }
bool StringUtil::tryParseS32(int* v, const SafeStringBase<char>& s, CardinalNumber c) { return StringUtilImpl::tryParseImpl_(v, s, c); }
bool StringUtil::tryParseU16(unsigned short* v, const SafeStringBase<char>& s, CardinalNumber c) { return StringUtilImpl::tryParseImpl_(v, s, c); }
bool StringUtil::tryParseS16(short* v, const SafeStringBase<char>& s, CardinalNumber c) { return StringUtilImpl::tryParseImpl_(v, s, c); }
bool StringUtil::tryParseU8(unsigned char* v, const SafeStringBase<char>& s, CardinalNumber c) { return StringUtilImpl::tryParseImpl_(v, s, c); }
bool StringUtil::tryParseS8(signed char* v, const SafeStringBase<char>& s, CardinalNumber c) { return StringUtilImpl::tryParseImpl_(v, s, c); }
int StringUtil::parseS32(const SafeStringBase<char>& s, CardinalNumber c) { return StringUtilImpl::parseImpl_<int>(s, c); }
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
