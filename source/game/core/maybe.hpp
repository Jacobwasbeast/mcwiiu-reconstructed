#ifndef GAME_CORE_MAYBE_HPP
#define GAME_CORE_MAYBE_HPP

template <class T>
class maybe
{
public:
    maybe();
    maybe(const T& value);
    maybe(const maybe<T>& other);
    ~maybe();

    bool isSet() const;
    const T& get() const;

    maybe<T> otherwise(maybe<T> fallback) const;

    template <class U>
    maybe<U> fmap(maybe<U> (*func)(T)) const;

private:
    bool hasValue;
    T value;
};

template <class T>
maybe<T>::maybe()
    : hasValue(false)
    , value()
{
}

template <class T>
maybe<T>::maybe(const T& value)
    : hasValue(true)
    , value(value)
{
}

template <class T>
maybe<T>::maybe(const maybe<T>& other)
    : hasValue(other.hasValue)
    , value(other.value)
{
}

template <class T>
maybe<T>::~maybe()
{
}

template <class T>
bool maybe<T>::isSet() const
{
    return hasValue;
}

template <class T>
const T& maybe<T>::get() const
{
    return value;
}

template <class T>
maybe<T> maybe<T>::otherwise(maybe<T> fallback) const
{
    if (hasValue)
    {
        return maybe<T>(value);
    }

    return fallback;
}

template <class T>
template <class U>
maybe<U> maybe<T>::fmap(maybe<U> (*func)(T)) const
{
    if (!hasValue)
    {
        return maybe<U>();
    }

    return func(value);
}

#endif
