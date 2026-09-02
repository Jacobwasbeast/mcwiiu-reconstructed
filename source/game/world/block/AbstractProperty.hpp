#ifndef GAME_WORLD_BLOCK_ABSTRACTPROPERTY_HPP
#define GAME_WORLD_BLOCK_ABSTRACTPROPERTY_HPP

#include <cstddef>
#include <string>
#include <typeinfo>
#include <vector>

#include <boost/functional/hash.hpp>
#include <boost/unordered_set.hpp>

#include "game/core/Boxed.hpp"
#include "game/world/block/Property.hpp"

template <class T> class TypedBoxed;

int javaStringHashCode(const std::wstring& value);

template <class T>
class AbstractProperty : public Property
{
public:
    virtual std::wstring getName() const;
    virtual std::wstring getName(const Boxed* value) const;
    virtual const std::type_info& getValueClass() const;
    virtual Boxed* getValue(const std::wstring& name) const;
    virtual std::wstring toString() const;
    virtual bool equals(const Property* other) const;
    virtual int hashCode() const;
    virtual std::wstring getName(const T& value) const = 0;
    virtual T getUnboxedValue(const std::wstring& name) const = 0;

    void getUnboxedPossibleValues(boost::unordered_set<T>& values) const;

protected:
    AbstractProperty(const std::wstring& name, const std::type_info& valueClass);

    static int hashBoxedSet(const std::vector<Boxed*>& values);

    const std::type_info* mValueClass;
    std::wstring mName;
};

template <class T>
AbstractProperty<T>::AbstractProperty(const std::wstring& name, const std::type_info& valueClass)
    : mValueClass(&valueClass),
      mName(name)
{
}

template <class T>
std::wstring AbstractProperty<T>::getName() const
{
    return mName;
}

template <class T>
std::wstring AbstractProperty<T>::getName(const Boxed* value) const
{
    const TypedBoxed<T>* boxed = value->isA(typeid(T)) ? static_cast<const TypedBoxed<T>*>(value) : NULL;
    return getName(boxed->getValue());
}

template <class T>
const std::type_info& AbstractProperty<T>::getValueClass() const
{
    return *mValueClass;
}

template <class T>
Boxed* AbstractProperty<T>::getValue(const std::wstring& name) const
{
    return new TypedBoxed<T>(getUnboxedValue(name));
}

template <class T>
std::wstring AbstractProperty<T>::toString() const
{
    return L"";
}

template <class T>
bool AbstractProperty<T>::equals(const Property* other) const
{
    if (this == other)
    {
        return true;
    }

    const AbstractProperty<T>* property = dynamic_cast<const AbstractProperty<T>*>(other);
    if (property == NULL)
    {
        return false;
    }

    return mValueClass == property->mValueClass && mName == property->mName;
}

template <class T>
int AbstractProperty<T>::hashCode() const
{
    return (int)mValueClass * 31 + javaStringHashCode(mName);
}

template <class T>
void AbstractProperty<T>::getUnboxedPossibleValues(boost::unordered_set<T>& values) const
{
    values.clear();

    const std::vector<Boxed*>& possibleValues = getPossibleValues();
    for (std::vector<Boxed*>::const_iterator it = possibleValues.begin(); it != possibleValues.end(); ++it)
    {
        const TypedBoxed<T>* boxed = (*it)->isA(typeid(T)) ? static_cast<const TypedBoxed<T>*>(*it) : NULL;
        values.insert(boxed->getValue());
    }
}

template <class T>
int AbstractProperty<T>::hashBoxedSet(const std::vector<Boxed*>& values)
{
    static boost::hash<T> hasher;

    int hash = 0;
    for (std::vector<Boxed*>::const_iterator it = values.begin(); it != values.end(); ++it)
    {
        const TypedBoxed<T>* boxed = (*it)->isA(typeid(T)) ? static_cast<const TypedBoxed<T>*>(*it) : NULL;
        T value = boxed->getValue();
        hash += hasher(value);
    }

    return hash;
}

#endif
