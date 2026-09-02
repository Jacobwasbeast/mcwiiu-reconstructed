#include "game/data/FloatTag.hpp"

#include "game/core/DataInput.hpp"
#include "game/core/DataOutput.hpp"
#include "game/data/Tag.hpp"

#include <wchar.h>

FloatTag::FloatTag()
{
    mValue = 0.0f;
}

FloatTag::FloatTag(float value)
{
    mValue = value;
}

FloatTag::~FloatTag()
{
}

void FloatTag::write(DataOutput* output)
{
    output->writeFloat(mValue);
}

void FloatTag::load(DataInput* input, int depth)
{
    mValue = input->readFloat();
}

std::wstring FloatTag::toString()
{
    static wchar_t description[32];

    swprintf(description, 32, L"%f", mValue);

    return description;
}

unsigned char FloatTag::getId()
{
    return 5;
}

bool FloatTag::equals(Tag* other)
{
    if (!Tag::equals(other))
    {
        return false;
    }

    return mValue == ((FloatTag*)other)->mValue;
}

FloatTag* FloatTag::copy()
{
    return new FloatTag(mValue);
}

long long FloatTag::getAsLong()
{
    return (long)mValue;
}

int FloatTag::getAsInt()
{
    int floorValue = (int)mValue;

    if (mValue < (float)floorValue)
    {
        --floorValue;
    }

    return floorValue;
}

short FloatTag::getAsShort()
{
    int floorValue = (int)mValue;

    if (mValue < (float)floorValue)
    {
        --floorValue;
    }

    return (short)floorValue;
}

unsigned char FloatTag::getAsByte()
{
    int floorValue = (int)mValue;

    if (mValue < (float)floorValue)
    {
        --floorValue;
    }

    return (unsigned char)floorValue;
}

double FloatTag::getAsDouble()
{
    return mValue;
}

float FloatTag::getAsFloat()
{
    return mValue;
}
