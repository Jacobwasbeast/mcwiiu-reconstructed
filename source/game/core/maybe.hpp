#ifndef GAME_CORE_MAYBE_HPP
#define GAME_CORE_MAYBE_HPP

template <class T>
class maybe
{
public:
    maybe()
        : hasValue(false)
        , value()
    {
    }

    maybe(const T& value)
        : hasValue(true)
        , value(value)
    {
    }

    bool isSet() const
    {
        return hasValue;
    }

    const T& get() const
    {
        return value;
    }

    maybe<T> otherwise(maybe<T> fallback) const
    {
        if (hasValue)
        {
            return maybe<T>(value);
        }

        return fallback;
    }

    template <class U>
    maybe<U> fmap(maybe<U> (*func)(T)) const
    {
        if (!hasValue)
        {
            return maybe<U>();
        }

        return func(value);
    }

private:
    bool hasValue;
    T value;
};

#endif
