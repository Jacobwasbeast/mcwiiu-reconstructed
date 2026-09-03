#ifndef GAME_WORLD_BLOCK_ENUMPROPERTY_HPP
#define GAME_WORLD_BLOCK_ENUMPROPERTY_HPP

#include <cstddef>
#include <string>
#include <typeinfo>
#include <vector>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/world/block/Property.hpp"

template <class T>
class EnumProperty : public AbstractProperty<T>
{
public:
    static EnumProperty<T>* create(const std::wstring& name, const std::type_info& valueClass,
                                   const boost::unordered_set<T>& values, const Predicate<T>* filter);

    virtual ~EnumProperty();

    virtual const std::vector<Boxed*>& getPossibleValues() const;
    virtual bool equals(const Property* other) const;
    virtual int hashCode() const;
    virtual unsigned int getValueCount() const;
    virtual Boxed* getValueAtIndex(unsigned int index) const;
    virtual unsigned int getIndexForValue(Boxed* value) const;
    virtual std::wstring getName(const T& value) const;
    virtual T getUnboxedValue(const std::wstring& name) const;

protected:
    typedef boost::unordered_map<std::wstring, T> ValuesByName;

    EnumProperty(const std::wstring& name, const std::type_info& valueClass,
                 const boost::unordered_set<T>& values);

    std::vector<Boxed*> mPossibleValues;
    ValuesByName mValuesByName;
};

template <class T>
EnumProperty<T>::EnumProperty(const std::wstring& name, const std::type_info& valueClass,
                              const boost::unordered_set<T>& values)
    : AbstractProperty<T>(name, valueClass)
{
    for (typename boost::unordered_set<T>::const_iterator it = values.begin(); it != values.end(); ++it)
    {
        std::wstring valueName = getName(*it);

        mValuesByName.find(valueName);
        mValuesByName[valueName] = *it;

        mPossibleValues.push_back(new TypedBoxed<T>(*it));
    }

    this->updateCachedHashCode();
}

template <class T>
EnumProperty<T>::~EnumProperty()
{
    for (std::vector<Boxed*>::iterator it = mPossibleValues.begin(); it != mPossibleValues.end(); ++it)
    {
        delete *it;
    }
}

template <class T>
EnumProperty<T>* EnumProperty<T>::create(const std::wstring& name, const std::type_info& valueClass,
                                         const boost::unordered_set<T>& values, const Predicate<T>* filter)
{
    boost::unordered_set<T> allowedValues;

    for (typename boost::unordered_set<T>::const_iterator it = values.begin(); it != values.end(); ++it)
    {
        if (filter->apply(*it))
        {
            allowedValues.insert(*it);
        }
    }

    return new EnumProperty<T>(name, valueClass, allowedValues);
}

template <class T>
const std::vector<Boxed*>& EnumProperty<T>::getPossibleValues() const
{
    return mPossibleValues;
}

template <class T>
bool EnumProperty<T>::equals(const Property* other) const
{
    if (this == other)
    {
        return true;
    }

    const EnumProperty<T>* property = dynamic_cast<const EnumProperty<T>*>(other);
    if (property == NULL)
    {
        return false;
    }

    if (!AbstractProperty<T>::equals(other))
    {
        return false;
    }

    return mPossibleValues == property->mPossibleValues && mValuesByName == property->mValuesByName;
}

template <class T>
int EnumProperty<T>::hashCode() const
{
    return AbstractProperty<T>::hashCode() * 31 + AbstractProperty<T>::hashBoxedSet(mPossibleValues);
}

template <class T>
unsigned int EnumProperty<T>::getValueCount() const
{
    return mPossibleValues.size();
}

template <class T>
Boxed* EnumProperty<T>::getValueAtIndex(unsigned int index) const
{
    return mPossibleValues.at(index);
}

template <class T>
unsigned int EnumProperty<T>::getIndexForValue(Boxed* value) const
{
    unsigned int index = 0;

    for (std::vector<Boxed*>::const_iterator it = mPossibleValues.begin(); it != mPossibleValues.end(); ++it)
    {
        if (*value == *it)
        {
            return index;
        }

        ++index;
    }

    return 0;
}

template <class T>
std::wstring EnumProperty<T>::getName(const T& value) const
{
    return value->getSerializedName();
}

template <class T>
T EnumProperty<T>::getUnboxedValue(const std::wstring& name) const
{
    typename ValuesByName::const_iterator entry = mValuesByName.find(name);
    if (entry != mValuesByName.end())
    {
        return entry->second;
    }

    return NULL;
}

#endif
