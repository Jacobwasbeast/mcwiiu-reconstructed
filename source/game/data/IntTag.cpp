#include "game/data/IntTag.hpp"

#include "game/core/DataInput.hpp"
#include "game/core/DataOutput.hpp"

#include <wchar.h>

IntTag::IntTag()
{
}

IntTag::IntTag(int value)
{
    mValue = value;
}

IntTag::~IntTag()
{
}

void IntTag::write(DataOutput* output)
{
    output->writeInt(mValue);
}

void IntTag::load(DataInput* input, int depth)
{
    mValue = input->readInt();
}

std::wstring IntTag::toString()
{
    static wchar_t description[32];

    swprintf(description, 32, L"%d", mValue);

    return description;
}

unsigned char IntTag::getId()
{
    return 3;
}

bool IntTag::equals(Tag* other)
{
    if (!Tag::equals(other))
    {
        return false;
    }

    return mValue == ((IntTag*)other)->mValue;
}

IntTag* IntTag::copy()
{
    return new IntTag(mValue);
}

long long IntTag::getAsLong()
{
    return mValue;
}

int IntTag::getAsInt()
{
    return mValue;
}

short IntTag::getAsShort()
{
    return (short)mValue;
}

unsigned char IntTag::getAsByte()
{
    return (unsigned char)mValue;
}

double IntTag::getAsDouble()
{
    return (double)mValue;
}

float IntTag::getAsFloat()
{
    return (float)mValue;
}
