#include "game/data/DoubleTag.hpp"

#include "game/core/DataInput.hpp"
#include "game/core/DataOutput.hpp"

#include <math.h>
#include <wchar.h>

DoubleTag::DoubleTag()
{
}

DoubleTag::DoubleTag(double value)
{
    mValue = value;
}

DoubleTag::~DoubleTag()
{
}

void DoubleTag::write(DataOutput* output)
{
    output->writeDouble(mValue);
}

void DoubleTag::load(DataInput* input, int depth)
{
    mValue = input->readDouble();
}

std::wstring DoubleTag::toString()
{
    static wchar_t description[32];

    swprintf(description, 32, L"%f", mValue);

    return description;
}

unsigned char DoubleTag::getId()
{
    return 6;
}

bool DoubleTag::equals(Tag* other)
{
    if (!Tag::equals(other))
    {
        return false;
    }

    return mValue == ((DoubleTag*)other)->mValue;
}

DoubleTag* DoubleTag::copy()
{
    return new DoubleTag(mValue);
}

long long DoubleTag::getAsLong()
{
    return (long long)floor(mValue);
}

int DoubleTag::getAsInt()
{
    int floorValue = (int)mValue;

    if (mValue < (double)floorValue)
    {
        --floorValue;
    }

    return floorValue;
}

short DoubleTag::getAsShort()
{
    int floorValue = (int)mValue;

    if (mValue < (double)floorValue)
    {
        --floorValue;
    }

    return (short)floorValue;
}

unsigned char DoubleTag::getAsByte()
{
    int floorValue = (int)mValue;

    if (mValue < (double)floorValue)
    {
        --floorValue;
    }

    return (unsigned char)floorValue;
}

double DoubleTag::getAsDouble()
{
    return mValue;
}

float DoubleTag::getAsFloat()
{
    return (float)mValue;
}
