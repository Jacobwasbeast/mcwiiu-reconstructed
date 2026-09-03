#include "game/core/RangedAttribute.hpp"

RangedAttribute::RangedAttribute(Attribute* parentAttribute, eATTRIBUTE_ID id, double defaultValue, double minValue, double maxValue)
    : BaseAttribute(parentAttribute, id, defaultValue),
      minValue(minValue),
      maxValue(maxValue)
{
}

RangedAttribute::~RangedAttribute()
{
}

double RangedAttribute::sanitizeValue(double value)
{
    if (value < this->minValue)
    {
        return this->minValue;
    }
    if (value > this->maxValue)
    {
        return this->maxValue;
    }
    return value;
}

double RangedAttribute::getMinValue()
{
    return this->minValue;
}

double RangedAttribute::getMaxValue()
{
    return this->maxValue;
}
