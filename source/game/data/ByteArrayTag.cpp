#include "game/data/ByteArrayTag.hpp"

#include "game/core/DataInput.hpp"
#include "game/core/DataOutput.hpp"
#include <cstring>
#include <cwchar>

ByteArrayTag::ByteArrayTag()
{
    mOwnsData = false;
}

ByteArrayTag::ByteArrayTag(arrayWithLength<unsigned char> data, bool ownsData)
{
    mData = data;
    mOwnsData = ownsData;
}

ByteArrayTag::~ByteArrayTag()
{
    if (mOwnsData)
    {
        delete[] mData.data;
    }
}

void ByteArrayTag::write(DataOutput* output)
{
    output->writeInt(mData.length);
    output->write(mData);
}

void ByteArrayTag::load(DataInput* input, int depth)
{
    unsigned int length = static_cast<unsigned int>(input->readInt());

    if (!mOwnsData)
    {
        mOwnsData = true;
    }
    else
    {
        delete[] mData.data;
    }

    mData = arrayWithLength<unsigned char>(length, true);
    input->readFully(mData);
}

unsigned char ByteArrayTag::getId()
{
    return 7;
}

std::wstring ByteArrayTag::toString()
{
    static wchar_t description[32];
    swprintf(description, 32, L"[%d bytes]", mData.length);
    return description;
}

bool ByteArrayTag::equals(Tag* other)
{
    if (!Tag::equals(other))
    {
        return false;
    }

    ByteArrayTag* otherArray = static_cast<ByteArrayTag*>(other);
    if (mData.data == NULL)
    {
        return otherArray->mData.data == NULL;
    }
    if (mData.length != otherArray->mData.length)
    {
        return false;
    }
    return std::memcmp(mData.data, otherArray->mData.data, mData.length) == 0;
}

ByteArrayTag* ByteArrayTag::copy()
{
    arrayWithLength<unsigned char> copiedData(mData.length, true);
    std::memcpy(copiedData.data, mData.data, mData.length);
    return new ByteArrayTag(copiedData, true);
}

arrayWithLength<unsigned char> ByteArrayTag::getAsByteArray()
{
    return mData;
}
