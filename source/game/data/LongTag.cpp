#include "game/data/LongTag.hpp"

#include "game/core/DataInput.hpp"
#include "game/core/DataOutput.hpp"

#include <wchar.h>

LongTag::LongTag()
{
    mValue = 0;
}

LongTag::LongTag(long long value)
{
    mValue = value;
}

LongTag::~LongTag()
{
}

void LongTag::write(DataOutput* output)
{
    output->writeLong(mValue);
}

void LongTag::load(DataInput* input, int depth)
{
    mValue = input->readLong();
}

std::wstring LongTag::toString()
{
    static wchar_t description[32];

    swprintf(description, 32, L"%I64d", mValue);

    return description;
}

unsigned char LongTag::getId()
{
    return 4;
}

bool LongTag::equals(Tag* other)
{
    if (!Tag::equals(other))
    {
        return false;
    }

    LongTag* otherTag = (LongTag*)other;

    return mValue == otherTag->mValue;
}

LongTag* LongTag::copy()
{
    return new LongTag(mValue);
}

long long LongTag::getAsLong()
{
    return mValue;
}

int LongTag::getAsInt()
{
    return (int)mValue;
}

short LongTag::getAsShort()
{
    return (short)(mValue & 0xffff);
}

unsigned char LongTag::getAsByte()
{
    return (unsigned char)(mValue & 0xff);
}

double LongTag::getAsDouble()
{
    return (double)mValue;
}

float LongTag::getAsFloat()
{
    return (float)mValue;
}
