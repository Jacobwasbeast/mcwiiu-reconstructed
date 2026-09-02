#include "game/data/ByteArrayTag.hpp"

#include "game/core/DataInput.hpp"
#include "game/core/DataOutput.hpp"
#include "game/core/System.hpp"

#include <string.h>
#include <wchar.h>

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
    int length = input->readInt();

    if (mOwnsData)
    {
        delete[] mData.data;
    }
    else
    {
        mOwnsData = true;
    }

    mData = arrayWithLength<unsigned char>(length, true);

    input->readFully(mData);
}

std::wstring ByteArrayTag::toString()
{
    static wchar_t description[32];

    swprintf(description, 32, L"[%d bytes]", mData.length);

    return description;
}

unsigned char ByteArrayTag::getId()
{
    return 7;
}

bool ByteArrayTag::equals(Tag* other)
{
    if (!Tag::equals(other))
    {
        return false;
    }

    ByteArrayTag* otherArray = (ByteArrayTag*)other;

    if (mData.data == NULL)
    {
        return otherArray->mData.data == NULL;
    }

    if (mData.length != otherArray->mData.length)
    {
        return false;
    }

    return memcmp(mData.data, otherArray->mData.data, mData.length) == 0;
}

ByteArrayTag* ByteArrayTag::copy()
{
    arrayWithLength<unsigned char> copiedData(mData.length, true);

    System::arraycopy(mData, 0, &copiedData, 0, mData.length);

    return new ByteArrayTag(copiedData, true);
}

arrayWithLength<unsigned char> ByteArrayTag::getAsByteArray()
{
    return mData;
}
