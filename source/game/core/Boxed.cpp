#include "game/core/Boxed.hpp"

Boxed::Boxed(const std::type_info* valueClass)
    : mValueClass(valueClass)
{
}

Boxed::~Boxed()
{
}

bool Boxed::isA(const std::type_info* valueClass) const
{
    return *mValueClass == *valueClass;
}

bool Boxed::isA(const std::type_info& valueClass) const
{
    return *mValueClass == valueClass;
}

bool Boxed::operator==(Boxed* other) const
{
    const Boxed* boxed = other;
    return *this == boxed;
}

bool Boxed::operator!=(const Boxed* other) const
{
    return !(*this == other);
}

bool Boxed::operator!=(Boxed* other) const
{
    const Boxed* boxed = other;
    return !(*this == boxed);
}
