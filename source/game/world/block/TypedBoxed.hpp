#ifndef GAME_WORLD_BLOCK_TYPEDBOXED_HPP
#define GAME_WORLD_BLOCK_TYPEDBOXED_HPP

#include <string>
#include <typeinfo>

#include <boost/functional/hash.hpp>

#include "game/core/Boxed.hpp"

template <class T> std::wstring _toString(T value);

template <class T>
std::wstring _toString(T* value)
{
    return value->toString();
}

template <class T>
class TypedBoxed : public Boxed
{
public:
    TypedBoxed(T value) : Boxed(&typeid(T)), mValue(value) {}
    virtual ~TypedBoxed() {}

    const T& getValue() const { return this->mValue; }

    virtual unsigned int hashOf() const;
    virtual bool operator==(const Boxed* other) const;
    virtual void setValue(const Boxed* other);
    virtual std::wstring toString() const;

private:
    T mValue;
};

template <class T>
unsigned int TypedBoxed<T>::hashOf() const
{
    return boost::hash_value(this->mValue);
}

template <class T>
bool TypedBoxed<T>::operator==(const Boxed* other) const
{
    if (other->isA(this->mValueClass))
    {
        const TypedBoxed<T>* boxed = other->isA(typeid(T)) ? static_cast<const TypedBoxed<T>*>(other) : NULL;
        if (this->mValue == boxed->mValue)
        {
            return true;
        }
    }

    return false;
}

template <class T>
void TypedBoxed<T>::setValue(const Boxed* other)
{
    const TypedBoxed<T>* boxed = other->isA(typeid(T)) ? static_cast<const TypedBoxed<T>*>(other) : NULL;
    this->mValue = boxed->mValue;
}

template <class T>
std::wstring TypedBoxed<T>::toString() const
{
    return _toString(this->mValue);
}

#endif
