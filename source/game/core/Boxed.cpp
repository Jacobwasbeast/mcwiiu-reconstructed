#include "game/core/Boxed.hpp"

Boxed::Boxed()
    : mValueClass(NULL)
{
}

Boxed::Boxed(const std::type_info* type)
    : mValueClass(type)
{
}

Boxed::~Boxed()
{
}

bool Boxed::isA(const std::type_info* type) const
{
    return *mValueClass == *type;
}

bool Boxed::isA(const std::type_info& type) const
{
    return *mValueClass == type;
}

bool Boxed::operator!=(const Boxed* value) const
{
    return !(*this == value);
}

bool Boxed::operator==(Boxed* value) const
{
    return *this == static_cast<const Boxed*>(value);
}

bool Boxed::operator!=(Boxed* value) const
{
    return !(*this == static_cast<const Boxed*>(value));
}
