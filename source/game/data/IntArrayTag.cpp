#include "game/data/IntArrayTag.hpp"

#include "game/core/DataInput.hpp"
#include "game/core/DataOutput.hpp"
#include "game/core/System.hpp"
#include <string.h>
#include <wchar.h>

IntArrayTag::IntArrayTag()
{
}

IntArrayTag::IntArrayTag(arrayWithLength<int> data)
    : mData(data)
{
}

IntArrayTag::~IntArrayTag()
{
    delete[] mData.data;
}

void IntArrayTag::write(DataOutput* output)
{
    output->writeInt(mData.length);

    for (unsigned int i = 0; i < mData.length; i++)
    {
        output->writeInt(mData.data[i]);
    }
}

void IntArrayTag::load(DataInput* input, int depth)
{
    unsigned int length = input->readInt();

    if (mData.data != NULL)
    {
        delete[] mData.data;
    }

    mData = arrayWithLength<int>(length, true);

    for (int i = 0; i < (int)length; i++)
    {
        mData.data[i] = input->readInt();
    }
}

std::wstring IntArrayTag::toString()
{
    static wchar_t description[32];

    swprintf(description, 32, L"[%d bytes]", mData.length);

    return description;
}

unsigned char IntArrayTag::getId()
{
    return 11;
}

bool IntArrayTag::equals(Tag* other)
{
    if (!Tag::equals(other))
    {
        return false;
    }

    IntArrayTag* otherArray = (IntArrayTag*)other;

    bool result = true;

    if (mData.data == NULL)
    {
        if (otherArray->mData.data != NULL)
        {
            result = false;
        }
    }
    else
    {
        if (mData.length != otherArray->mData.length)
        {
            return false;
        }

        if (memcmp(mData.data, otherArray->mData.data, mData.length * sizeof(int)) != 0)
        {
            result = false;
        }
    }

    return result;
}

IntArrayTag* IntArrayTag::copy()
{
    arrayWithLength<int> copiedData(mData.length, true);

    System::arraycopy(mData, 0, &copiedData, 0, mData.length);

    return new IntArrayTag(copiedData);
}

arrayWithLength<int> IntArrayTag::getAsIntArray()
{
    arrayWithLength<int> result;
    result.data = mData.data;
    result.length = mData.length;
    return result;
}
